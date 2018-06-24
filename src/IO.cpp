#include"IO.h"
#include<stdio.h>
#include<iostream>
#include<sstream>
#include<fstream>
#include<string.h>
#include<string>
#include <stdlib.h>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;
void ReadFile(Info &detail,const char fileName[])
{
    /*FILE *fp =fopen(fileName,"r");
    if(fp==NULL)
    {
        cout<<"Cannot find this file."<<endl;
        return ;
    }*/
    fstream file;      //宣告fstream物件
    file.open(fileName, ios::in);
    if(file.is_open()==false)
    {
        cout<<"No such input file\n";
        return;
    }

    vector<vec3> vertex,vn,vt;
    vec3 zero;
    vertex.push_back(zero),vt.push_back(zero);
    string statement,fragment;
    Material *mptr = nullptr;
    while(getline(file,statement))//v 1.0 1.0 2.0
    {
        //cout<<statement<<endl;
        istringstream buffer(statement);
        getline(buffer,fragment,' ');
        if(strcmp(fragment.c_str(),"v")==0)
        {
            vec3 temp;
            int index=0;
            getline(buffer,fragment,' ');//take off abundant ' '
            while(getline(buffer,fragment,' '))
                temp[index++]=strtof(fragment.c_str(),0);
            vertex.push_back(temp);
        }
        else if(strcmp(fragment.c_str(),"vt")==0)
        {
            vec3 temp;
            int index=0;
            while(getline(buffer,fragment,' '))
                temp[index++]=strtof(fragment.c_str(),0);
            vt.push_back(temp);
        }
        else if(strcmp(fragment.c_str(),"usemtl")==0)//ex: usemtl sp_svod_kapitel
        {
            bool found = false;
            getline(buffer,fragment,' ');
            for(int i=0; i<detail.mtl.size(); ++i)
                if(detail.mtl[i].title==fragment)
                {
                    found =true;
                    mptr = &detail.mtl[i];
                    //printf("title=%s mmptr=%s\n",fragment.c_str(),
                    //        mptr->title.c_str());
                    break;
                }
            if(found==false)
                printf("Not found\n");
        }
        else if(strcmp(fragment.c_str(),"f")==0)// ex: f 1/1/1 2/2/2 3/3/3
        {
            vector<Index> index;
            while(getline(buffer,fragment,' '))// fragment=1/1/1
            {
                Index temp;
                istringstream v_vt(fragment);
                string v,vt;
                getline(v_vt,v,'/');
                getline(v_vt,vt,'/');
                temp.v = stoi(v);
                temp.vt = stoi(vt);
                index.push_back(temp);
            }
            MakeTriangle(detail,index,vertex,vt,mptr);
        }
        else if(strcmp(fragment.c_str(),"mtllib")==0)
        {
            getline(buffer,fragment,' ');
            ReadMTL(detail,fragment);
        }
        else if(strcmp(fragment.c_str(),"E")==0)
        {
            vec3 temp;
            int index=0;
            getline(buffer,fragment,' ');//take off abundant ' '
            while(getline(buffer,fragment,' '))
                temp[index++]=strtof(fragment.c_str(),0);
            detail.eye = temp;
        }
        else if(strcmp(fragment.c_str(),"V")==0)
        {
            vec3 temp;
            int index=0;
            while(getline(buffer,fragment,' '))
                temp[index++]=strtof(fragment.c_str(),0);
            detail.direction = temp;
        }
        else if(strcmp(fragment.c_str(),"F")==0)
        {
            float temp;
            int index=0;
            getline(buffer,fragment,' ');
            temp=strtof(fragment.c_str(),0);
            detail.FOV = temp;
        }
        else if(strcmp(fragment.c_str(),"R")==0)
        {
            int w,h;
            getline(buffer,fragment,' ');
            w=atoi(fragment.c_str());
            getline(buffer,fragment,' ');
            h=atoi(fragment.c_str());
            detail.w = w;
            detail.h = h;
        }
        else if(strcmp(fragment.c_str(),"L")==0)
        {
            Light light;
            int index=0;
            while(getline(buffer,fragment,' '))
                light.position[index++]=strtof(fragment.c_str(),0);
            detail.lig.push_back(light) ;
        }
    }
    file.close();
}

void ReadMTL(Info &detail,const string &mtlFileName)
{
    string completeURL = "input/" +mtlFileName;
    FILE* fp = fopen(completeURL.c_str(),"r");
    if(fp==nullptr)
    {
        cout<<"No such material file\n";
        return;
    }
    char input[1000]="";
    Material temp;
    while(fscanf(fp,"%s",input)!=EOF)
    {
        if(strcmp(input,"newmtl")==0)
        {
            if(temp.title!="")//the last mtl
            {
                bool hasExist = false;
                for(int i=0; i<detail.imgIndex; ++i)
                    if(detail.img[i].title==temp.map_Ka)
                    {
                        hasExist = true;
                        temp.imgKa = &detail.img[i].img;
                        break;
                    }
                if(hasExist==false)
                {
                    MatImage newImage;
                    newImage.title = temp.map_Ka;
                    newImage.img = imread(temp.map_Ka,CV_LOAD_IMAGE_UNCHANGED);
                    //if(newImage.img.empty())printf("WOW\n\n");
                    detail.img[detail.imgIndex] = newImage;
                    temp.imgKa = &detail.img[detail.imgIndex++].img;
                    if(temp.imgKa->empty())
                        printf("impossible\n");
                }//materail img

                hasExist = false;
                for(int i=0; i<detail.imgIndex; ++i)
                    if(detail.img[i].title==temp.map_Kd)
                    {
                        hasExist = true;
                        temp.imgKd = &detail.img[i].img;
                        break;
                    }
                if(hasExist==false)
                {
                    MatImage newImage;
                    newImage.title = temp.map_Kd;
                    newImage.img = imread(temp.map_Kd,CV_LOAD_IMAGE_UNCHANGED);
                    detail.img[detail.imgIndex]=newImage;
                    temp.imgKd = &detail.img[detail.imgIndex++].img;
                    //cout<<temp.imgKd;
                }

                detail.mtl.push_back(temp);
            }

            fscanf(fp,"%s",input);//new mtl
            temp.title = input;
        }
        else if(strcmp(input,"Ns")==0)
        {
            fscanf(fp,"%s",input);
            temp.Ns = strtof(input,0);
        }
        else if(strcmp(input,"Ni")==0)
        {
            fscanf(fp,"%s",input);
            temp.Ni = strtof(input,0);
        }
        else if(strcmp(input,"d")==0)
        {
            fscanf(fp,"%s",input);
            temp.d = strtof(input,0);
        }
        else if(strcmp(input,"Tr")==0)
        {
            fscanf(fp,"%s",input);
            temp.Tr = strtof(input,0);
        }
        else if(strcmp(input,"Tf")==0)
        {
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&temp.Tf[i]);
        }
        else if(strcmp(input,"illum")==0)
        {
            fscanf(fp,"%s",input);
            temp.illum = strtof(input,0);
        }
        else if(strcmp(input,"Ka")==0)
        {
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&temp.Ka[i]);
        }
        else if(strcmp(input,"Kd")==0)
        {
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&temp.Kd[i]);
        }
        else if(strcmp(input,"Ks")==0)
        {
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&temp.Ks[i]);
        }
        else if(strcmp(input,"Ke")==0)
        {
            for(int i=0; i<3; ++i)
                fscanf(fp,"%f",&temp.Ke[i]);
        }
        else if(strcmp(input,"map_Ka")==0)
        {
            fscanf(fp,"%s",input);
            temp.map_Ka = "Input/" + string(input);
        }
        else if(strcmp(input,"map_Kd")==0)
        {
            fscanf(fp,"%s",input);
            temp.map_Kd = "Input/" + string(input);
        }
        else if(strcmp(input,"map_d")==0)
        {
            fscanf(fp,"%s",input);
            temp.map_d = "Input/" + string(input);
        }
        else if(strcmp(input,"map_bump")==0)
        {
            fscanf(fp,"%s",input);
            temp.map_bump = "Input/" + string(input);
        }
    }
    //LAST ONE
    bool hasExist = false;
    for(int i=0; i<detail.imgIndex; ++i)
        if(detail.img[i].title==temp.map_Ka)
        {
            hasExist = true;
            temp.imgKa = &detail.img[i].img;
            break;
        }
    if(hasExist==false)
    {
        MatImage newImage;
        newImage.title = temp.map_Ka;
        newImage.img = imread(temp.map_Ka,CV_LOAD_IMAGE_UNCHANGED);
        //if(newImage.img.empty())printf("WOW\n\n");
        detail.img[detail.imgIndex] = newImage;
        temp.imgKa = &detail.img[detail.imgIndex++].img;
    }//materail img

    hasExist = false;
    for(int i=0; i<detail.img.size(); ++i)
        if(detail.img[i].title==temp.map_Kd)
        {
            hasExist = true;
            temp.imgKd = &detail.img[i].img;
            break;
        }
    if(hasExist==false)
    {
        MatImage newImage;
        newImage.title = temp.map_Kd;
        newImage.img = imread(temp.map_Kd,CV_LOAD_IMAGE_UNCHANGED);
        detail.img[detail.imgIndex] = newImage;
        temp.imgKd = &detail.img[detail.imgIndex].img;
        //cout<<temp.imgKd;
    }

    detail.mtl.push_back(temp);

    fclose(fp);
    /*
    fstream file;      //宣告fstream物件
    string completeURL = "input/" +mtlFileName;
    file.open(completeURL, ios::in);
    if(file.is_open()==false)
    {
        cout<<"No such material file\n";
        return;
    }

    string statement,fragment;
    Material temp;
    while(getline(file,statement))//ex: newmtl leaf
    {
        cout<<statement<<endl;
        istringstream buffer(statement);
        getline(buffer,fragment,' ');cout<<"f"<<fragment<<"?"<<endl;
        if(strcmp(fragment.c_str(),"newmtl")==0)
        {printf("mtl size=%s\n",temp.map_Ka.c_str());
            if(temp.title!="")//the last mtl
                detail.mtl.push_back(temp);

            getline(buffer,fragment,' ');//new mtl
            temp.title = fragment;
        }
        else if(strcmp(fragment.c_str(),"Ns")==0)
        {
            getline(buffer,fragment,' ');
            temp.Ns = strtof(fragment.c_str(),0);
            cout<<temp.Ns<<endl;
        }
        else if(strcmp(fragment.c_str(),"Ni")==0)
        {
            getline(buffer,fragment,' ');
            temp.Ni = strtof(fragment.c_str(),0);
        }
        else if(strcmp(fragment.c_str(),"d")==0)
        {
            getline(buffer,fragment,' ');
            temp.d = strtof(fragment.c_str(),0);
        }
        else if(strcmp(fragment.c_str(),"Tr")==0)
        {
            getline(buffer,fragment,' ');
            temp.Tr = strtof(fragment.c_str(),0);
        }
        else if(strcmp(fragment.c_str(),"Tf")==0)
        {
            for(int i=0; i<3; ++i)
            {
                getline(buffer,fragment,' ');
                temp.Tf[i] = strtof(fragment.c_str(),0);
            }
        }
        else if(strcmp(fragment.c_str(),"illum")==0)
        {
            getline(buffer,fragment,' ');
            temp.illum = strtof(fragment.c_str(),0);
        }
        else if(strcmp(fragment.c_str(),"Ka")==0)
        {
            for(int i=0; i<3; ++i)
            {
                getline(buffer,fragment,' ');
                temp.Ka[i] = strtof(fragment.c_str(),0);
            }
        }
        else if(strcmp(fragment.c_str(),"Kd")==0)
        {
            for(int i=0; i<3; ++i)
            {
                getline(buffer,fragment,' ');
                temp.Kd[i] = strtof(fragment.c_str(),0);
            }
        }
        else if(strcmp(fragment.c_str(),"Ks")==0)
        {
            for(int i=0; i<3; ++i)
            {
                getline(buffer,fragment,' ');
                temp.Ks[i] = strtof(fragment.c_str(),0);
            }
        }
        else if(strcmp(fragment.c_str(),"Ke")==0)
        {
            for(int i=0; i<3; ++i)
            {
                getline(buffer,fragment,' ');
                temp.Ke[i] = strtof(fragment.c_str(),0);
            }
        }
        else if(strcmp(fragment.c_str(),"map_Ka")==0)
        {
            getline(buffer,fragment,' ');
            string fullPath = "input/" + fragment;
            cout<<"??"<<temp.map_Ka<<endl;
            temp.map_Ka = fullPath;
        }
        else if(strcmp(fragment.c_str(),"map_Kd")==0)
        {
            getline(buffer,fragment,' ');
            string fullPath = "input/" + fragment;
            temp.map_Kd = fullPath;
        }
        else if(strcmp(fragment.c_str(),"map_d")==0)
        {
            getline(buffer,fragment,' ');
            string fullPath = "input/" + fragment;
            temp.map_d = fullPath;
        }
        else if(strcmp(fragment.c_str(),"map_bump")==0)
        {
            getline(buffer,fragment,' ');
            string fullPath = "input/" + fragment;
            temp.map_bump = fullPath;
            cout<<temp.map_bump<<endl;
        }
    }
    file.close();
    */
}
void MakeTriangle(Info &detail,vector<Index>&index,vector<vec3>&vertex,vector<vec3>& vt,Material *mptr)
{
    while(index.size()>=3)
    {
        int i;
        vector<Index>temp;
        temp.push_back(index[0]);
        for(i=0; i+2<index.size(); i+=2)
        {
            Triangle tri;
            for(int j=i; j<=i+2; ++j)
            {
                tri.p[j] = vertex[index[j].v];
                tri.texture[j] = vt[index[j].vt];
            }
            tri.v[0] = tri.p[1] - tri.p[0];
            tri.v[1] = tri.p[2] - tri.p[0];
            tri.n  = tri.v[0]^tri.v[1];//cross
            tri.n.normalize();
            tri.mtr = mptr;
            cout<<tri.mtr->Ni<<endl;
            if(tri.mtr->imgKa->empty()||tri.mtr->imgKd->empty())
                printf("??????????\n\n\n");

            detail.tri.push_back(tri);
            //PrintTriangle(tri);
            temp.push_back(index[i+2]);
        }
        for(int left = i+1; left<index.size(); ++left)
            temp.push_back(index[left]);
        index.clear();
        for(int j=0; j<temp.size(); ++j)
            index.push_back(temp[j]);
    }

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
            printf("%f ",detail.tri[i].p[0][j]);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.tri[i].p[1][j]);
        for(int j=0; j<3; ++j)
            printf("%f ",detail.tri[i].p[2][j]);
        cout<<endl;
    }
}
void PrintVec3(vec3 v)
{
    printf("%f %f %f\n",v[0],v[1],v[2]);
}

void PrintTriangle(Triangle tri)
{
    printf("print Tri\n");
    PrintVec3(tri.p[0]);
    PrintVec3(tri.p[1]);
    PrintVec3(tri.p[2]);
}
