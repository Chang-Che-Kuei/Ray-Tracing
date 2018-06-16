#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include"algebra3.h"
#include<vector>
#include<iostream>
using namespace std;

struct Material{
    string title;
    float Ns,Ni,d,Tr,Tf,illum;
    vec3 Ka,Kd,Ks,Ke;
    string map_Ka,map_Kd,map_d,map_bump;
    /* ex:
    Ns 10.0000
	Ni 1.5000
	d 1.0000
	Tr 0.0000
	Tf 1.0000 1.0000 1.0000
	illum 2
	Ka 0.5880 0.5880 0.5880
	Kd 0.5880 0.5880 0.5880
	Ks 0.0000 0.0000 0.0000
	Ke 0.0000 0.0000 0.0000
	map_Ka textures\sponza_thorn_diff.png
	map_Kd textures\sponza_thorn_diff.png
	map_d textures\sponza_thorn_mask.png
	map_bump textures\sponza_thorn_bump.png
    */
};
struct Index{
    int v,vt;
};
struct Light{
    vec3 position;
};

struct Triangle
{
    vec3 origin,p1,p2,v1,v2,n;//vector 1,2
    vec3 texture1,texture2,texture3;
    Material *mtr;
};

struct Sphere
{
    vec3 origin;
    float r;
    Material *mtr;
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
    vector<Material> mtl;
};

#endif // OBJECT_H_INCLUDED
