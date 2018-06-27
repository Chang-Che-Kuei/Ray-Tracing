#include"algebra3.h"
#include"Object.h"
#include"IO.h"
#include"RayProcess.h"
#include"KDTree.h"
#include<time.h>
#include<limits>
int main()
{
    time_t s = clock();

    Info detail;
    detail.img.resize(100);//default give 100 image of space
    char fileName[200] = "Input/Banana.obj";
    ReadFile(detail,fileName);
//cout<<"asd\n";

    float VT1=0,VT2=0;
    for(int i=0; i<detail.tri.size(); ++i)
        for(int j=0; j<3; ++j)
        {
            VT1= max(VT1,detail.tri[i].texture[j][0]);
            VT2 = max(VT2,detail.tri[i].texture[j][1]);
        }
    printf("VT %f %f\n",VT1,VT2);

cout<<tan(45*M_PI/180)<<endl;
    printf("Triangle=%d\n",detail.tri.size());
    KDTree *kdtree;
    BBox rootBox;
    vector<Triangle*> allTri;
    InitTriAndBox(detail.tri,allTri,rootBox);//Initialize bbox
    kdtree = kdtree->Build(allTri,rootBox,0);//0 is depth, for debugging

    ColorImage image;
    image.init(detail.w, detail.h);
    RayIntersection(detail,image,kdtree);

    time_t e =clock();
    printf("running time %f",((float)(e-s)/(float)CLOCKS_PER_SEC) );
    return 0;

}
