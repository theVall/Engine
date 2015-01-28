#include "System.h"


System::System(void)
{
    m_Application = 0;
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
    bool result = false;

    // Create the application wrapper object.
    // Must be called __before__ windows API initialization.
    m_Application = new Application;
    if (!m_Application)
    {
        return false;
    }

    //  Initialize windows API.
    InitializeWindows(screenWidth, screenHeight);

    // Initialize the application wrapper object.
    // Must be called __after__ windows API initialization.
    result = m_Application->Initialize(m_hwnd, screenWidth, screenHeight);
    if (!result)
    {
        return false;
    }

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

    if (m_Application)
    {
        m_Application->Shutdown();
        delete m_Application;
        m_Application = 0;
    }

    return;
}


LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    // first handle AntTweakBar events
    if (TwEventWin(hwnd, umsg, wParam, lParam))
    {
        return 0;
    }

    switch (umsg)
    {
    case WM_LBUTTONDOWN:
    {
        m_Application->SetLeftMouseDown(true);
        while (ShowCursor(false) >= 0);
        break;
    }
    case WM_LBUTTONUP:
    {
        m_Application->SetLeftMouseDown(false);
        ShowCursor(true);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        m_Application->SetRightMouseDown(true);
        break;
    }
    case WM_RBUTTONUP:
    {
        m_Application->SetRightMouseDown(false);
        break;
    }
    }

    return DefWindowProc(hwnd, umsg, wParam, lParam);
}


bool System::ProcessFrame()
{
    bool result;

    // Do the frame processing for the application object.
    result = m_Application->ProcessFrame();
    if (!result)
    {
        return false;
    }

    return true;
}


void System::InitializeWindows(int &screenWidth, int &screenHeight)
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
    m_applicationName = L"VallEngine";

    // Icon
    wc.hIcon = (HICON)LoadImage(NULL,
                                L"vallEngine.ico",
                                IMAGE_ICON,
                                0,
                                0,
                                LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
    //wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);

    // Setup the windows class with default settings.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIconSm = NULL;
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
    if(m_Application->IsFullScreen())
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
        screenWidth  = 1280;
        screenHeight = 720;

        //  Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }

    int borderCaptionWidth = 2 * GetSystemMetrics(SM_CXSIZEFRAME);
    borderCaptionWidth += 2 * GetSystemMetrics(SM_CXPADDEDBORDER);

    int borderCaptionHeight = 4 * GetSystemMetrics(SM_CYSIZEFRAME);
    borderCaptionHeight += GetSystemMetrics(SM_CYCAPTION);

    //  Create the window with the screen settings and get the handle to it.
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                            m_applicationName,
                            m_applicationName,
                            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP |
                            WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                            posX,
                            posY,
                            screenWidth + borderCaptionWidth,
                            screenHeight + borderCaptionHeight,
                            NULL,
                            NULL,
                            m_hinstance,
                            NULL);

    //  Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    ShowCursor(true);

    return;
}


void System::ShutdownWindows()
{
    ShowCursor(true);

    //  Fix the display settings if leaving full screen mode.
    if(m_Application->IsFullScreen())
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