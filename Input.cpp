#include "Input.h"


Input::Input(void)
{
}


Input::Input(const Input &)
{
}


Input::~Input(void)
{
}


bool Input::Initialize(HWND hwnd, int screenWidth, int screenHeight)
{
    m_hwnd = hwnd;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_mousePoint.x = screenWidth / 2;
    m_mousePoint.y = screenHeight / 2;

    return true;
}


void Input::Shutdown()
{
}


bool Input::ReadKeyboard()
{
    return true;
}


bool Input::ReadMouse()
{
    BOOL result;
    result = GetCursorPos(&m_mousePoint);
    if (!result)
    {
        return false;
    }
    result = ScreenToClient(m_hwnd, &m_mousePoint);
    if (!result)
    {
        return false;
    }

    // Clip cursor to DX window
    //RECT rc;
    //GetWindowRect(m_hwnd, &rc);

    //RECT *pRc = &rc;
    //ClipCursor(pRc);

    return true;
}


void Input::GetMouseLocation(int &mouseX, int &mouseY)
{
    mouseX = m_mousePoint.x;
    mouseY = m_mousePoint.y;
    return;
}


void Input::GetMouseLocationChange(int &dX, int &dY)
{
    POINT lastMousePoint = m_mousePoint;

    ReadMouse();

    dX = lastMousePoint.x - m_mousePoint.x;
    dY = lastMousePoint.y - m_mousePoint.y;

    return;
}


POINT Input::GetMousePoint()
{
    ReadMouse();
    return m_mousePoint;
}