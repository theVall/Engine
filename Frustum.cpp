#include "Frustum.h"


Frustum::Frustum()
{
}


Frustum::Frustum(const Frustum &)
{
}


Frustum::~Frustum()
{
}


void Frustum::ConstructFrustum(XMMATRIX &projectionMatrix,
                               XMMATRIX &viewMatrix,
                               float screenDepth)
{
    float zMinimum;
    float r;

    XMFLOAT4X4 projectionFloatMatrix;
    XMFLOAT4X4 viewFloatMatrix;
    XMFLOAT4X4 tmpFloatMatrix;

    XMMATRIX tmpMatrix;
    XMMATRIX projectionMatrixCopy;

    XMStoreFloat4x4(&projectionFloatMatrix, projectionMatrix);
    XMStoreFloat4x4(&viewFloatMatrix, viewMatrix);

    // Calculate the minimum Z distance in the frustum.
    zMinimum = -projectionFloatMatrix._43 / projectionFloatMatrix._33;
    r = screenDepth / (screenDepth - zMinimum);
    projectionFloatMatrix._33 = r;
    projectionFloatMatrix._43 = -r * zMinimum;

    projectionMatrixCopy = XMLoadFloat4x4(&projectionFloatMatrix);

    // Create the frustum matrix from the view matrix and updated projection matrix.
    tmpMatrix = viewMatrix * projectionMatrix;
    XMStoreFloat4x4(&tmpFloatMatrix, tmpMatrix);

    // Calculate near plane of frustum.
    m_planes[0].x = tmpFloatMatrix._14 + tmpFloatMatrix._13;
    m_planes[0].y = tmpFloatMatrix._24 + tmpFloatMatrix._23;
    m_planes[0].z = tmpFloatMatrix._34 + tmpFloatMatrix._33;
    m_planes[0].w = tmpFloatMatrix._44 + tmpFloatMatrix._43;
    m_planes[0] = normalize(m_planes[0]);

    // Calculate far plane of frustum.
    m_planes[1].x = tmpFloatMatrix._14 - tmpFloatMatrix._13;
    m_planes[1].y = tmpFloatMatrix._24 - tmpFloatMatrix._23;
    m_planes[1].z = tmpFloatMatrix._34 - tmpFloatMatrix._33;
    m_planes[1].w = tmpFloatMatrix._44 - tmpFloatMatrix._43;
    m_planes[1] = normalize(m_planes[1]);

    // Calculate left plane of frustum.
    m_planes[2].x = tmpFloatMatrix._14 + tmpFloatMatrix._11;
    m_planes[2].y = tmpFloatMatrix._24 + tmpFloatMatrix._21;
    m_planes[2].z = tmpFloatMatrix._34 + tmpFloatMatrix._31;
    m_planes[2].w = tmpFloatMatrix._44 + tmpFloatMatrix._41;
    m_planes[2] = normalize(m_planes[2]);

    // Calculate right plane of frustum.
    m_planes[3].x = tmpFloatMatrix._14 - tmpFloatMatrix._11;
    m_planes[3].y = tmpFloatMatrix._24 - tmpFloatMatrix._21;
    m_planes[3].z = tmpFloatMatrix._34 - tmpFloatMatrix._31;
    m_planes[3].w = tmpFloatMatrix._44 - tmpFloatMatrix._41;
    m_planes[3] = normalize(m_planes[3]);

    // Calculate top plane of frustum.
    m_planes[4].x = tmpFloatMatrix._14 - tmpFloatMatrix._12;
    m_planes[4].y = tmpFloatMatrix._24 - tmpFloatMatrix._22;
    m_planes[4].z = tmpFloatMatrix._34 - tmpFloatMatrix._32;
    m_planes[4].w = tmpFloatMatrix._44 - tmpFloatMatrix._42;
    m_planes[4] = normalize(m_planes[4]);

    // Calculate bottom plane of frustum.
    m_planes[5].x = tmpFloatMatrix._14 + tmpFloatMatrix._12;
    m_planes[5].y = tmpFloatMatrix._24 + tmpFloatMatrix._22;
    m_planes[5].z = tmpFloatMatrix._34 + tmpFloatMatrix._32;
    m_planes[5].w = tmpFloatMatrix._44 + tmpFloatMatrix._42;
    m_planes[5] = normalize(m_planes[5]);

    return;
}


bool Frustum::CheckPoint(float x, float y, float z)
{
    // Check if the point is inside all six planes of the view frustum.
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (planeDot(m_planes[i], XMFLOAT3(x, y, z)) < 0.0f)
        {
            return false;
        }
    }

    return true;
}


bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
    // Check if any one point of the cube is in the view frustum.
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (planeDot(m_planes[i], XMFLOAT3((xCenter - radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
        {
            continue;
        }

        return false;
    }

    return true;
}


bool Frustum::CheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
    // Check if the radius of the sphere is inside the view frustum.
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (planeDot(m_planes[i], XMFLOAT3(xCenter, yCenter, zCenter)) < -radius)
        {
            return false;
        }
    }

    return true;
}


bool Frustum::CheckRectangle(float xCenter,
                             float yCenter,
                             float zCenter,
                             float xSize,
                             float ySize,
                             float zSize)
{
    // Check if any of the 6 planes of the rectangle are inside the view frustum.
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (planeDot(m_planes[i], XMFLOAT3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
        {
            continue;
        }

        if (planeDot(m_planes[i], XMFLOAT3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
        {
            continue;
        }

        return false;
    }

    return true;
}



