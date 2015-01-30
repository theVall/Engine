#pragma once

/**
* AntTweakBar is used as GUI system:
* <link> http://anttweakbar.sourceforge.net/
*
* The AntTweakBar library is free to use and redistribute.
* It is released under the zlib/libpng license:
*
* Copyright © 2005-2013 Philippe Decaudin
*
* This software is provided ‘as-is’, without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from the use
* of this software.
*
* Permission is granted to anyone to use this software for any purpose, including
* commercial applications, and to alter it and redistribute it freely, subject to the
* following restrictions:
*
* 1.) The origin of this software must not be misrepresented; you must not claim that
* you wrote the original software. If you use this software in a product, an
* acknowledgment in the product documentation would be appreciated but is not required.
*
* 2.) Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3.) This notice may not be removed or altered from any source distribution.
*/

#include <d3d11.h>
#include <AntTweakBar.h>
#include <String.h>

class GUI
{
public:
    GUI();
    GUI(const GUI &);
    ~GUI();

    bool Initialize(ID3D11Device *pDevice,
                    const char *name,
                    int windowWidth,
                    int windowHeight);
    void Shutdown();
    bool ClearAll();

    bool AddFloatVar(const char *name, float &var, const char *params);
    bool AddIntVar(const char *name, int &var, const char *params);
    bool AddBoolVar(const char *name, bool &var, const char *params);
    bool AddVec3fVar(const char *name, float &var, const char *params);
    bool AddSeperator(const char *name, const char *params);
    bool AddLabel(const char *label);

    void RenderGUI();

private:
    TwBar *m_pTweakBar;
};

