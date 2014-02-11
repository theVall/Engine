#pragma once

#include "D3D.h"

#include "Camera.h"
#include "Model.h"
#include "ColorShader.h"
#include "TextureShader.h"
#include "LightShader.h"
#include "Light.h"

class Graphics
{
public:

    //  Constructor
    Graphics(void);

    //  Copy constructor
    Graphics(const Graphics&);

    //  Destructor
    ~Graphics(void);

    //  Initialization
    bool Initialize(int, int, HWND);

    //  Safely destroy graphics class.
    void Shutdown();

    //  
    bool ProcessFrame();

    //  Getter methods
    bool IsFullScreen();
    bool IsVsync();
    float GetScreenDepth();
    float GetScreenNear();

private:

    //  
    bool Render(float);

//  Member variables
private:

    bool m_fullScreen;
    bool m_vSync;
    float m_screenDepth;
    float m_screenNear;

    D3D *m_D3D;
    Camera* m_Camera;
    Model* m_Model;
    TextureShader* m_TextureShader;
    LightShader* m_LightShader;
    Light* m_Light;
};

