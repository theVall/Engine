#pragma once

#include <math.h>

#include "Vec3f.h"

using namespace math;

class Position
{
public:
    Position();
    Position(const Position &);
    ~Position();

    void SetPosition(Vec3f pos);
    void SetRotation(Vec3f rot);

    void GetPosition(Vec3f &pos);
    void GetRotation(Vec3f &rot);

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

    Vec3f m_position;
    Vec3f m_rotation;

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

