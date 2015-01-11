#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Vec4f.h"

using namespace DirectX;

namespace math
{
class Vec3f
{
public:
    static const Vec3f X_AXIS;
    static const Vec3f Y_AXIS;
    static const Vec3f Z_AXIS;

    // Constructors
    Vec3f()
    {
        m_v = GetFromVector3(XMVectorZero());
    }

    Vec3f(float x, float y, float z)
    {
        this->m_v.x = x;
        this->m_v.y = y;
        this->m_v.z = z;
    }

    Vec3f(float xyz)
    {
        this->m_v.x = xyz;
        this->m_v.y = xyz;
        this->m_v.z = xyz;
    }

    Vec3f(const Vec3f &v)
    {
        this->m_v = v.m_v;
    }

    Vec3f(const XMFLOAT3 &v)
    {
        this->m_v = v;
    }

    ~Vec3f()
    {
    }


    // Getter
    inline float GetX() const
    {
        return this->m_v.x;
    }

    inline float GetY() const
    {
        return this->m_v.y;
    }

    inline float GetZ() const
    {
        return this->m_v.z;
    }

    inline float GetAxis(UINT axis) const
    {
        switch (axis)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        }
        return 0;
    }


    // Setter
    inline void SetX(float x)
    {
        this->m_v.x = x;
    }

    inline void SetY(float y)
    {
        this->m_v.y = y;
    }

    inline void SetZ(float z)
    {
        this->m_v.z = z;
    }

    inline void Set(float x, float y, float z)
    {
        this->m_v.x = x;
        this->m_v.y = y;
        this->m_v.z = z;
    }

    inline void Set(Vec3f &set)
    {
        this->m_v.x = set.x;
        this->m_v.y = set.y;
        this->m_v.z = set.z;
    }


    // Addition
    inline void Add(const Vec3f &v)
    {
        this->m_v.x += v.x;
        this->m_v.y += v.y;
        this->m_v.z += v.z;
    }

    inline void Add(float x, float y, float z)
    {
        this->m_v.x += x;
        this->m_v.y += y;
        this->m_v.z += z;
    }


    // Subtraction
    inline void Sub(float x, float y, float z)
    {
        this->m_v.x -= x;
        this->m_v.y -= y;
        this->m_v.z -= z;
    }

    inline void Sub(const Vec3f &v)
    {
        this->m_v.x -= v.x;
        this->m_v.y -= v.y;
        this->m_v.z -= v.z;
    }


    // Scaling
    inline void Scale(float s)
    {
        this->m_v.x *= s;
        this->m_v.y *= s;
        this->m_v.z *= s;
    }

    inline void Scale(const Vec3f &vec)
    {
        this->m_v.x *= vec.x;
        this->m_v.y *= vec.y;
        this->m_v.z *= vec.z;
    }


    // Other vector operations
    inline float Length()
    {
        XMVECTOR tmp = XMVector3Length(GetFromFloat3(m_v));
        return GetFromVector3(tmp).x;
    }

    inline Vec3f &Normalize()
    {
        XMVECTOR tmp = XMVector3Normalize(GetFromFloat3(m_v));
        m_v = GetFromVector3(tmp);
        return *this;
    }

    inline float Dot(const Vec3f &v) const
    {
        //return m_v.x*v.x + m_v.y*v.y + m_v.z*v.z;
        return XMVector3Dot(XMLoadFloat3(&this->m_v), XMLoadFloat3(&v.m_v)).m128_f32[0];
        //return Vec3f::GetDot(*this, v);
    }


    // Operators
    Vec3f operator+(const Vec3f &right) const
    {
        Vec3f s(*this);
        s.Add(right);
        return s;
    }

    Vec3f &operator+=(const Vec3f &right)
    {
        this->m_v.x += right.x;
        this->m_v.y += right.y;
        this->m_v.z += right.z;
        return *this;
    }

    Vec3f operator-(const Vec3f &right) const
    {
        Vec3f s(*this);
        s.Sub(right);
        return s;
    }

    Vec3f &operator-=(const Vec3f &right)
    {
        this->m_v.x -= right.x;
        this->m_v.y -= right.y;
        this->m_v.z -= right.z;
        return *this;
    }

    Vec3f operator*(const Vec3f &right) const
    {
        Vec3f s(*this);
        s.Scale(right);
        return s;
    }

    Vec3f &operator*=(const Vec3f &right)
    {
        this->m_v.x *= right.x;
        this->m_v.y *= right.y;
        this->m_v.z *= right.z;
        return *this;
    }

    Vec3f operator*(float s) const
    {
        Vec3f v(*this);
        v.Scale(s);
        return v;
    }

    Vec3f &operator*=(float right)
    {
        this->m_v.x *= right;
        this->m_v.y *= right;
        this->m_v.z *= right;
        return *this;
    }

    Vec3f operator/(float s) const
    {
        Vec3f v(*this);
        v.Scale(1.0f / s);
        return v;
    }

    Vec3f &operator/=(float right)
    {
        this->m_v.x = this->m_v.x / right;
        this->m_v.y = this->m_v.y / right;
        this->m_v.z = this->m_v.z / right;
        return *this;
    }

    Vec3f operator-() const
    {
        Vec3f v(*this);
        v.Scale(-1.f);
        return v;
    }

    Vec3f &operator=(const Vec3f &vec)
    {
        m_v.x = vec.m_v.x;
        m_v.y = vec.m_v.y;
        m_v.z = vec.m_v.z;
        return *this;
    }


    //inline void Print(void) const
    //{
    //    DEBUG_OUT_A("Vec3f (%f, %f, %f)\n", this->m_v.x, this->m_v.y, this->m_v.z);
    //}


    // Static functions
    inline static float GetDot(const Vec3f &v0, const Vec3f &v1)
    {
        return XMVector3Dot(XMLoadFloat3(&v0.m_v), XMLoadFloat3(&v1.m_v)).m128_f32[0];
    }


    inline static Vec3f Sub(Vec3f &v0, Vec3f &v1)
    {
        Vec3f v(v0);
        v.Sub(v1);
        return v;
    }


    inline static Vec3f GetNormalize(const Vec3f &vec)
    {
        XMVECTOR m_v = XMVector3Normalize(GetFromFloat3(vec.m_v));
        return GetFromVector3(m_v);
    }


    inline static Vec3f GetCross(const Vec3f &v0, const Vec3f v1)
    {
        XMVECTOR m_v0 = GetFromFloat3(v0.m_v);
        XMVECTOR m_v1 = GetFromFloat3(v1.m_v);
        return GetFromVector3(XMVector3Cross(m_v0, m_v1));
    }


    inline static Vec3f GetCross(const Vec4f &v0, const Vec4f v1)
    {
        Vec3f m_v0(v0.x, v0.y, v0.z);
        Vec3f m_v1(v1.x, v1.y, v1.z);
        return Vec3f::GetCross(m_v0, m_v1);
    }


    inline static Vec3f Min(const Vec3f &p0, const Vec3f &p1)
    {
        Vec3f result;
        result.x = min(p0.x, p1.x);
        result.y = min(p0.y, p1.y);
        result.z = min(p0.z, p1.z);
        return result;
    }


    inline static Vec3f Max(const Vec3f &p0, const Vec3f &p1)
    {
        Vec3f result;
        result.x = max(p0.x, p1.x);
        result.y = max(p0.y, p1.y);
        result.z = max(p0.z, p1.z);
        return result;
    }


    inline static Vec3f lerp(const Vec3f &p0, const Vec3f &p1, float t)
    {
        Vec3f result;
        result.x = p0.x * (1 - t) + t * p1.x;
        result.y = p0.y * (1 - t) + t * p1.y;
        result.z = p0.z * (1 - t) + t * p1.z;
        return result;
    }


    inline static XMVECTOR GetFromFloat3(XMFLOAT3 v)
    {
        return XMLoadFloat3(&v);
    }


    inline static XMFLOAT3 GetFromVector3(XMVECTOR v)
    {
        XMFLOAT3 m_v;
        XMStoreFloat3(&m_v, v);
        return m_v;
    }


    inline XMVECTOR GetAsXMVector()
    {
        return XMLoadFloat3(&m_v);
    }


    inline XMFLOAT3 GetAsXMFloat3()
    {
        return m_v;
    }


    // 3D position
    __declspec(property(get = GetX, put = SetX)) float x;
    __declspec(property(get = GetY, put = SetY)) float y;
    __declspec(property(get = GetZ, put = SetZ)) float z;

    // texture coordinate
    __declspec(property(get = GetX, put = SetX)) float u;
    __declspec(property(get = GetY, put = SetY)) float v;

    // color
    __declspec(property(get = GetX, put = SetX)) float r;
    __declspec(property(get = GetY, put = SetY)) float g;
    __declspec(property(get = GetZ, put = SetZ)) float b;


private:
    XMFLOAT3 m_v;
};
}