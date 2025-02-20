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
    TwDefine(" Settings position='10 10' ");
    TwDefine(" Settings iconified=false ");
    TwDefine(" Settings fontsize=2 ");
    TwDefine(" Settings size='220 380' ");

    TwDefine(" TW_HELP visible=false ");
    TwDefine(" GLOBAL iconpos=topleft ");

    return true;
}


void GUI::Shutdown()
{
    TwTerminate();
}


bool GUI::ClearAll()
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwRemoveAllVars(m_pTweakBar);

    return true;
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


bool GUI::AddIntVar(const char *name, int &var, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarRW(m_pTweakBar, name, TW_TYPE_INT32, &var, params);

    return true;
}


bool GUI::AddBoolVar(const char *name, bool &var, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarRW(m_pTweakBar, name, TW_TYPE_BOOL8, &var, params);

    return true;
}


bool GUI::AddVec3fVar(const char *name, float &var, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarRW(m_pTweakBar, name, TW_TYPE_DIR3F, &var, params);

    return true;
}


bool GUI::AddVarCB(const char *name, GuiParam<float> *pParam, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarCB(m_pTweakBar,
               name,
               TW_TYPE_FLOAT,
               GuiParam<float>::SetCB,
               GuiParam<float>::GetCB,
               pParam,
               params);


    return true;
}

bool GUI::AddVarCB(const char *name, GuiParam<bool> *pParam, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarCB(m_pTweakBar,
               name,
               TW_TYPE_BOOL8,
               GuiParam<float>::SetCB,
               GuiParam<float>::GetCB,
               pParam,
               params);


    return true;
}

bool GUI::AddVarCB(const char *name, GuiParam<int> *pParam, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddVarCB(m_pTweakBar,
               name,
               TW_TYPE_INT32,
               GuiParam<float>::SetCB,
               GuiParam<float>::GetCB,
               pParam,
               params);


    return true;
}


bool GUI::AddSeperator(const char *name, const char *params)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddSeparator(m_pTweakBar, name, params);

    return true;
}


bool GUI::AddLabel(const char *label)
{
    if (!m_pTweakBar)
    {
        return false;
    }

    TwAddButton(m_pTweakBar, NULL, NULL, NULL, label);

    return true;
}


void GUI::RenderGUI()
{
    TwDraw();
}


TwBar *GUI::GetCurrentBar()
{
    return m_pTweakBar;
}
