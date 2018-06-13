#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include"algebra3.h"
#include<vector>
#include<iostream>
using namespace std;

struct Material{
    float r, g, b, Ka, Kd, Ks, exp, Reflect, Refract, Nr;
    /*
    (r, g, b) is the surface color;0~1
    Ka, Kd, Ks are the coefficients of the ambient, diffuse, and specular components;
    exp is the specularity;
    Reflect and Refract are within the range of [0, 1] and represent the ratio of reflection of refraction;
    Nr is the density of object which affects refraction angles.
    */
};

struct Light{
    vec3 position;
};

struct Triangle
{
    vec3 origin,p1,p2,v1,v2,n;//vector 1,2
    Material mtr;
};

struct Sphere
{
    vec3 origin;
    float r;
    Material mtr;
};

struct Plane{
    vec3 origin,v1,v2;
};

struct Info{
    vec3 eye,direction;
    float FOV=0;
    int w,h;
    vector<Triangle>tri;
    vector<Sphere>  sph;
    vector<Light> lig;
};

#endif // OBJECT_H_INCLUDED
