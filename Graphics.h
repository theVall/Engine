#pragma once

#include "D3D.h"

#include "Camera.h"
#include "Model.h"
#include "ColorShader.h"
#include "TextureShader.h"
#include "LightShader.h"
#include "Light.h"
#include "Element2d.h"

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

    // Process function called every frame.
    // <param> mouseX x-position of mouse 
    // <param> mouseY y-position of mouse 
    bool ProcessFrame();

    bool Render(float);

    //  Getter methods
    bool IsFullScreen();
    bool IsVsync();
    float GetScreenDepth();
    float GetScreenNear();


private:



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
    Element2d* m_Element2d;
};

