#pragma once

#include <d3d11.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "VertexType.h"

using namespace std;

class Util
{
public:
    Util();
    ~Util();

    bool LoadBMP(const WCHAR *filename, unsigned char *&pixelData, int *height, int *width);

    bool LoadModel(WCHAR *filename, vector<VertexType> &model, int &indexCount, int &vertexCount);
};

