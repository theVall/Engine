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

    float a;
    float b;
    float c;
    float d;

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
    tmpMatrix = viewMatrix * projectionMatrixCopy;
    // tmpMatrix = XMMatrixTranspose(tmpMatrix);
    XMStoreFloat4x4(&tmpFloatMatrix, tmpMatrix);

    // Calculate near plane of frustum.
    a = tmpFloatMatrix._13;
    b = tmpFloatMatrix._23;
    c = tmpFloatMatrix._33;
    d = tmpFloatMatrix._43;
    m_planes[0].Init(a, b, c, d);

    // Calculate far plane of frustum.
    a = tmpFloatMatrix._14 - tmpFloatMatrix._13;
    b = tmpFloatMatrix._24 - tmpFloatMatrix._23;
    c = tmpFloatMatrix._34 - tmpFloatMatrix._33;
    d = tmpFloatMatrix._44 - tmpFloatMatrix._43;
    m_planes[1].Init(a, b, c, d);

    // Calculate left plane of frustum.
    a = tmpFloatMatrix._14 + tmpFloatMatrix._11;
    b = tmpFloatMatrix._24 + tmpFloatMatrix._21;
    c = tmpFloatMatrix._34 + tmpFloatMatrix._31;
    d = tmpFloatMatrix._44 + tmpFloatMatrix._41;
    m_planes[2].Init(a, b, c, d);

    // Calculate right plane of frustum.
    a = tmpFloatMatrix._14 - tmpFloatMatrix._11;
    b = tmpFloatMatrix._24 - tmpFloatMatrix._21;
    c = tmpFloatMatrix._34 - tmpFloatMatrix._31;
    d = tmpFloatMatrix._44 - tmpFloatMatrix._41;
    m_planes[3].Init(a, b, c, d);

    // Calculate top plane of frustum.
    a = tmpFloatMatrix._14 - tmpFloatMatrix._12;
    b = tmpFloatMatrix._24 - tmpFloatMatrix._22;
    c = tmpFloatMatrix._34 - tmpFloatMatrix._32;
    d = tmpFloatMatrix._44 - tmpFloatMatrix._42;
    m_planes[4].Init(a, b, c, d);

    // Calculate bottom plane of frustum.
    a = tmpFloatMatrix._14 + tmpFloatMatrix._12;
    b = tmpFloatMatrix._24 + tmpFloatMatrix._22;
    c = tmpFloatMatrix._34 + tmpFloatMatrix._32;
    d = tmpFloatMatrix._44 + tmpFloatMatrix._42;
    m_planes[5].Init(a, b, c, d);

    return;
}


bool Frustum::CheckPoint(Vec3f vec)
{
    // Check if the point is inside all six planes of the view frustum.
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (m_planes[i].IsInside(vec))
        {
            return false;
        }
    }

    return true;
}


bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
    // Check if any one point of the cube is in the view frustum.
    // ATTENTION: y-value is taken 4x because of terrain quad-tree (got no y-Center)
    // for the use with other cubes a separate function should be used/implemented
    for (int i = 0; i < NUM_PLANES; i++)
    {
        if (m_planes[i].IsInside(Vec3f((xCenter), (yCenter), (zCenter))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - radius), (yCenter - 4*radius), (zCenter - radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + radius), (yCenter - 4*radius), (zCenter - radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - radius), (yCenter + 4*radius), (zCenter - radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + radius), (yCenter + 4*radius), (zCenter - radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - radius), (yCenter - 4*radius), (zCenter + radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + radius), (yCenter - 4*radius), (zCenter + radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - radius), (yCenter + 4*radius), (zCenter + radius))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + radius), (yCenter + 4*radius), (zCenter + radius))))
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
        // TODO negative radius?
        //if (m_planes[i].GetDistance(Vec3f(xCenter, yCenter, zCenter)) < -radius)
        //{
        //    return false;
        //}
        if (m_planes[i].IsInside(Vec3f(xCenter, yCenter, zCenter), -radius))
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
        if (m_planes[i].IsInside(Vec3f((xCenter - xSize), (yCenter - ySize), (zCenter - zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + xSize), (yCenter - ySize), (zCenter - zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - xSize), (yCenter + ySize), (zCenter - zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - xSize), (yCenter - ySize), (zCenter + zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + xSize), (yCenter + ySize), (zCenter - zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + xSize), (yCenter - ySize), (zCenter + zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter - xSize), (yCenter + ySize), (zCenter + zSize))))
        {
            continue;
        }

        if (m_planes[i].IsInside(Vec3f((xCenter + xSize), (yCenter + ySize), (zCenter + zSize))))
        {
            continue;
        }

        return false;
    }

    return true;
}



