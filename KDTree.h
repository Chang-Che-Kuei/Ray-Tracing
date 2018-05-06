#ifndef KDTREE_H_INCLUDED
#define KDTREE_H_INCLUDED

#include<algorithm>
#include"Object.h"
#define DEPTH_LIMIT 100

struct BBox//bounding box
{
    vec3 minimum = vec3(INT_MAX,INT_MAX,INT_MAX),
         maximum = vec3(INT_MIN,INT_MIN,INT_MIN); //box extent

    bool IntersectWithRay(vec3 &point,vec3 &rayVec);
};

class KDTree
{
private:
    KDTree *left=nullptr,*right=nullptr;
    vector<Triangle*> kdTri;
    BBox box;

public:
    KDTree* Build(vector<Triangle*>&tri,BBox bbox,int depth);
    void IntersectWithTriangle(KDTree* node,vec3 &point,vec3 &rayVec,
                       float &t,Triangle **nearestTri);
};

int FindLongestAxis(vec3& midPoint);
void InitTriAndBox(vector<Triangle>&tri,vector<Triangle*>&all,BBox &box);
#endif // KDTREE_H_INCLUDED
