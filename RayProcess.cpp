#include"RayProcess.h"
#include<algorithm>
#include<math.h>
using namespace std;


void PhongShading(Material* m,vec3 lightV,vec3 surface,vec3 N,vec3 surfaceToEye,Pixel *pix)
{
    N.normalize(),lightV.normalize(),surfaceToEye.normalize();
    float zero = 0,r=0,g=0,b=0,colorLimit=255;
    float Ii = 200;//suppose light intensity is (50,50,50)

    //Ambient, Ka*Ia
    r += m->Ka*m->r, g += m->Ka*m->g, b += m->Ka*m->b;

    //Diffuse, Id = kd*max(0, N¡PL)*Ld
    //cos £c = N¡PL, If N¡PL< 0, then the point is on the dark side of the object.
    r += m->Kd*max(zero,N*lightV)*m->r;
    g += m->Kd*max(zero,N*lightV)*m->g;
    b += m->Kd*max(zero,N*lightV)*m->b;

    //Specular, Is = Ks*Ii(N ¡P H)^n
    vec3 H = (lightV+surfaceToEye)/2;
    float dot_NH=N*H;
    r += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii;
    g += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii;
    b += m->Ks*max( zero,pow(dot_NH,m->exp) )*Ii;

    pix->R = min(r,colorLimit);//pixel value should not exceed 255
    pix->G = min(g,colorLimit);
    pix->B = min(b,colorLimit);
}


void RayIntersection(Info &detail,ColorImage &image)
{
    float d =0.1;//distance between eye and view
    vec3 dir = detail.direction,eye = detail.eye;
    //suppose the vision is vertical with z axis
    vec3 horizontal = vec3(min(float(-1),dir[1]),dir[0],0);//max() prevent (0,0,0)
    vec3 vertical = dir^horizontal;//outer product

    //get the plane of image
    vec3 temp = dir,temp2;
    vec3 dVector = d*temp.normalize();
    vec3 halfVisionLength = dVector*tan(detail.FOV/2*M_PI/180);
    horizontal = halfVisionLength.length()*horizontal.normalize();
    vertical = halfVisionLength.length()*vertical.normalize();
    vec3 leftTop = eye+dVector-horizontal+vertical,//three vertices of the plane
         leftDown = eye+dVector-horizontal-vertical,
         rightTop = eye+dVector+horizontal+vertical;
    Plane p;
    p.origin = leftTop,p.v1=rightTop-leftTop,p.v2 = leftDown-leftTop;//the info of the view plane

    float digitDistanceW = halfVisionLength.length()*2/detail.w,//width distance per digit
          digitDistanceH = halfVisionLength.length()*2/detail.h;
    temp = p.v1,temp2=p.v2;
    vec3 halfDigitVecW = digitDistanceW/2*temp.normalize();//half width vector per digit
    vec3 halfDigitVecH = digitDistanceH/2*temp2.normalize();

    vec3 start = leftTop + halfDigitVecW + halfDigitVecH;//start from the 'center' of the digit(space), but not vertex(line)
    for(int i=0; i<256; ++i)
    {
        for(int j=0; j<256; ++j)
        {
            Pixel  pix = {0,0,0};
            vec3 digit = start + halfDigitVecW*j*2 + halfDigitVecH*i*2;
            vec3 rayVec = digit - eye;//with coefficient
            //determine whether intersect with spheres
            bool intersect = false;
            for(unsigned int s=0; s<detail.sph.size()&&intersect==false; ++s)
            {
                vec3 a = eye-detail.sph[s].origin;
                vec3 two = prod(rayVec,rayVec), one = 2*prod(rayVec,a), zero = prod(a,a);
                const float Two = two[0]+two[1]+two[2];//the coefficient of power 2
                const float One = one[0]+one[1]+one[2];
                const float Zero = zero[0]+zero[1]+zero[2]-detail.sph[s].r*detail.sph[s].r;
                float t = min( (-One+sqrt(One*One-4*Two*Zero))/2/Two,
                               (-One-sqrt(One*One-4*Two*Zero))/2/Two );//if there are two t, choose smaller one because the big one in at back
                if( t>0)//t>0 implies that eye can see it
                {
                    vec3 intersection = eye + t*rayVec;
                    //PhongShading(material,light vector, intersection,N vector,surfaceToEye,pixel)
                    PhongShading(&detail.sph[s].mtr,detail.lig[0].position-intersection,
                                 intersection, intersection-detail.sph[s].origin,
                                 eye-intersection,&pix);
                    intersect=true;
                }

            }

            for(unsigned int r=0; r<detail.tri.size()&&intersect==false; ++r)
            {
                /*
                Triangle: (x,y,z)= origin + s1*v1 +s2*v2
                Ray:      (x,y,z)= eye + t(xd,yd,zd), (xd,yd,zd) = digit-eye
                       -->s1*v1 + s2*v2 - t*(digit-eye) = eye-origin

                s1*v1.x + s2*v2.x - t*(digit-eye).x = (eye-origin).x
                s1*v1.y + s2*v2.y - t*(digit-eye).y = (eye-origin).y
                s1*v1.z + s2*v2.z - t*(digit-eye).z = (eye-origin).z
                Want to know s1,s2 and t

                Solve it with Gauss-Jordan elimination
                [v1.x v2.x (digit-eye).x]   [s1]   [ (eye-origin).x]
                [v2.y v2.y (digit-eye).y] * [s2] = [ (eye-origin).y]
                [v2.z v2.z (digit-eye).z]   [t ]   [ (eye-origin).z]
                */
                Triangle now = detail.tri[r];
                vec3 rightSystem = eye - now.origin;
                vec3 v1 = now.v1, v2 = now.v2, v3 = -digit+eye;
                mat3 m = mat3(vec3(v1[0],v2[0],v3[0]),
                              vec3(v1[1],v2[1],v3[1]),
                              vec3(v1[2],v2[2],v3[2]));
                //PrintVec3(now.v1);
                //PrintVec3(now.v2);
                //PrintVec3(-digit+eye);
                vec3 ans = m.inverse(rightSystem);

                if(ans[0]>0&&ans[1]>0&&ans[0]+ans[1]<=1&&ans[2]>0)//0<s1,s2<1 , t>0
                {
                    vec3 intersection = eye + ans[2]*(digit-eye);//ans[2] is t
                    vec3 N = now.v1^now.v2;
                    //N may be a vector not toward to eye
                    if( N*(eye-intersection)<0)N=-N;//Dot Product
                    //PhongShading(material,light vector,surface,N vector,surfaceToEye,pixel)
                    PhongShading(&detail.tri[r].mtr,detail.lig[0].position-intersection,
                                 intersection, N,eye-intersection,&pix);
                    intersect=true;
                }
            }
            image.writePixel(j, i, pix);
        }
    }
    image.outputPPM("result.ppm");

}
