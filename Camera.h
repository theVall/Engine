#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class Camera
{
public:
    Camera();
    Camera(const Camera &);
    ~Camera();

    void SetPosition(float, float, float);
    void SetRotation(float, float, float);

    XMFLOAT3 GetPosition();
    XMFLOAT3 GetRotation();
    void GetViewMatrix(XMMATRIX &viewMatrix);

    void Render();
    // Orbital camera mode "trackball camera"
    void RenderOrbital(float zoom);


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
    XMFLOAT3 m_position;
    XMFLOAT3 m_rotation;

    XMMATRIX m_viewMatrix;
};

