#pragma once

#include <math.h>

class Position
{
public:
    Position();
    Position(const Position &);
    ~Position();

    void SetPosition(float, float, float);
    void SetRotation(float, float, float);

    void GetPosition(float&, float&, float&);
    void GetRotation(float&, float&, float&);

    void SetFrameTime(float);
    // Movement
    void MoveForward(bool, float sensitivity);
    void MoveBackward(bool, float sensitivity);
    void MoveLeft(bool, float sensitivity);
    void MoveRight(bool, float sensitivity);
    void MoveUpward(bool, float sensitivity);
    void MoveDownward(bool, float sensitivity);

    // Pitch and yaw
    void TurnOnMouseMovement(int mouseX, int mouseY, float sensitivity);

private:

    float m_positionX;
    float m_positionY;
    float m_positionZ;

    float m_rotationX;
    float m_rotationY;
    float m_rotationZ;

    float m_frameTime;

    // Movement
    float m_forwardSpeed;
    float m_backwardSpeed;
    float m_moveLeftSpeed;
    float m_moveRightSpeed;
    float m_upwardSpeed;
    float m_downwardSpeed;

    float m_pitchSpeed;
    float m_yawSpeed;
};

