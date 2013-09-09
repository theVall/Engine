#include "System.h"


System::System(void)
{
    m_input = 0;
    m_graphics = 0;
}


System::System(const System &)
{
}


System::~System(void)
{
}


bool System::Initialize()
{
    int screenHeight  = 0;
    int screenWidth   = 0;

    m_graphics = new Graphics;
    if (!m_graphics)
    {
        return false;
    }

    //  Initialize windows API.
    InitializeWindows(screenWidth, screenHeight);
    
    if (!m_graphics->Initialize(screenWidth, screenHeight, m_hwnd))
    {
        return false;
    }

    m_input = new Input;
    if (!m_input)
    {
        return false;
    }
    m_input->Initialize();


    return true;
}


void System::Run()
{
    MSG msg;
    bool isRunning = true;
    
    ZeroMemory(&msg, sizeof(msg));

    while (isRunning)
    {
        //  Handle windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //  Check for exit signals.
        if (msg.message == WM_QUIT)
        {
            isRunning = false;
        }
        else
        {
            if (!ProcessFrame())
            {
                isRunning = false;
            }
        }
    }

    return;
}


void System::Shutdown()
{
    ShutdownWindows();

    if (m_graphics)
    {
        m_graphics->Shutdown();
        delete m_graphics;
        m_graphics = 0;
    }

    if (m_input)
    {
        delete m_input;
        m_input = 0;
    }

    return;
}


LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch(umsg)
    {
        //  Check if a key has been pressed on the keyboard.
        case WM_KEYDOWN:
        {
            m_input->ProcessKeyDown((unsigned int)wparam);
            return 0;
        }

        //  Check if a key has been released on the keyboard.
        case WM_KEYUP:
        {
            m_input->ProcessKeyUp((unsigned int)wparam);
            return 0;
        }

        // Send other messages to the default message handler.
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}


bool System::ProcessFrame()
{

    // Check if user pressed ESC.
    if (m_input->IsKeyDown(VK_ESCAPE))
    {
        return false;
    }

    return m_graphics->ProcessFrame();
}


void System::InitializeWindows(int& screenWidth, int& screenHeight)
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX = 0;
    int posY = 0;

    // External pointer to this object.
    ApplicationHandle = this;

    // Instance of this application.
    m_hinstance = GetModuleHandle(NULL);

    // Application name.
    m_applicationName = L"Engine";

    // Setup the windows class with default settings.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize = sizeof(WNDCLASSEX);

    RegisterClassEx(&wc);

    // Determine the resolution of the clients desktop screen.
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    //  Setup the screen settings depending on whether it is running in full screen or window mode.
    if(m_graphics->IsFullScreen())
    {
        //  Set the screen to maximum size of the users desktop and 32bit color depth.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        //  Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
    }
    else
    {
        screenWidth  = 800;
        screenHeight = 600;

        //  Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }

    //  Create the window with the screen settings and get the handle to it.
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                m_applicationName, 
                m_applicationName, 
                WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
                posX,
                posY,
                screenWidth,
                screenHeight,
                NULL,
                NULL,
                m_hinstance,
                NULL);

    //  Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    ShowCursor(false);

    return;
}


void System::ShutdownWindows()
{
    ShowCursor(true);

    //  Fix the display settings if leaving full screen mode.
    if(m_graphics->IsFullScreen())
    {
        ChangeDisplaySettings(NULL, 0);
    }

    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    //  Remove the application instance.
    UnregisterClass(m_applicationName, m_hinstance);
    m_hinstance = NULL;

    //  Release the pointer to this class.
    ApplicationHandle = NULL;

    return;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch(umessage)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }

        //  Pass all other messages to the message handler in the system class.
        default:
        {
            return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
        }
    }
}