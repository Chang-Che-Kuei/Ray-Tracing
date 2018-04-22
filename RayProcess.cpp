#include"RayProcess.h"
#include<algorithm>
#include<math.h>
using namespace std;
#define D_Eye2View 0.1 // distance between eye and view
#define RECUR_LIMIT 3
#define REFLECT_LIMIT 0.01

#define ERROR 1e-6
void SetViewXY(vec3 &x,vec3 &y,vec3 dir)
{
    //suppose the vision's x vector is vertical with z axis
    x = vec3(dir[1],-dir[0],0);
    if(x[0]==0&&x[1]==0)x[0]=1;//prevent (0,0,0)
    y = dir^x;//outer product
    x.normalize(),y.normalize();
}

void SetViewPlane(vec3 &viewX,vec3 &viewY,vec3 &start,Info &detail)
{
    vec3 dVector = D_Eye2View*detail.direction.normalize();
    vec3 digitLength = dVector*tan(detail.FOV/2*M_PI/180)*2/detail.w;
    viewX *= digitLength.length();
    viewY *= digitLength.length();
    vec3 leftTop  = detail.eye + dVector - viewX*detail.w/2 - viewY*detail.h/2;//the leftmost up-most vertex
    start = leftTop + viewX/2 - viewY/2;//start from the 'center' of the digit(space), but not vertex(line)
}

Sphere* IntersectWithSphere(vec3 &point,vec3 &rayVec,Info &detail,float &t)
{
    Sphere * ret = NULL;
    for(unsigned int s=0; s<detail.sph.size(); ++s)
    {
        vec3 a = point-detail.sph[s].origin;
        vec3 two = prod(rayVec,rayVec), one = 2*prod(rayVec,a), zero = prod(a,a);
        const float Two = two[0]+two[1]+two[2];//the coefficient of power 2
        const float One = one[0]+one[1]+one[2];
        const float Zero = zero[0]+zero[1]+zero[2]-detail.sph[s].r*detail.sph[s].r;
        float temp_t = min( (-One+sqrt(One*One-4*Two*Zero))/2/Two,
                            (-One-sqrt(One*One-4*Two*Zero))/2/Two );//if there are two t, choose smaller one because the big one in at back
        if(temp_t<t && temp_t>0)
        {
            t=temp_t;
            ret = &detail.sph[s];
        }
    }
    return ret;
}

Triangle* IntersectWithTriangle(vec3 &point,vec3 &rayVec,Info &detail,float &t)
{
    Triangle * ret = NULL;
    for(unsigned int r=0; r<detail.tri.size(); ++r)
    {
        /*
        Triangle: (x,y,z)= origin + s1*v1 +s2*v2
        Ray:      (x,y,z)= eye + t(xd,yd,zd), (xd,yd,zd) = digit-eye = rayVec
               -->s1*v1 + s2*v2 - t*(digit-eye) = eye-origin

        s1*v1.x + s2*v2.x - t*(digit-eye).x = (eye-origin).x
        s1*v1.y + s2*v2.y - t*(digit-eye).y = (eye-origin).y
        s1*v1.z + s2*v2.z - t*(digit-eye).z = (eye-origin).z
        Want to know s1,s2 and t

        Solve it with Gauss-Jordan elimination
        [v1.x v2.x (digit-eye).x]   [s1]   [ (eye-origin).x]
        [v2.y v2.y (digit-eye).y] * [s2] = [ (eye-origin).y]
        [v2.z v2.z (digit-eye).z]   [t ]   [ (eye-origin).z]*/

        Triangle now = detail.tri[r];
        vec3 rightSystem = point - now.origin;
        vec3 v1 = now.v1, v2 = now.v2, v3 = -rayVec;
        mat3 m = mat3(vec3(v1[0],v2[0],v3[0]),
                      vec3(v1[1],v2[1],v3[1]),
                      vec3(v1[2],v2[2],v3[2]));
        //PrintVec3(now.v1);
        //PrintVec3(now.v2);
        //PrintVec3(-digit+eye);
        vec3 ans = m.inverse(rightSystem);

        if(ans[0]>0&&ans[1]>0 && ans[0]+ans[1]<=1 && ans[2]>0+ERROR)//0<s1,s2<1 , t>0
            if(ans[2]<t)//ans[2] is t
            {
                t = ans[2];
                ret = &detail.tri[r];
            }
    }
    return ret;
}
void PhongShading(Material* m,vector<Light> &light,vec3 surface,vec3 N,vec3 surfaceToEye,
                  float reflectRate,Info &detail,Pixel *pix)
{
    N.normalize(),surfaceToEye.normalize();
    float zero = 0,r=0,g=0,b=0,colorLimit=255;
    float Ii = 1*reflectRate;//suppose light intensity
    for(unsigned int L=0; L<light.size(); ++L)
    {
        //If there is any object blocks between surface and light -> black
        vec3 lightV = light[L].position - surface;
        lightV.normalize();
        float t=9999,u=9999;
        Sphere * closestSphere = IntersectWithSphere(surface,lightV,detail,t);
        if(closestSphere)continue;//stop earlier when found an sphere obstacle
        Triangle * closestTriangle = IntersectWithTriangle(surface,lightV,detail,u);//if(closestTriangle)printf("TTasd\n");
        if(closestTriangle)continue;

        //Ambient, Ka*Ia
        r += m->Ka*m->r, g += m->Ka*m->g, b += m->Ka*m->b;

        //Diffuse, Id = kd*max(0, N dot L)*Ld
        //If N dot L< 0, then the point is on the dark side of the object.
        r += m->Kd*max(zero,N*lightV)*m->r*Ii;
        g += m->Kd*max(zero,N*lightV)*m->g*Ii;
        b += m->Kd*max(zero,N*lightV)*m->b*Ii;

        //Specular, Is = Ks*Ii(N dot H)^n
        vec3 H = (lightV+surfaceToEye)/2;
        float dot_NH=N*H;
        float emphize = 50;
        r += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii*emphize;
        g += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii*emphize;
        b += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii*emphize;

        r+=pix->R,g+=pix->G,b+=pix->B;
        pix->R = min(r,colorLimit);//pixel value should not exceed 255
        pix->G = min(g,colorLimit);
        pix->B = min(b,colorLimit);
    }
}

void RecursiveRayTracing(Info &detail,Pixel *pix,vec3 point,vec3 rayVec,
                         int recursiveLimit,float lastMtrReflect)
{
    for(int iteration=0; iteration<RECUR_LIMIT; ++iteration)
   {
        //determine whether rayVec intersects with spheres or triangles
        float t = INT_MAX,u = INT_MAX;
        Sphere * closestSphere = IntersectWithSphere(point,rayVec,detail,t);
        Triangle * closestTriangle = IntersectWithTriangle(point,rayVec,detail,u);

        vec3 intersection , reflectVec;
        if( t!=INT_MAX&&t<u&&closestSphere )//t>0 implies that eye can see it
        {
            float reflectRate = pow(lastMtrReflect,iteration);
            if(reflectRate<REFLECT_LIMIT)return;
            lastMtrReflect = closestSphere->mtr.Reflect;//update to next material reflect

            intersection = point + t*rayVec;
            vec3 N =intersection-closestSphere->origin;
            N.normalize();
            reflectVec = rayVec - 2*(rayVec*N)*N;

            //PhongShading(material,light , intersection,N vector,surfaceToLaunchPoint,pixel)
            PhongShading(&closestSphere->mtr,detail.lig,
                         intersection, N,
                         point-intersection,reflectRate,detail,pix);
        }

        else if( u!=INT_MAX&&t>u&&closestTriangle)
        {
            float reflectRate = pow(lastMtrReflect,iteration);
            if(reflectRate<REFLECT_LIMIT)return;
            lastMtrReflect = closestTriangle->mtr.Reflect;

            intersection = point + u*rayVec;
            vec3 N = closestTriangle->n;
            reflectVec = rayVec - 2*(rayVec*N)*N;
            if( N*(point-intersection)<=0)return;//N may be a vector not toward to eye

            //PhongShading(material,light ,surface,N vector,surfaceToEye,pixel)
            PhongShading(&closestTriangle->mtr,detail.lig,
                         intersection, N,point-intersection,reflectRate,detail,pix);
        }
        else return;//no any hit point
        if(closestSphere||closestTriangle)
            point=intersection,rayVec=reflectVec;
    }
}

void RayIntersection(Info &detail,ColorImage &image)
{
    vec3 dir = detail.direction,eye = detail.eye;
    vec3 viewX,viewY;
    SetViewXY(viewX,viewY,dir);//find the unit vector of x,y of the view

    vec3 start;//start from left-up most digit , and calculate the digit vector of x and y
    SetViewPlane(viewX,viewY,start,detail);

    for(int i=0; i<detail.h; ++i)
    {
        printf("i=%d \n",i);
        for(int j=0; j<detail.w; ++j)
        {
            //printf("i=%d j=%d\n",i,j);
            Pixel  pix = {0,0,0};
            vec3 digit = start + viewX*j + viewY*i;
            vec3 rayVec = digit - eye;//with coefficient 't'
            RecursiveRayTracing(detail,&pix,eye,rayVec,0,1);
            image.writePixel(j, i, pix);
        }
    }
    image.outputPPM("result.ppm");

}
