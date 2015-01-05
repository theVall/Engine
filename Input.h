#pragma once

#include <Windows.h>

class Input
{
public:

    //  Constructor
    Input(void);
    //  Copy Constructor
    Input(const Input &);
    //  Destructor
    ~Input(void);

    bool Initialize(HWND, int, int);
    void Shutdown();

    // Get addresses to absolute mouse positions.
    void GetMouseLocation(int &mouseX, int &mouseY);
    // Get addresses to mouse movement delta, relative to last frame.
    void GetMouseLocationChange(int &dX, int &dY);
    // Get the mouse coordinates as POINT struct.
    POINT GetMousePoint();

private:

    bool ReadKeyboard();
    bool ReadMouse();

private:

    HWND m_hwnd;

    POINT m_mousePoint;

    int m_screenWidth;
    int m_screenHeight;
};

