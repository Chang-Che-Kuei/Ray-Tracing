#ifndef RAYPROCESS_H_INCLUDED
#define RAYPROCESS_H_INCLUDED

#include"KDTree.h"
#include"Object.h"
#include"IO.h"

void SetViewXY(vec3 &x,vec3 &y,vec3 dir);
void SetViewPlane(vec3 &viewX,vec3 &viewY,vec3 &start,Info &detail);

Sphere* IntersectWithSphere(vec3 &digit,vec3 &rayVec,Info &detail,float &t);

void PhongShading(Material *m,vec3 lightV,vec3 surface,vec3 N,vec3 surfaceToEye,
                  float reflectRate,Info &detail,Pixel *pix,KDTree* kdtree);
void RecursiveRayTracing(Info &detail,Pixel *pix,vec3 point,vec3 rayVec,float lastMtrReflect,KDTree* kdtree);
void RayIntersection(Info &detail,ColorImage &image,KDTree *kdtree);

bool IsIntersect(vec3 &point,vec3& rayVec,vector<KDTree*> &nodes);
void FindClosetTri(vec3& point,vec3& rayVec,vector<KDTree*> &nodes,Triangle **nearestTri , float &t);
#endif // RAYPROCESS_H_INCLUDED
