#include "ImageUtil.h"


ImageUtil::ImageUtil()
{
}


ImageUtil::~ImageUtil()
{
}

bool ImageUtil::LoadBMP(const WCHAR *filename, unsigned char *&pixelData, int *height, int *width)
{
    int* datBuff[2] = { nullptr, nullptr };

    BITMAPFILEHEADER* bmpHeader = nullptr;
    BITMAPINFOHEADER* bmpInfo = nullptr;

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cout << "Failure to open bitmap file.\n";
        return false;
    }

    datBuff[0] = new int[sizeof(BITMAPFILEHEADER)];
    datBuff[1] = new int[sizeof(BITMAPINFOHEADER)];

    file.read((char*)datBuff[0], sizeof(BITMAPFILEHEADER));
    file.read((char*)datBuff[1], sizeof(BITMAPINFOHEADER));

    bmpHeader = (BITMAPFILEHEADER*)datBuff[0];
    bmpInfo = (BITMAPINFOHEADER*)datBuff[1];

    // Check if the file is an actual BMP file
    if (bmpHeader->bfType != 0x4D42)
    {
        std::cout << "File \"" << filename << "\" is not a bitmap file\n";
        return false;
    }

    pixelData = new unsigned char[bmpInfo->biSizeImage];

    // Go to where image data starts, then read in image data
    file.seekg(bmpHeader->bfOffBits);
    file.read((char*)pixelData, bmpInfo->biSizeImage);

    unsigned char tmpRGB = 0;
    // convert BGR pixel data to RGB
    for (unsigned long i = 0; i < bmpInfo->biSizeImage; i += 3)
    {
        tmpRGB = pixelData[i];
        pixelData[i] = pixelData[i + 2];
        pixelData[i + 2] = tmpRGB;
    }

    *height = (bmpInfo->biHeight);
    *width = (bmpInfo->biWidth);

    return true;
}