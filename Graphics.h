#pragma once

#include "D3D.h"

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
    bool Render();

//  Member variables
private:

    bool m_fullScreen;
    bool m_vSync;
    float m_screenDepth;
    float m_screenNear;

    D3D *m_D3D;
};

