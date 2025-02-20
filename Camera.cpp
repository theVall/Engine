#include "Camera.h"


Camera::Camera()
{
    m_position = Vec3f();
    m_rotation = Vec3f();
}


Camera::Camera(const Camera &)
{
}


Camera::~Camera()
{
}


void Camera::SetPosition(Vec3f pos)
{
    m_position = pos;
    return;
}


void Camera::SetRotation(Vec3f rot)
{
    m_rotation = rot;
    return;
}

Vec3f Camera::GetPosition()
{
    return m_position;
}


Vec3f Camera::GetRotation()
{
    return m_rotation;
}


void Camera::GetViewMatrix(XMMATRIX &viewMatrix)
{
    viewMatrix = m_viewMatrix;
    return;
}


void Camera::Render()
{
    XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 lookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);

    XMVECTOR upVec = XMLoadFloat3(&up);
    XMVECTOR positionVec = m_position.GetAsXMVector();

    //  Setup where the camera is looking by default.
    XMVECTOR lookAtVec = XMLoadFloat3(&lookAt);

    //  Yaw, pitch and roll in radians.
    float pitch = m_rotation.x * 0.0174532925f;
    float yaw = m_rotation.y * 0.0174532925f;
    float roll = m_rotation.z * 0.0174532925f;

    // Create the rotation matrix from the yaw, pitch, and roll values.
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Transform the look and up vectors with rotation matrix so the view is correctly rotated at the origin.
    lookAtVec = XMVector3TransformCoord(lookAtVec, rotationMatrix);
    upVec     = XMVector3TransformCoord(    upVec, rotationMatrix);

    // Translate the rotated camera position to the location of the viewer.
    lookAtVec = positionVec + lookAtVec;

    // Create the view matrix from the three updated vectors.
    m_viewMatrix = XMMatrixLookAtLH(positionVec, lookAtVec, upVec);

    return;
}


void Camera::RenderOrbital(Vec3f targetPoint, float zoom)
{
    // fixed up vector - TODO: variable for true orbital cam
    Vec3f up = Vec3f(0.0f, 1.0f, 0.0f);

    // Yaw and pitch in radians.
    float pitch = m_rotation.x * 0.0174532925f;
    float yaw = m_rotation.y * 0.0174532925f;

    // TODO: avoid rotation over north and south pole
    //Vec3f eval = (targetPoint - m_position).Normalize();
    //if (eval.y > 0.8f)
    //{
    //    pitch = 0.0f;
    //}

    float distance = (m_position - targetPoint).Length() + zoom;

    m_position -= targetPoint;

    // Calculate the camera position using the distance and angles
    m_position.x = distance * -sinf(yaw) * cosf(pitch);
    m_position.y = distance * -sinf(pitch);
    m_position.z = -distance * cosf(yaw) * cosf(pitch);

    m_position += targetPoint;


    // Create the view matrix from the three updated vectors.
    m_viewMatrix = XMMatrixLookAtLH(m_position.GetAsXMVector(),
                                    targetPoint.GetAsXMVector(),
                                    up.GetAsXMVector());

    return;
}