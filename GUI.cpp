#include "GUI.h"


GUI::GUI()
{
    m_pTweakBar = nullptr;
}


GUI::GUI(const GUI &)
{
}


GUI::~GUI()
{
}


bool GUI::Initialize(ID3D11Device *pDevice,
                     const char *name,
                     int windowWidth,
                     int windowHeight)
{
    TwInit(TW_DIRECT3D11, pDevice);
    TwWindowSize(windowWidth, windowHeight);

    m_pTweakBar = TwNewBar(name);

    //TwDefine(" Settings color='255 255 0' ");
    TwDefine(" Settings position='10 40' ");
    TwDefine(" Settings iconified=true ");
    TwDefine(" TW_HELP visible=false ");
    TwDefine(" GLOBAL iconpos=topleft ");

    return true;
}


void GUI::Shutdown()
{
    TwTerminate();
}


bool GUI::AddFloatVar(const char *name, float &var, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarRW(m_pTweakBar, name, TW_TYPE_FLOAT, &var, params);

    return true;
}


bool GUI::AddBoolVar(const char *name, bool &var)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarRW(m_pTweakBar, name, TW_TYPE_BOOL8, &var, "");

    return true;
}


void GUI::RenderGUI()
{
    TwDraw();
}


