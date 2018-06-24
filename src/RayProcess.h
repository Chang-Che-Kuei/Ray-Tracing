#ifndef RAYPROCESS_H_INCLUDED
#define RAYPROCESS_H_INCLUDED

#include"KDTree.h"
#include"Object.h"
#include"IO.h"
#include <opencv2/opencv.hpp>
using namespace cv;
void SetViewXY(vec3 &x,vec3 &y,vec3 dir);
void SetViewPlane(vec3 &viewX,vec3 &viewY,vec3 &start,Info &detail);

Sphere* IntersectWithSphere(vec3 &digit,vec3 &rayVec,Info &detail,float &t);

void PhongShading(Material *m,vec3 lightV,vec3 surface,vec3 N,vec3 surfaceToEye,
                  float reflectRate,Info &detail,Pixel *pix,KDTree* kdtree);
void RecursiveRayTracing(Info &detail,Pixel *pix,vec3 point,vec3 rayVec,float lastMtrReflect,KDTree* kdtree);
void RayIntersection(Info &detail,ColorImage &image,KDTree *kdtree);

bool IsIntersect(vec3 &point,vec3& rayVec,vector<KDTree*> &nodes);
void FindClosetTri(Triangle *tri,vec3& surface,vec3& point,vec3& rayVec,vector<KDTree*> &nodes,Triangle **nearestTri , float &t);
void ConvertTo2D(Triangle* tri,mat3 &M,mat3& Tri2D,vec2& surface2D);
void TriangleAffine(mat3& Tri2D,vec2 &surface2D,Triangle* tri);
void GetTexturePoint(int &x,int& y,Material* m,vec2& surface2D);
#endif // RAYPROCESS_H_INCLUDED
