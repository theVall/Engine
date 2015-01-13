#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

namespace math
{
class Vec4f
{
public:

    // Constructors
    Vec4f()
    {
        m_v = GetFromVec4f(XMVectorZero());
    }

    Vec4f(float x, float y, float z, float w)
    {
        this->m_v.x = x;
        this->m_v.y = y;
        this->m_v.z = z;
        this->m_v.w = w;
    }

    Vec4f(float xyzw)
    {
        this->m_v.x = xyzw;
        this->m_v.y = xyzw;
        this->m_v.z = xyzw;
        this->m_v.w = xyzw;
    }

    Vec4f(const Vec4f &v)
    {
        this->m_v = v.m_v;
    }

    Vec4f(const XMFLOAT4 &v)
    {
        this->m_v = v;
    }

    ~Vec4f()
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

    inline float GetW() const
    {
        return this->m_v.w;
    }

    inline float GetAxis(UINT axis) const
    {
        switch (axis)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
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

    inline void SetW(float w)
    {
        this->m_v.w = w;
    }

    inline void Set(float x, float y, float z, float w)
    {
        this->m_v.x = x;
        this->m_v.y = y;
        this->m_v.z = z;
        this->m_v.w = w;
    }


    // Addition
    inline void Add(const Vec4f &v)
    {
        this->m_v.x += v.x;
        this->m_v.y += v.y;
        this->m_v.z += v.z;
        this->m_v.w += v.w;
    }

    inline void Add(float x, float y, float z, float w)
    {
        this->m_v.x += x;
        this->m_v.y += y;
        this->m_v.z += z;
        this->m_v.w += w;
    }


    // Subtraction
    inline void Sub(const Vec4f &v)
    {
        this->m_v.x -= v.x;
        this->m_v.y -= v.y;
        this->m_v.z -= v.z;
        this->m_v.w -= v.w;
    }

    inline void Sub(float x, float y, float z, float w)
    {
        this->m_v.x -= x;
        this->m_v.y -= y;
        this->m_v.z -= z;
        this->m_v.w -= w;
    }


    // Scaling
    inline void Scale(float s)
    {
        this->m_v.x *= s;
        this->m_v.y *= s;
        this->m_v.z *= s;
        this->m_v.w *= s;
    }

    inline void Scale(const math::Vec4f &vec)
    {
        this->m_v.x *= vec.x;
        this->m_v.y *= vec.y;
        this->m_v.z *= vec.z;
        this->m_v.w *= vec.w;
    }


    // Other vector operations
    inline float Length()
    {
        XMVECTOR tmp = XMVector4Length(XMLoadFloat4(&m_v));
        return GetFromVec4f(tmp).x;
    }

    inline Vec4f Normalize()
    {
        XMVECTOR tmp = XMVector4Normalize(XMLoadFloat4(&m_v));
        m_v = GetFromVec4f(tmp);
        return *this;
    }

    inline void Homogenize(void)
    {
        if (m_v.w != 0)
        {
            m_v.x = m_v.x / m_v.w;
            m_v.y = m_v.y / m_v.w;
            m_v.z = m_v.z / m_v.w;
            m_v.w = 1;
        }
    }

    inline float Dot(const Vec4f &v)
    {
        return Vec4f::GetDot(*this, v);
    }


    //inline void Print(void) const
    //{
    //    DEBUG_OUT_A("Vec4f (%f, %f, %f, %f)\n", this->m_v.x, this->m_v.y, this->m_v.z, this->m_v.w);
    //}


    // Operators
    Vec4f operator+(const Vec4f &right) const
    {
        Vec4f s(*this);
        s.Add(right);
        return s;
    }

    Vec4f &operator+=(const Vec4f &right)
    {
        this->m_v.x += right.x;
        this->m_v.y += right.y;
        this->m_v.z += right.z;
        return *this;
    }

    Vec4f operator-(const Vec4f &right) const
    {
        Vec4f s(*this);
        s.Sub(right);
        return s;
    }

    Vec4f &operator-=(const Vec4f &right)
    {
        this->m_v.x -= right.x;
        this->m_v.y -= right.y;
        this->m_v.z -= right.z;
        return *this;
    }

    Vec4f operator*(const Vec4f &right) const
    {
        Vec4f s(*this);
        s.Scale(right);
        return s;
    }

    Vec4f &operator*=(const Vec4f &right)
    {
        this->m_v.x *= right.x;
        this->m_v.y *= right.y;
        this->m_v.z *= right.z;
        return *this;
    }

    Vec4f operator*(float s) const
    {
        math::Vec4f v(*this);
        v.Scale(s);
        return v;
    }

    Vec4f &operator*=(float right)
    {
        this->m_v.x *= right;
        this->m_v.y *= right;
        this->m_v.z *= right;
        return *this;
    }

    Vec4f operator/(float s) const
    {
        math::Vec4f v(*this);
        v.Scale(1.0f / s);
        return v;
    }

    Vec4f &operator/=(float right)
    {
        this->m_v.x = this->m_v.x / right;
        this->m_v.y = this->m_v.y / right;
        this->m_v.z = this->m_v.z / right;
        return *this;
    }

    Vec4f operator-() const
    {
        Vec4f v(*this);
        v.Scale(-1.f);
        return v;
    }

    Vec4f &operator=(const Vec4f &vec)
    {
        m_v.x = vec.m_v.x;
        m_v.y = vec.m_v.y;
        m_v.z = vec.m_v.z;
        return *this;
    }

    bool operator==(const Vec4f &vec)
    {
        if (m_v.x == vec.m_v.x)
        {
            if (m_v.y == vec.m_v.y)
            {
                if (m_v.z == vec.m_v.z)
                {
                    if (m_v.w == vec.m_v.w)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool operator!=(const Vec4f &vec)
    {
        if (m_v.x != vec.m_v.x)
        {
            return true;
        }
        else if (m_v.y != vec.m_v.y)
        {
            return true;
        }
        else if (m_v.z != vec.m_v.z)
        {
            return true;
        }
        else if (m_v.w != vec.m_v.w)
        {
            return true;
        }
        return false;
    }


    // static functions
    inline static float GetDot(const Vec4f &v0, const Vec4f &v1)
    {
        XMVECTOR m_v = XMVector4Dot(XMLoadFloat4(&v0.m_v), XMLoadFloat4(&v1.m_v));
        return GetFromVec4f(m_v).x;
    }


    inline static Vec4f GetNormalize(const Vec4f &vec)
    {
        XMVECTOR m_v = XMVector4Normalize(XMLoadFloat4(&vec.m_v));
        return Vec4f(GetFromVec4f(m_v));
    }


    inline static XMFLOAT4 GetFromVec4f(XMVECTOR v)
    {
        XMFLOAT4 m_v;
        XMStoreFloat4(&m_v, v);
        return m_v;
    }


    inline XMVECTOR GetAsXMVector()
    {
        return XMLoadFloat4(&m_v);
    }


    inline XMFLOAT4 GetAsXMFloat4()
    {
        return m_v;
    }


    __declspec(property(get = GetX, put = SetX)) float x;
    __declspec(property(get = GetY, put = SetY)) float y;
    __declspec(property(get = GetZ, put = SetZ)) float z;
    __declspec(property(get = GetW, put = SetW)) float w;

    __declspec(property(get = GetX, put = SetX)) float r;
    __declspec(property(get = GetY, put = SetY)) float g;
    __declspec(property(get = GetZ, put = SetZ)) float b;
    __declspec(property(get = GetW, put = SetW)) float a;

private:

    XMFLOAT4 m_v;
};
}