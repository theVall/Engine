#include "Util.h"


Util::Util()
{
}


Util::~Util()
{
}


bool Util::LoadBMP(const WCHAR *filename, unsigned char *&pixelData, int *height, int *width)
{
    int* datBuff[2] = { nullptr, nullptr };

    BITMAPFILEHEADER* bmpHeader = nullptr;
    BITMAPINFOHEADER* bmpInfo = nullptr;

    std::ifstream file(filename, std::ios::binary);
    if (file.fail())
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

    unsigned char tmpB;
    // convert BGR pixel data to RGB
    for (unsigned long i = 0; i < bmpInfo->biSizeImage; i += 3)
    {
        tmpB = pixelData[i];
        pixelData[i] = pixelData[i + 2];
        pixelData[i + 2] = tmpB;
    }

    *height = (bmpInfo->biHeight);
    *width = (bmpInfo->biWidth);

    return true;
}


bool Util::LoadModel(WCHAR *filename,
                     vector<VertexType> &model,
                     int &indexCount,
                     int &vertexCount)
{
    ifstream fin;
    char input;

    fin.open(filename);
    if (fin.fail())
    {
        return false;
    }

    // Read up to the value of vertex count.
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }

    fin >> vertexCount;
    indexCount = vertexCount;

    model.resize(vertexCount);

    // Read up to the beginning of the data.
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }
    fin.get(input);
    fin.get(input);

    float tmpX;
    float tmpY;
    float tmpZ;

    // Read in the vertex data.
    for (int i = 0; i < vertexCount; ++i)
    {
        // position
        fin >> tmpX >> tmpY >> tmpZ;
        model[i].position.Set(tmpX, tmpY, tmpZ);
        // texture coordinates
        fin >> tmpX >> tmpY;
        model[i].texture.Set(tmpX, tmpY, 0.0);
        // normal
        fin >> tmpX >> tmpY >> tmpZ;
        model[i].normal.Set(tmpX, tmpY, tmpZ);
    }

    fin.close();

    return true;
}