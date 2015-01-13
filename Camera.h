#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Vec3f.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace math;

class Camera
{
public:
    Camera();
    Camera(const Camera &);
    ~Camera();

    void SetPosition(Vec3f pos);
    void SetRotation(Vec3f rot);

    Vec3f GetPosition();
    Vec3f GetRotation();
    void GetViewMatrix(XMMATRIX &viewMatrix);

    void Render();
    // Orbital camera mode "trackball camera"
    void RenderOrbital(Vec3f targetPoint, float zoom);


    // guarantee align 16 for heap allocations
    void *operator new(size_t size) throw()
    {
        return _aligned_malloc(size, 16);
    }

    void operator delete(void *ptr) throw()
    {
        _aligned_free(ptr);
    }

private:
    Vec3f m_position;
    Vec3f m_rotation;

    XMMATRIX m_viewMatrix;
};

