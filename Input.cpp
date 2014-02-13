#include "Input.h"


Input::Input(void)
{
    m_directInput = 0;
    m_keyboard = 0;
    m_mouse = 0;
}


Input::Input(const Input &)
{
}


Input::~Input(void)
{
}


bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result;

    // Store the screen size for positioning the mouse cursor.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize the location of the mouse on the screen.
    m_mouseX = 0;
    m_mouseY = 0;

    // Initialize the main direct input interface.
    result = DirectInput8Create(hinstance,
                                DIRECTINPUT_VERSION,
                                IID_IDirectInput8,
                                (void**)&m_directInput,
                                NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the keyboard.
    result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format (predefined keyboard).
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the keyboard to not share with other programs.
    result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Acquire the keyboard.
    result = m_keyboard->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the mouse.
    result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format for the mouse using the predefined mouse data format.
    result = m_mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the mouse to share with other programs.
    result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Acquire the mouse.
    result = m_mouse->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void Input::Shutdown()
{
    // Release the mouse.
    if (m_mouse)
    {
        m_mouse->Unacquire();
        m_mouse->Release();
        m_mouse = 0;
    }

    // Release the keyboard.
    if (m_keyboard)
    {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = 0;
    }

    // Release the main interface to direct input.
    if (m_directInput)
    {
        m_directInput->Release();
        m_directInput = 0;
    }

    return;
}


bool Input::Frame()
{
    bool result;

    result = ReadKeyboard();
    if (!result)
    {
        return false;
    }

    result = ReadMouse();
    if (!result)
    {
        return false;
    }

    ProcessInput();

    return true;
}


bool Input::ReadKeyboard()
{
    HRESULT result;

    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_keyboard->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}


bool Input::ReadMouse()
{
    HRESULT result;

    result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
    if (FAILED(result))
    {
        // If the mouse lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_mouse->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}


void Input::ProcessInput()
{
    // Update the location of the mouse cursor based on the change during the frame.
    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;

    // Ensure the mouse location doesn't exceed the screen width or height.
    if (m_mouseX < 0)  { m_mouseX = 0; }
    if (m_mouseY < 0)  { m_mouseY = 0; }

    if (m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
    if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }

    return;
}


// Keyboard -----------------
bool Input::IsEscapePressed()
{
    // Do a bitwise and on the keyboard state to check if the escape key is being pressed.
    if (m_keyboardState[DIK_ESCAPE] & 0x80)
    {
        return true;
    }

    return false;
}


bool Input::IsLeftPressed()
{
    if (m_keyboardState[DIK_LEFT])
    {
        return true;
    }

    return false;
}


bool Input::IsRightPressed()
{
    if (m_keyboardState[DIK_RIGHT])
    {
        return true;
    }

    return false;
}


bool Input::IsUpPressed()
{
    if (m_keyboardState[DIK_UP])
    {
        return true;
    }

    return false;
}


bool Input::IsDownPressed()
{
    if (m_keyboardState[DIK_DOWN])
    {
        return true;
    }

    return false;
}


bool Input::IsWPressed()
{
    if (m_keyboardState[DIK_W])
    {
        return true;
    }

    return false;
}


bool Input::IsAPressed()
{
    if (m_keyboardState[DIK_A])
    {
        return true;
    }

    return false;
}


bool Input::IsSPressed()
{
    if (m_keyboardState[DIK_S])
    {
        return true;
    }

    return false;
}


bool Input::IsDPressed()
{
    if (m_keyboardState[DIK_D])
    {
        return true;
    }

    return false;
}


bool Input::IsSpacePressed()
{
    if (m_keyboardState[DIK_SPACE])
    {
        return true;
    }

    return false;
}


bool Input::IsCPressed()
{
    if (m_keyboardState[DIK_C])
    {
        return true;
    }

    return false;
}


bool Input::IsQPressed()
{
    if (m_keyboardState[DIK_Q])
    {
        return true;
    }

    return false;
}


bool Input::IsEPressed()
{
    if (m_keyboardState[DIK_E])
    {
        return true;
    }

    return false;
}


bool Input::IsZPressed()
{
    if (m_keyboardState[DIK_Z])
    {
        return true;
    }

    return false;
}


bool Input::IsPgUpPressed()
{
    if (m_keyboardState[DIK_PGUP])
    {
        return true;
    }

    return false;
}


bool Input::IsPgDownPressed()
{
    if (m_keyboardState[DIK_PGDN])
    {
        return true;
    }

    return false;
}

// Mouse -----------------------------
void Input::GetMouseLocation(int& mouseX, int& mouseY)
{
    mouseX = m_mouseX;
    mouseY = m_mouseY;
    return;
}
