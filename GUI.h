#pragma once

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

    bool AddFloatVar(const char *name, float &var, const char *params);
    bool AddBoolVar(const char *name, bool &var);
    bool AddSeperator(const char *name, const char *params);

    void RenderGUI();

private:
    TwBar *m_pTweakBar;
};

