#pragma once

#include <d3d11.h>
#include <iostream>
#include <fstream>

class ImageUtil
{
public:
    ImageUtil();
    ~ImageUtil();

    bool LoadBMP(const WCHAR *filename, unsigned char *&pixelData, int *height, int *width);
};

