#pragma once

#define WIN32_LEAN_AND_MEAN

//  own includes
#include "Input.h"
#include "Graphics.h"

/*-----------------------------------
Class for handling the system.
-----------------------------------*/
class System
{
public:

    //  Constructor: initialization of members.
    System(void);

    //  Copy constructor: Nothing done here.
    System(const System&);

    //  Destructor: Nothing done here.
    ~System(void);

    //  
    bool Initialize(void);
    
    //  Calls application processing in a loop.
    void Run(void);

    //  Replacement for destructor for safe cleanup.
    void Shutdown(void);

    //  Handles incomming windows system messages.
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);


private:

    //  All application processing is done here.
    bool ProcessFrame();

    //  Build the Window for rendering to.
    void InitializeWindows(int&, int&);

    //  Safely destroy windows.
    void ShutdownWindows();

//  Member variables
private:
    LPCWSTR m_applicationName;
    HINSTANCE m_hinstance;
    HWND m_hwnd;

    Input *m_input;
    Graphics *m_graphics;
};

//  Windows message handling
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static System* ApplicationHandle = 0;