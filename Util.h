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

    // Load pixel data from a BMP image.
    bool LoadBMP(const WCHAR *filename, unsigned char *&pixelData, int *height, int *width);

    // Simple model loader from ASCII file.
    bool LoadModel(WCHAR *filename, vector<VertexType> &model, int &indexCount, int &vertexCount);
};

