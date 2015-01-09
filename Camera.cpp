#include "Camera.h"


Camera::Camera()
{
    m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
}


Camera::Camera(const Camera &)
{
}


Camera::~Camera()
{
}


void Camera::SetPosition(float x, float y, float z)
{
    m_position = XMFLOAT3(x, y, z);
    return;
}


void Camera::SetRotation(float x, float y, float z)
{
    m_rotation = XMFLOAT3(x, y, z);
    return;
}

XMFLOAT3 Camera::GetPosition()
{
    return m_position;
}


XMFLOAT3 Camera::GetRotation()
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
    XMVECTOR positionVec = XMLoadFloat3(&m_position);

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


void Camera::RenderOrbital(float zoom)
{
    XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 lookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 targetPoint = XMFLOAT3(1000.0f, 0.0f, 1000.0f);

    XMVECTOR upVec = XMLoadFloat3(&up);
    XMVECTOR positionVec = XMLoadFloat3(&m_position);
    XMVECTOR targetVec = XMLoadFloat3(&targetPoint);

    // Setup where the camera is looking by default.
    XMVECTOR lookAtVec = XMLoadFloat3(&lookAt) + targetVec;

    // Yaw and pitch in radians.
    float pitch = m_rotation.x * 0.0174532925f;
    float yaw = m_rotation.y * 0.0174532925f;

    // Create the rotation matrix from the yaw and pitch values.
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

    // Transform the look and up vectors with rotation matrix so the view is correctly rotated.
    positionVec = XMVector3TransformCoord(lookAtVec, rotationMatrix);

    // Convert the relative position to the absolute position
    positionVec *= zoom;
    positionVec += lookAtVec;

    // Create the view matrix from the three updated vectors.
    m_viewMatrix = XMMatrixLookAtLH(positionVec, lookAtVec, upVec);
    XMStoreFloat3(&m_position, positionVec);

    return;
}