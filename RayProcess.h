#ifndef RAYPROCESS_H_INCLUDED
#define RAYPROCESS_H_INCLUDED

#include"Object.h"
#include"IO.h"

void PhongShading(Material m,vec3 lightV,vec3 surface,vec3 N,vec3 surfaceToEye,Pixel *pix);
void RayIntersection(Info &detail,ColorImage &image);

#endif // RAYPROCESS_H_INCLUDED
