#pragma once

#include <d3d11.h>

#include "FW1FontWrapper.h"
#pragma comment (lib, "FW1FontWrapper.lib")

/**
 * FW1FontWrapper is used for font rendering:
 * <link> http://www.rufelt.com/fw1fontwrapper/
 *
 * Copyright (c) 2011 Erik Rufelt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 */

class Font
{
public:
    Font();
    Font(const Font &);
    ~Font();

    bool Initialize(LPCWSTR fontType, ID3D11Device *device);
    void Shutdown();

    // Change the font type.
    bool changeFontType(LPCWSTR fontType, ID3D11Device *device);

    // Draws text as overlay on the scene.
    void drawText(ID3D11DeviceContext *context,
                  WCHAR *text,
                  float fontSize,
                  float xPos,
                  float yPos,
                  int color,
                  int flags);

private:
    IFW1FontWrapper* m_FontWrapper;
    IFW1Factory* m_FW1Factory;
};

