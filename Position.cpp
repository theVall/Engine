#include "Position.h"


Position::Position()
{
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;

    m_rotationX = 0.0f;
    m_rotationY = 0.0f;
    m_rotationZ = 0.0f;

    m_frameTime = 0.0f;

    m_forwardSpeed = 0.0f;
    m_backwardSpeed = 0.0f;
    m_upwardSpeed = 0.0f;
    m_downwardSpeed = 0.0f;

    m_pitchSpeed = 0.0f;
    m_yawSpeed = 0.0f;
}


Position::Position(const Position &)
{
}


Position::~Position()
{
}


void Position::SetPosition(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    return;
}


void Position::SetRotation(float x, float y, float z)
{
    m_rotationX = x;
    m_rotationY = y;
    m_rotationZ = z;
    return;
}


void Position::GetPosition(float& x, float& y, float& z)
{
    x = m_positionX;
    y = m_positionY;
    z = m_positionZ;
    return;
}


void Position::GetRotation(float& x, float& y, float& z)
{
    x = m_rotationX;
    y = m_rotationY;
    z = m_rotationZ;
    return;
}


void Position::SetFrameTime(float time)
{
    m_frameTime = time;
    return;
}


void Position::MoveForward(bool keydown, float sensitivity)
{
    float radians;

    // Update the forward speed movement based on the frame time
    // and whether the user is holding the key down or not.
    if (keydown)
    {
        m_forwardSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_forwardSpeed = 0.0f;
    }

    // Convert degrees to radians.
    radians = m_rotationY * 0.0174532925f;

    // Update the position.
    m_positionX += sinf(radians) * m_forwardSpeed;
    m_positionZ += cosf(radians) * m_forwardSpeed;

    return;
}


void Position::MoveBackward(bool keydown, float sensitivity)
{
    float radians;

    // Update the backward speed movement based on the frame time and whether the user is holding the key down or not.
    if (keydown)
    {
        m_backwardSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_backwardSpeed = 0.0f;
    }

    // Convert degrees to radians.
    radians = m_rotationY * 0.0174532925f;

    // Update the position.
    m_positionX -= sinf(radians) * m_backwardSpeed;
    m_positionZ -= cosf(radians) * m_backwardSpeed;

    return;
}


void Position::MoveLeft(bool keydown, float sensitivity)
{
    float radians;

    // Update the backward speed movement based on the frame time and whether the user is holding the key down or not.
    if (keydown)
    {
        m_moveLeftSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_moveLeftSpeed = 0.0f;
    }

    // Convert degrees to radians.
    radians = (m_rotationY - 90.0f) * 0.0174532925f;

    // Update the position.
    m_positionX += sinf(radians) * m_moveLeftSpeed;
    m_positionZ += cosf(radians) * m_moveLeftSpeed;

    return;
}


void Position::MoveRight(bool keydown, float sensitivity)
{
    float radians;

    // Update the backward speed movement based on the frame time and whether the user is holding the key down or not.
    if (keydown)
    {
        m_moveRightSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_moveRightSpeed = 0.0f;
    }

    // Convert degrees to radians.
    radians = (m_rotationY - 90.0f) * 0.0174532925f;

    // Update the position.
    m_positionX -= sinf(radians) * m_moveRightSpeed;
    m_positionZ -= cosf(radians) * m_moveRightSpeed;

    return;
}


void Position::MoveUpward(bool keydown, float sensitivity)
{
    // Update the upward speed movement based on the frame time
    // and whether the user is holding the key down or not.
    if (keydown)
    {
        m_upwardSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_upwardSpeed = 0.0f;
    }

    // Update the height position.
    m_positionY += m_upwardSpeed;

    return;
}


void Position::MoveDownward(bool keydown, float sensitivity)
{
    // Update the downward speed movement based on the frame time 
    // and whether the user is holding the key down or not.
    if (keydown)
    {
        m_downwardSpeed = m_frameTime * sensitivity;
    }
    else
    {
        m_downwardSpeed = 0.0f;
    }

    // Update the height position.
    m_positionY -= m_downwardSpeed;

    return;
}


void Position::TurnOnMouseMovement(int mouseX, int mouseY, float sensitivity)
{
    m_pitchSpeed -= sensitivity * mouseX;
    m_rotationY = m_pitchSpeed;

    m_yawSpeed -= sensitivity * mouseY;
    m_rotationX = m_yawSpeed;

    return;
}
