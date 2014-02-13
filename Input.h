#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Input
{
public:

    //  Constructor
    Input(void);
    //  Copy Constructor
    Input(const Input &);
    //  Destructor
    ~Input(void);

    bool Initialize(HINSTANCE, HWND, int, int);
    void Shutdown();
    // Read the current state of the devices into state buffers and processes the changes.
    bool Frame();

    // Keyboard key checks
    bool IsEscapePressed();
    // Movement
    bool IsLeftPressed();
    bool IsRightPressed();
    bool IsUpPressed();
    bool IsDownPressed();
    bool IsWPressed();
    bool IsAPressed();
    bool IsSPressed();
    bool IsDPressed();
    bool IsQPressed();
    bool IsEPressed();
    bool IsSpacePressed();
    bool IsCPressed();
    bool IsZPressed();
    bool IsPgUpPressed();
    bool IsPgDownPressed();
    
    void GetMouseLocation(int&, int&);

private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput();

private:
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;

    unsigned char m_keyboardState[256];
    DIMOUSESTATE m_mouseState;

    int m_screenWidth;
    int m_screenHeight;
    int m_mouseX;
    int m_mouseY;
};

