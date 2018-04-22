#include"algebra3.h"
#include"Object.h"
#include"IO.h"
#include"RayProcess.h"
#include<time.h>
int main(){
    time_t s =clock();

    Info detail;
    char fileName[20] = "input.txt";
    ReadFile(detail,fileName);
    //printf("t=%d\n",detail.tri.size());
   // printf("%f %d ss\n",detail.sph[0].p_mtr->r,detail.sph.size());
    //PrintInfo(detail);
    ColorImage image;
	image.init(detail.w, detail.h);
    RayIntersection(detail,image);

    time_t e =clock();
    printf("running time %f",(float)(e-s)/1000);
    return 0;
}
