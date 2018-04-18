#include"IO.h"
#include<stdio.h>

void ReadFile(Info &detail,const char fileName[])
{
    FILE *fp =fopen(fileName,"r");
    if(fp==NULL)
    {
        cout<<"Cannot find this file."<<endl;
        return ;
    }

    char inputTarget;
    while(fscanf(fp," %c",&inputTarget)!=EOF)
    {
        Material m;
        switch(inputTarget)
        {
        case 'E':
            fscanf(fp,"%f%f%f",&detail.eye[0],
                   &detail.eye[1],&detail.eye[2]);
            break;
        case 'V':
            fscanf(fp,"%f%f%f",&detail.direction[0],
                   &detail.direction[1],&detail.direction[2]);
            break;
        case 'F':
            fscanf(fp,"%f",&detail.FOV);
            break;
        case 'R':
            fscanf(fp,"%d%d",&detail.w,&detail.h);
            break;
        case 'S':
        {
            Sphere s;
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&s.origin[i]);
            fscanf(fp,"%f",&s.r);
            s.mtr = m;
            detail.sph.push_back(s);
            //printf("%f %f %f ??\n",s.p_mtr->exp,s.p_mtr->Nr,s.p_mtr->b);
            break;
        }
        case 'T':
        {
            vec3 vertex[3];
            for(int i=0; i<3; ++i)
                for(int j=0; j<3; ++j)
                    fscanf(fp,"%f",&vertex[i][j]);
            Triangle t;
            t.origin = vertex[0];
            t.v1 = vertex[1] - t.origin;
            t.v2 = vertex[2] - t.origin;
            t.mtr = m;
            detail.tri.push_back(t);
            break;
        }
        case 'L':
        {
            Light temp;
            fscanf(fp,"%f %f %f",&temp.position[0],&temp.position[1],&temp.position[2]);
            detail.lig.push_back(temp);
            break;
        }
        case 'M':
        {
            fscanf(fp,"%f %f %f %f %f %f %f %f %f %f",
                   &m.r,&m.g,&m.b,&m.Ka,&m.Kd,&m.Ks,
                   &m.exp,&m.Reflect,&m.Refract,&m.Nr);
            //scale input rgb from 0~1 to 0~255
            m.r*=255,m.g*=255,m.b*=255;
            break;
        }
        }
    }
    fclose(fp);
}

ColorImage::ColorImage()
{
    pPixel = 0;
}

ColorImage::~ColorImage()
{
    if (pPixel)
        delete[] pPixel;
    pPixel = 0;
}

void ColorImage::init(int xSize, int ySize)
{
    Pixel p = {0,0,0};
    xRes = xSize;
    yRes = ySize;
    pPixel = new Pixel[xSize*ySize];
    clear(p);
}

void ColorImage::clear(Pixel background)
{
    int i;

    if (! pPixel)
        return;
    for (i=0; i<xRes*yRes; i++)
        pPixel[i] = background;
}

Pixel ColorImage::readPixel(int x, int y)
{
    assert(pPixel); // die if image not initialized
    return pPixel[x + y*yRes];
}

void ColorImage::writePixel(int x, int y, Pixel p)
{
    assert(pPixel); // die if image not initialized
    pPixel[x + y*xRes] = p;
}

void ColorImage::outputPPM(char *filename)
{
    FILE *outFile = fopen(filename, "wb");

    assert(outFile); // die if file can't be opened

    fprintf(outFile, "P6 %d %d 255\n", xRes, yRes);
    fwrite(pPixel, 1, 3*xRes*yRes, outFile );

    fclose(outFile);
}






void PrintInfo( Info detail)
{
    printf("eye ");
    for(int i=0; i<3; ++i)
        cout<<detail.eye[i]<<" ";
    cout<<endl;
    cout<<"Direction ";
    for(int i=0; i<3; ++i)
        cout<<detail.direction[i]<<" ";
    cout<<endl;
    cout<<"EOF "<<detail.FOV<<endl;
    cout<<"W="<<detail.w<<" H="<<detail.h<<endl;
    cout<<"Sphere"<<endl;
    for(unsigned int i=0; i<detail.sph.size(); ++i)
    {
        printf("i=%d ",i+1);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.sph[i].origin[j]);
        printf(" %f",detail.sph[i].r);
        cout<<endl;
    }
    cout<<"Triangle"<<endl;
    for(unsigned int i=0; i<detail.tri.size(); ++i)
    {
        printf("i=%d ",i+1);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.tri[i].origin[j]);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.tri[i].v1[j]);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.tri[i].v2[j]);
        cout<<endl;
    }
}
void PrintVec3(vec3 v)
{
    printf("%f %f %f\n",v[0],v[1],v[2]);
}
