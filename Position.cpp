#include "Position.h"


Position::Position()
{
    m_position = Vec3f();
    m_rotation = Vec3f();

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


void Position::SetPosition(Vec3f pos)
{
    m_position = pos;
    return;
}


void Position::SetRotation(Vec3f rot)
{
    m_rotation = rot;
    return;
}


void Position::GetPosition(Vec3f &pos)
{
    pos = m_position;
    return;
}


void Position::GetRotation(Vec3f &rot)
{
    rot = m_rotation;
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
    radians = m_rotation.y * 0.0174532925f;

    // Update the position.
    m_position.x += sinf(radians) * m_forwardSpeed;
    m_position.z += cosf(radians) * m_forwardSpeed;

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
    radians = m_rotation.y * 0.0174532925f;

    // Update the position.
    m_position.x -= sinf(radians) * m_backwardSpeed;
    m_position.z -= cosf(radians) * m_backwardSpeed;

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
    radians = (m_rotation.y - 90.0f) * 0.0174532925f;

    // Update the position.
    m_position.x += sinf(radians) * m_moveLeftSpeed;
    m_position.z += cosf(radians) * m_moveLeftSpeed;

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
    radians = (m_rotation.y - 90.0f) * 0.0174532925f;

    // Update the position.
    m_position.x -= sinf(radians) * m_moveRightSpeed;
    m_position.z -= cosf(radians) * m_moveRightSpeed;

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
    m_position.y += m_upwardSpeed;

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
    m_position.y -= m_downwardSpeed;

    return;
}


void Position::TurnOnMouseMovement(int mouseX, int mouseY, float sensitivity)
{
    m_pitchSpeed -= sensitivity * mouseX;
    m_rotation.y = m_pitchSpeed;

    m_yawSpeed -= sensitivity * mouseY;
    m_rotation.x = m_yawSpeed;

    return;
}
