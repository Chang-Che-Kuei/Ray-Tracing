

#include"KDTree.h"
#define ERROR 1e-6
#define MaxTriInNode 10
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
KDTree* KDTree::Build(vector<Triangle*>&tri,BBox &bbox,int depth)
{

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
    float split = bbox.minimum[axis] +axisLen[axis]/2;
    int same = 0;
    for(unsigned int i=0; i<tri.size(); ++i)
    {
        float m = min(min(tri[i]->origin[axis],tri[i]->p1[axis]),tri[i]->p2[axis]);
        float M = max(max(tri[i]->origin[axis],tri[i]->p1[axis]),tri[i]->p2[axis]);
        M = max(M,tri[i]->p2[axis]);

        if(m<=split)
            left_tri.push_back(tri[i]);
        if(M>=split)
            right_tri.push_back(tri[i]);
        if(m<=split&&M>=split)same++;

    }

    BBox bLeft,bRight;
    bLeft.minimum = bbox.minimum;   //build left bbox
    bLeft.maximum = bbox.maximum;
    bLeft.maximum[axis] = bbox.minimum[axis] + axisLen[axis]/2;
    bRight.minimum = bbox.minimum;    //build right bbox
    bRight.minimum[axis] = bbox.minimum[axis] + axisLen[axis]/2;
    bRight.maximum = bbox.maximum;
    if(left_tri.size()>MaxTriInNode&&same!=tri.size())//stop when no decrease
        node->left = Build(left_tri,bLeft,depth+1);
    else
    {
        node->left = new KDTree();//printf("leaf left=%4d %4d\n",left_tri.size(),depth);
        node->left->kdTri = left_tri;
        node->left->box = bLeft;
        node->left->left = new KDTree();
        node->left->right = new KDTree();
    }

    if(right_tri.size()>MaxTriInNode&&same!=tri.size())
        node->right = Build(right_tri,bRight,depth+1);
    else
    {
        node->right = new KDTree();//printf("leaf rigt=%4d %4d\n",right_tri.size(),depth);
        node->right->kdTri = right_tri;
        node->right->box = bRight;
        node->right->left = new KDTree();
        node->right->right = new KDTree();
    }
    return node;
}

void KDTree::IntersectWithTriangle(KDTree* node,vec3 &point,vec3 &rayVec,
                                   float &t,Triangle **nearestTri)
{
    if(box.IntersectWithRay(point,rayVec))
    {
        bool leftChildExist = false, rightChildExist = false;
        if(left->kdTri.size() > 0)
            leftChildExist=true,left->IntersectWithTriangle(left,point,rayVec,
                           t,nearestTri);
        if(right->kdTri.size() > 0)
            rightChildExist=true,right->IntersectWithTriangle(right,point,rayVec,
                            t,nearestTri);
        if(leftChildExist==false&&rightChildExist==false)//reach leaf node

            for(unsigned int i=0; i<kdTri.size(); ++i)//check ray-triangle intersection
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
                [v2.z v2.z (digit-eye).z]   [t ]   [ (eye-origin).z]
                */
                Triangle now = *kdTri[i];
                vec3 rightSystem = point - now.origin;
                vec3 v1 = now.v1, v2 = now.v2, v3 = -rayVec;
                mat3 m = mat3(vec3(v1[0],v2[0],v3[0]),
                              vec3(v1[1],v2[1],v3[1]),
                              vec3(v1[2],v2[2],v3[2]));

                vec3 ans = m.inverse(rightSystem);

                if(ans[0]>0&&ans[1]>0 && ans[0]+ans[1]<=1 && ans[2]>0+ERROR)//0<s1,s2<1 , t>0
                    if(ans[2]<t)//ans[2] is t
                    {
                        t = ans[2];
                        *nearestTri = kdTri[i];

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
        if (rayVec[dimen] != 0 )  //dimension x y z
        {
            double t1 = ( minimum[dimen] - point[dimen] ) / rayVec[dimen];
            double t2 = ( maximum[dimen] - point[dimen] ) / rayVec[dimen];
            tmin = max(tmin, min(t1, t2));
            tmax = min(tmax, max(t1, t2));
        }
    return tmax >= tmin;
}
