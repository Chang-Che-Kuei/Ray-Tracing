#ifndef KDTREE_H_INCLUDED
#define KDTREE_H_INCLUDED

#include<algorithm>
#include"Object.h"
#include"IO.h"
#define DEPTH_LIMIT 100

struct BBox//bounding box
{
    vec3 minimum = vec3(),
         maximum = vec3(INT_MIN,INT_MIN,INT_MIN); //box extent

    bool IntersectWithRay(vec3 &point,vec3 &rayVec);
};

class KDTree
{
private:
    KDTree *left=nullptr,*right=nullptr;
    BBox box;
public:
    vector<Triangle*> kdTri;
    vec3 GetBoxCenter()
    {
        return vec3( (box.maximum[0]+box.minimum[0])/2,
                    (box.maximum[1]+box.minimum[1])/2,
                    (box.maximum[2]+box.minimum[2])/2 );
    }
    KDTree* Build(vector<Triangle*>&tri,BBox &bbox,int depth);
    void FindIntersectionNodes(KDTree* node,vec3 &point,vec3 &rayVec,
                       float &t,vector<KDTree*>&nodes);
};

int FindLongestAxis(vec3& midPoint);
void InitTriAndBox(vector<Triangle>&tri,vector<Triangle*>&all,BBox &box);
#endif // KDTREE_H_INCLUDED
