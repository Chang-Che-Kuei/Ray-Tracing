#include"RayProcess.h"
#include<algorithm>
#include<math.h>
using namespace std;
#define D_Eye2View 0.1 // distance between eye and view
#define RECUR_LIMIT 1
#define REFLECT_LIMIT 0.01

#define ERROR 1e-3

int gloX=0,gloY=0;
void SetViewXY(vec3 &x,vec3 &y,vec3 dir)
{
    //suppose the vision's x vector is vertical with z axis
    x = vec3(dir[1],-dir[0],0);
    if(x[0]==0&&x[1]==0)x[0]=1;//prevent (0,0,0)
    y = dir^x;//outer product
//PrintVec3(y);
    x.normalize(),y.normalize();
}

void SetViewPlane(vec3 &viewX,vec3 &viewY,vec3 &start,Info &detail)
{
    vec3 dVector = D_Eye2View*detail.direction.normalize();
    vec3 digitLength = dVector*tan(detail.FOV/2*M_PI/180)*2/detail.w;
    viewX *= -digitLength.length();
    viewY *= -digitLength.length();
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

void ConvertTo2D(Triangle* tri,vec3& surface,Mat &M,Mat& Tri2D,vec2& surface2D)
{
    /*
        (x, y, z) -> (ax+by+cz, dx+ey+fz)
            Tri3D   *     M    =    Tri2D
        [ x1 y1 z1 ]   [ a d ]   [ x1' y1' ]
        [ x2 y2 z2 ] * [ b e ] = [ x2' y2' ]
        [ x3 y3 z3 ]   [ c f ]   [ x3' y3' ]

        Convert 3D triangle to 2D space and one of
        the vertex (x1',y1') is (0,0), the another
        vertex is located at x axis (L12,0) where L12
        is the distance between vertex 1 and 2.
        The third vertex can be get by using sine and
        cosine.

    */
    Mat Tri3D(3,3,CV_32F);
    for(int i=0;i<3;++i)
        for(int j=0;j<3;++j)
            Tri3D.at<float>(i,j)=tri->p[i][j];

    float L12 = (tri->p[0]-tri->p[1]).length();
    float L13 = (tri->p[0]-tri->p[2]).length();
    float L23 = (tri->p[1]-tri->p[2]).length();

    float cosine = (L12*L12 + L13*L13 - L23*L23)/(2*L12*L13);
    float angle = acos(cosine);
    float p3X = L13*cos(angle),
          p3Y = L13*sin(angle);
    //build Tri2D
    Tri2D.at<float>(0,0)=0;
    Tri2D.at<float>(0,1)=0;

    Tri2D.at<float>(1,0)=L12;
    Tri2D.at<float>(1,1)=0;

    Tri2D.at<float>(2,0)=p3X;
    Tri2D.at<float>(2,1)=p3Y;

    Mat inverse(3,3,CV_32F);
    invert(Tri3D,inverse,CV_SVD  );
    M = inverse * Tri2D;

//printf("M %f %f %f\n",M[2][0],M[2][1],M[2][2]);
    //move 3D point "surface" to 2D point
    surface2D[0]=surface2D[1]=0;
    for(int row =0;row<2;++row)
        for(int i=0;i<3;++i)
        surface2D[row]+=M.at<float>(i,row)*surface[i];
 //   cout<<"sur "<<surface2D[0]<<" "<<surface2D[1]<<endl;

}

void TriangleAffine(Mat& Tri2D,vec2 &surface2D,Triangle* tri)
{
    /*Affine Tri2D to texture triangle
        Affine   *    Tri3*3    =   TextureTri
    [ t1 t2 t3 ]   [ x1 x2 x3 ]   [ x1' x2' x3' ]
    [ t4 t5 t6 ] * [ y1 y2 y3 ] = [ y1' y2' y3' ]
    [ 0  0  1  ]   [ 1  1  1  ]   [  1   1   1  ]
    Affine = TextureTri * inverse(Tri33)
    */
    Mat Tri33 = Mat(3,3,CV_32F);
    for(int i=0;i<2;++i)
        for(int j=0;j<3;++j)
            Tri33.at<float>(i,j) = Tri2D.at<float>(j,i);
    for(int i=0;i<3;++i)Tri33.at<float>(2,i)=1;
    Mat inverseTri33 = Mat(3,3,CV_32F);
    invert(Tri33,inverseTri33,CV_SVD);
//cout<<"AFF1\n";
    Mat textureTri(3,3,CV_32F);
    for(int i=0;i<2;++i)
        for(int j=0;j<3;++j)
            textureTri.at<float>(i,j) = tri->texture[j][i];
    for(int i=0;i<3;++i)textureTri.at<float>(2,i) = 1;

//printf("text %f %f \n",tri->texture[0][0],tri->texture[0][1]);
    Mat affine = textureTri * inverseTri33;
/*if(gloX%10==0){printf("Affine %d %d\n",gloX,gloY);
for(int i=0;i<3;++i)
{
    for(int j=0;j<3;++j)
     printf("%f ",affine.at<float>(i,j));
    printf("\n");
}}*/
    //change surface2D to the point after affine
    vec3 surfaceVec3 = vec3(surface2D[0],surface2D[1],1);
    vec3 surfaceAffine = vec3(0,0,0);
    for(int i=0;i<2;++i)
        for(int j=0;j<3;++j)
            surfaceAffine[i] += affine.at<float>(i,j)*surfaceVec3[j];
    surface2D[0]=surfaceAffine[0];
    surface2D[1]=surfaceAffine[1];
//if(surface2D[0]<0||surface2D[1]<0)
 // cout<<"After Affine "<<surface2D[0]<<" "<<surface2D[1]<<endl;
   /* for(int i=0;i<2;++i)
        PrintVec3(Tri2D[i]);
        for(int i=0;i<2;++i)
        PrintVec3(textureTri[i]);*/
}

void GetTexturePoint(int &x,int& y,Material* m,vec2& surface2D)
{
//if(m->imgKd==nullptr)printf("?????\n");
    int rows = m->imgKd->rows;
    int cols = m->imgKd->cols;
    //printf("rows=%d cols=%d surf %f %f\n",
      //     rows,cols,surface2D[0],surface2D[1]);
    x = surface2D[0] * cols ;
    y = surface2D[1] * rows ;
x=abs(x),y=abs(y);
}

bool IsIntersect(vec3 &point,vec3& rayVec,vector<KDTree*> &nodes)
{
    for(unsigned int n=0;n<nodes.size();++n)
        for(unsigned int i=0; i<nodes[n]->kdTri.size(); ++i)//check ray-triangle intersection
            {
//tri++;
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
                [v2.z v2.z (digit-eye).z]   [t ]   [ (eye-origin).z]
                */
                Triangle now = *(nodes[n]->kdTri[i]);
                vec3 rightSystem = point - now.p[0];
                vec3 v1 = now.v[0], v2 = now.v[1], v3 = -rayVec;
                mat3 m = mat3(vec3(v1[0],v2[0],v3[0]),
                              vec3(v1[1],v2[1],v3[1]),
                              vec3(v1[2],v2[2],v3[2]));

                vec3 ans = m.inverse(rightSystem);

                if(ans[0]>0&&ans[1]>0 && ans[0]+ans[1]<=1 && ans[2]>0+ERROR)//0<s1,s2<1 , t>0
                    {
//printf("n=%d n all = %d\n",n,nodes.size());
                        return true;
                    }
            }
    return false;
}

void PhongShading(Triangle *tri,Material* m,vector<Light> &light,vec3 surface,vec3 N,vec3 surfaceToEye,
                  float reflectRate,Info &detail,Pixel *pix,KDTree* kdtree)
{
    N.normalize(),surfaceToEye.normalize();
    float r=0,g=0,b=0,colorLimit=255;
    float Ii = 1.6;//suppose light intensity

    //convert 3D triangle to 2D space
    Mat M(3,2,CV_32F),Tri2D(3,2,CV_32F);
    vec2 surface2D;
    ConvertTo2D(tri,surface,M,Tri2D,surface2D);

    //Get texture position
    TriangleAffine(Tri2D,surface2D,tri);
    int x=0,y=0;
    GetTexturePoint(x,y,m,surface2D);
    //cout<<surface2D[0]<<" "<<surface2D[1]<<endl;
    //cout<<x<<" "<<y<<endl;
//if(gloX==101)cout<<x<<" "<<y<<endl;
    //if(x>1024||y>1024)cout<<x<<" "<<y<<endl;
    for(unsigned int L=0; L<light.size(); ++L)
    {
        //Ambient, Ka*Ia
        //Mat imgKa = imread(m->map_Ka,CV_LOAD_IMAGE_UNCHANGED);
        if(m->imgKa!=NULL){
        r += m->Ka[0]*m->imgKa->at<cv::Vec3b>(y,x)[2]*Ii;//opencv B-G-R
        g += m->Ka[1]*m->imgKa->at<cv::Vec3b>(y,x)[1]*Ii;
        b += m->Ka[2]*m->imgKa->at<cv::Vec3b>(y,x)[0]*Ii;
        }
        else{
            r += m->Ka[0]*Ii;//opencv B-G-R
            g += m->Ka[1]*Ii;
            b += m->Ka[2]*Ii;
        }
        //If there is any object blocks between surface and light -> black
        vec3 lightV = light[L].position - surface;
        lightV.normalize();
        float t=INT_MAX,u=INT_MAX;
        Sphere * closestSphere = IntersectWithSphere(surface,lightV,detail,t);
        if(closestSphere)continue;//stop earlier when found an sphere obstacle
        vector<KDTree*> nodes;
        kdtree->FindIntersectionNodes(kdtree,surface,lightV,u,nodes);//if(closestTriangle)printf("TTasd\n");
        if( IsIntersect(surface,lightV,nodes) ){
//printf("hide %d\n",gloX);
                continue;
        }
//printf("%d %d\n",x,y);
        //Diffuse, Id = kd*abs(N dot L)*Ld
        //If N dot L< 0, then the point is on the dark side of the object. -->but only find the closest triangle could prevent this problem
        //Mat imgKd = imread(m->map_Kd,CV_LOAD_IMAGE_UNCHANGED);
        r += m->Kd[0]*abs(N*lightV)*m->imgKd->at<cv::Vec3b>(y,x)[2]*Ii;
        g += m->Kd[1]*abs(N*lightV)*m->imgKd->at<cv::Vec3b>(y,x)[1]*Ii;
        b += m->Kd[2]*abs(N*lightV)*m->imgKd->at<cv::Vec3b>(y,x)[0]*Ii;
//if(gloX>49)printf("N*lightV=%f\n",N*lightV);
        //Specular, Is = Ks*Ii(N dot H)^n
        vec3 H = (lightV+surfaceToEye)/(lightV+surfaceToEye).length();
        float dot_NH=abs(N*H);
        //float emphize = 50;
        r += m->Ks[0]*pow(dot_NH,m->Ns) *Ii;
        g += m->Ks[1]*pow(dot_NH,m->Ns) *Ii;
        b += m->Ks[2]*pow(dot_NH,m->Ns) *Ii;

    }
    r+=pix->R,g+=pix->G,b+=pix->B;
    pix->R = min(r,colorLimit);//pixel value should not exceed 255
    pix->G = min(g,colorLimit);
    pix->B = min(b,colorLimit);
//printf("r=%d g=%d b=%d\n",pix->R,pix->G,pix->B);
}


void FindClosetTri(vec3& point,vec3& rayVec,vector<KDTree*> &nodes,Triangle **nearestTri, float &t )
{


    //find the triangle which intersects with rayVec
    for(unsigned int n=0;n<nodes.size();++n)
        {
            for(unsigned int i=0; i<nodes[n]->kdTri.size(); ++i)//check ray-triangle intersection
            {
//tri++;
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
                [v2.z v2.z (digit-eye).z]   [t ]   [ (eye-origin).z]
                */
                Triangle now = *(nodes[n]->kdTri[i]);
                vec3 rightSystem = point - now.p[0];
                vec3 v1 = now.v[0], v2 = now.v[1], v3 = -rayVec;
                mat3 m = mat3(vec3(v1[0],v2[0],v3[0]),
                              vec3(v1[1],v2[1],v3[1]),
                              vec3(v1[2],v2[2],v3[2]));

                vec3 ans = m.inverse(rightSystem);

                if(ans[0]>0&&ans[1]>0 && ans[0]+ans[1]<=1 && ans[2]>0+ERROR)//0<s1,s2<1 , t>0
                    if(ans[2]<t)//ans[2] is t
                    {
                        t = ans[2];
                        *nearestTri = nodes[n]->kdTri[i];
                    }
            }
//if(intersection==true)
    //printf("n=%d n all = %d\n",n,nodes.size());
        }

}

void RecursiveRayTracing(Info &detail,Pixel *pix,vec3 point,vec3 rayVec,
                         float lastMtrReflect,KDTree *kdtree)
{
    for(int iteration=0; iteration<RECUR_LIMIT; ++iteration)
   {
        //determine whether rayVec intersects with spheres or triangles
        float t = INT_MAX,u = INT_MAX;
        Sphere * closestSphere = IntersectWithSphere(point,rayVec,detail,t);
        Triangle * closestTriangle = nullptr;
        vector<KDTree*> nodes;
//printf("Root box\n");
//PrintVec3(kdtree->box.maximum);
//PrintVec3(kdtree->box.minimum);
        kdtree->FindIntersectionNodes(kdtree,point,rayVec,u,nodes);
        FindClosetTri(point,rayVec,nodes,&closestTriangle,u);
//printf("has %d nodes\n",nodes.size());
//if(closestTriangle!=nullptr)printf("has triangle\n");
        vec3 intersection , reflectVec;
/*if(gloX>49)
{
    printf("X%d Y %d\n",gloX,gloY);
    if(closestTriangle==nullptr)printf("no");
    else printf("yes\n");
}*/
       /* if( t!=INT_MAX&&t<u&&closestSphere )//t>0 implies that eye can see it
        {
            float reflectRate = pow(lastMtrReflect,iteration);
            if(reflectRate<REFLECT_LIMIT)return;
            lastMtrReflect = closestSphere->mtr->Ks[0];//update to next material reflect

            intersection = point + t*rayVec;
            vec3 N =intersection-closestSphere->origin;
            N.normalize();
            reflectVec = rayVec - 2*(rayVec*N)*N;

            //PhongShading(material,light , intersection,N vector,surfaceToLaunchPoint,pixel,kdtree)
            PhongShading(closestSphere->mtr,detail.lig,
                         intersection, N,
                         point-intersection,reflectRate,detail,pix,kdtree);
        }*/

        if( u!=INT_MAX&&closestTriangle)
        {
//if(gloX>49)cout<<"ghere\n";
            float reflectRate = pow(lastMtrReflect,iteration);
            if(reflectRate<REFLECT_LIMIT)return;
            lastMtrReflect = closestTriangle->mtr->Ks[0];

            intersection = point + u*rayVec;
            vec3 N = closestTriangle->n;
            reflectVec = rayVec - 2*(rayVec*N)*N;
            //if( N*(point-intersection)<=0)return;//N may be a vector not toward to eye
            PhongShading(closestTriangle, closestTriangle->mtr,detail.lig,
                         intersection, N,point-intersection,reflectRate,detail,pix,kdtree);
        }
        else return;//no any hit point
        if(closestSphere||closestTriangle)
            point=intersection,rayVec=reflectVec;
    }
}

void RayIntersection(Info &detail,ColorImage &image,KDTree *kdtree)
{
    vec3 dir = detail.direction,eye = detail.eye;
    vec3 viewX,viewY;
    SetViewXY(viewX,viewY,dir);//find the unit vector of x,y of the view

    vec3 start;//start from left-up most digit , and calculate the digit vector of x and y
    SetViewPlane(viewX,viewY,start,detail);
cout<<detail.h<<" "<<detail.w<<endl;
    for(int i=0; i<detail.h; ++i,++gloX)
    {
printf("i=%d \n",i);
        for(int j=0; j<detail.w; ++j,++gloY)
        {
            //printf("i=%d j=%d\n",i,j);
            Pixel  pix = {0,0,0};
            vec3 digit = start + viewX*j + viewY*i;
//if(j%10==0)PrintVec3(digit);
            vec3 rayVec = digit - eye;//with coefficient 't'
            RecursiveRayTracing(detail,&pix,eye,rayVec,1,kdtree);
            image.writePixel(j, i, pix);
        }
gloY=0;
    }
    image.outputPPM("result.ppm");

}
