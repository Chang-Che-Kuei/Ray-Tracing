

#include"KDTree.h"
#define MaxTriInNode 50
enum Axis {X,Y,Z};//X=0 Y=1 Z=2

void InitTriAndBox(vector<Triangle>&tri,vector<Triangle*>&all,BBox &box)
{
    for(unsigned int i=0; i<tri.size(); ++i)
    {
        all.push_back(&tri[i]);
        for(int axis=0; axis<3; ++axis)
        {
            box.minimum[axis] = min(box.minimum[axis],tri[i].origin[axis]);
            box.minimum[axis] = min(box.minimum[axis],tri[i].p1[axis]);
            box.minimum[axis] = min(box.minimum[axis],tri[i].p2[axis]);

            box.maximum[axis] = max(box.maximum[axis],tri[i].origin[axis]);
            box.maximum[axis] = max(box.maximum[axis],tri[i].p1[axis]);
            box.maximum[axis] = max(box.maximum[axis],tri[i].p2[axis]);
        }
    }
}

int FindLongestAxis(vec3& midPoint)
{
    float arr[3]= {midPoint[0],midPoint[1],midPoint[2]};
    int axis = distance(arr, max_element(arr, arr + 3));
    return axis;
}

int times = 0,depth = 0;
KDTree* KDTree::Build(vector<Triangle*>&tri,BBox bbox,int depth)
{
    //  printf("times=%d tri=%d\n",times++,tri.size());
    KDTree *node = new KDTree();
    node->kdTri = tri;
    node->box = bbox;

    if(tri.size()==0)
        return node;

    //find longest axis
    vec3 axisLen = vec3();//0 0 0
    for(int i=0; i<3; ++i)
        axisLen[i] = bbox.maximum[i] - bbox.minimum[i];
    int axis = FindLongestAxis(axisLen);

    //split triangles at center based on longest axis
    vector<Triangle*> left_tri;
    vector<Triangle*> right_tri;
    float split = (bbox.minimum[axis] + bbox.maximum[axis])/2;
    for(unsigned int i=0; i<tri.size(); ++i)
    {
        float m = min(tri[i]->origin[axis],tri[i]->p1[axis]);
        m = min(m,tri[i]->p2[axis]);
        float M = max(tri[i]->origin[axis],tri[i]->p1[axis]);
        M = max(M,tri[i]->p2[axis]);
        //printf("i=%d m=%f M=%f split=%f\n",i,m,M,split);
        if(m<=split)
            left_tri.push_back(tri[i]);
        if(M>=split)
            right_tri.push_back(tri[i]);
    }
//printf("left =%d right=%d split=%f %d\n",left_tri.size(),right_tri.size(),split,axis);

    BBox bLeft,bRight;
    if(left_tri.size()>MaxTriInNode&&left_tri.size()!=right_tri.size())//stop when no much divide
    {
        bLeft.minimum = bbox.minimum;   //build bbox
        bLeft.maximum = bbox.maximum;
        bLeft.maximum[axis] = bbox.minimum[axis] + axisLen[axis]/2;
//printf("left\n");
        node->left = Build(left_tri,bLeft,depth+1);
    }
    else
        node->left = new KDTree(),printf("leaf left=%4d %4d\n",left_tri.size(),depth);

    if(right_tri.size()>MaxTriInNode&&left_tri.size()!=right_tri.size())
    {
        bRight.minimum = bbox.minimum;    //build bbox
        bRight.minimum[axis] = bbox.minimum[axis] + axisLen[axis]/2;
        bRight.maximum = bbox.maximum;
//printf("right\n");
        node->right = Build(right_tri,bRight,depth+1);
    }
    else
        node->right = new KDTree(),printf("leaf rigt=%4d %4d\n",right_tri.size(),depth);
    return node;
}

void KDTree::IntersectWithTriangle(KDTree* node,vec3 &point,vec3 &rayVec,
                                   float &t,Triangle **nearestTri)
{// !! don't need argument KDTree* node.
    if(box.IntersectWithRay(point,rayVec))
    {
        bool hisTri = false;
        bool hasTri = false;
        if(node->left->kdTri.size() > 0)
            hasTri=true,IntersectWithTriangle(node->left,point,rayVec,
                                       t,nearestTri);
        if(node->right->kdTri.size() > 0)
            hasTri=true,IntersectWithTriangle(node->right,point,rayVec,
                                       t,nearestTri);
        if(hasTri)
        {
            for(unsigned int i=0;i<kdTri.size();++i)
            {

            }
        }
    }
}


//struct BBox function
//Ray - Box Intersection, "slabs" algorithm -- Kay and Kayjia
bool BBox::IntersectWithRay(vec3 &point,vec3 &rayVec)
{
    double tmin = INT_MIN, tmax = INT_MAX;
    for(int dimen=0; dimen<3; dimen++)
        if (rayVec[dimen] != 0 )  //x
        {
            double t1 = ( minimum[dimen] - point[dimen] ) / rayVec[dimen];
            double t2 = ( maximum[dimen] - point[dimen] ) / rayVec[dimen];

            tmin = max(tmin, min(t1, t2));
            tmax = min(tmax, max(t1, t2));
        }
    return tmax >= tmin;
}
