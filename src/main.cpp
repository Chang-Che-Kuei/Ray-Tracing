#include"algebra3.h"
#include"Object.h"
#include"IO.h"
#include"RayProcess.h"
#include"KDTree.h"
#include<time.h>
int main(){
    time_t s = clock();

    Info detail;
    detail.img.resize(100);//default give 100 image of space
    char fileName[20] = "Input/test.obj";
    ReadFile(detail,fileName);
for(unsigned int i=0;i<detail.img.size();++i)
{
    printf("%s\n",detail.img[i].title.c_str());
    if(detail.img[i].img.empty())printf("!!\n");
}

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
