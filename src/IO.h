#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

#include"Object.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void ReadFile(Info &detail,const char fileName[]);
void ReadMTL(Info &detail,const string &mtlFileName);
void MakeTriangle(Info &detail,vector<Index>&index,vector<vec3>&vertex,vector<vec3>& vt,Material *mptr);
void PrintInfo( Info detail);
void PrintVec3(vec3 v);

struct Pixel {
	unsigned char R, G, B;  // Blue, Green, Red
};

class ColorImage {
	Pixel *pPixel;
	int xRes, yRes;
public:
	ColorImage();
	~ColorImage();
	void init(int xSize, int ySize);
	void clear(Pixel background);
	Pixel readPixel(int x, int y);
	void writePixel(int x, int y, Pixel p);
	void outputPPM(char *filename);
};





#endif // IO_H_INCLUDED
