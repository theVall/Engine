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


void Input::Initialize()
{
    int i = 0;

    //  Initialize all key states to being released.
    for (i = 0; i < 256; i++)
    {
        m_keys[i] = false;
    }

    return;
}


void Input::ProcessKeyDown(unsigned int input)
{
    m_keys[input] = true;
    return;
}


void Input::ProcessKeyUp(unsigned int input)
{
    m_keys[input] = false;
    return;
}


bool Input::IsKeyDown(unsigned int key)
{
    return m_keys[key];
}