#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Vec4f.h"

using namespace DirectX;

namespace math
{
    class Vec2f
    {
    public:
        static const Vec2f X_AXIS;
        static const Vec2f Y_AXIS;

        // Constructors
        Vec2f()
        {
            m_v = GetFromVector2(XMVectorZero());
        }

        Vec2f(float x, float y)
        {
            this->m_v.x = x;
            this->m_v.y = y;
        }

        Vec2f(const Vec2f& v)
        {
            this->m_v = v.m_v;
        }

        Vec2f(const XMFLOAT2& v)
        {
            this->m_v = v;
        }

        ~Vec2f()
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

        inline float GetAxis(UINT axis) const
        {
            switch (axis)
            {
            case 0: return x;
            case 1: return y;
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

        inline void Set(float x, float y)
        {
            this->m_v.x = x;
            this->m_v.y = y;
        }

        inline void Set(Vec2f& set)
        {
            this->m_v.x = set.x;
            this->m_v.y = set.y;
        }


        // Addition
        inline void Add(const Vec2f& v)
        {
            this->m_v.x += v.x;
            this->m_v.y += v.y;
        }

        inline void Add(float x, float y)
        {
            this->m_v.x += x;
            this->m_v.y += y;
        }


        // Subtraction
        inline void Sub(float x, float y)
        {
            this->m_v.x -= x;
            this->m_v.y -= y;
        }

        inline void Sub(const Vec2f& v)
        {
            this->m_v.x -= v.x;
            this->m_v.y -= v.y;
        }


        // Scaling
        inline void Scale(float s)
        {
            this->m_v.x *= s;
            this->m_v.y *= s;
        }

        inline void Scale(const Vec2f& vec)
        {
            this->m_v.x *= vec.x;
            this->m_v.y *= vec.y;
        }


        // Other vector operations
        inline float Length()
        {
            XMVECTOR tmp = XMVector2Length(GetFromFloat2(m_v));
            return GetFromVector2(tmp).x;
        }

        inline Vec2f& Normalize()
        {
            XMVECTOR tmp = XMVector2Normalize(GetFromFloat2(m_v));
            m_v = GetFromVector2(tmp);
            return *this;
        }

        inline float Dot(const Vec2f& v) const
        {
            //return m_v.x*v.x + m_v.y*v.y + m_v.z*v.z;
            return XMVector2Dot(XMLoadFloat2(&this->m_v), XMLoadFloat2(&v.m_v)).m128_f32[0];
            //return Vec2f::GetDot(*this, v);
        }


        // Operators
        Vec2f operator+(const Vec2f& right) const
        {
            Vec2f s(*this);
            s.Add(right);
            return s;
        }

        Vec2f& operator+=(const Vec2f& right)
        {
            this->m_v.x += right.x;
            this->m_v.y += right.y;
            return *this;
        }

        Vec2f operator-(const Vec2f& right) const
        {
            Vec2f s(*this);
            s.Sub(right);
            return s;
        }

        Vec2f& operator-=(const Vec2f& right)
        {
            this->m_v.x -= right.x;
            this->m_v.y -= right.y;
            return *this;
        }

        Vec2f operator*(const Vec2f& right) const
        {
            Vec2f s(*this);
            s.Scale(right);
            return s;
        }

        Vec2f& operator*=(const Vec2f& right)
        {
            this->m_v.x *= right.x;
            this->m_v.y *= right.y;
            return *this;
        }

        Vec2f operator*(float s) const
        {
            Vec2f v(*this);
            v.Scale(s);
            return v;
        }

        Vec2f& operator*=(float right)
        {
            this->m_v.x *= right;
            this->m_v.y *= right;
            return *this;
        }

        Vec2f operator/(float s) const
        {
            Vec2f v(*this);
            v.Scale(1.0f / s);
            return v;
        }

        Vec2f& operator/=(float right)
        {
            this->m_v.x = this->m_v.x / right;
            this->m_v.y = this->m_v.y / right;
            return *this;
        }

        Vec2f operator-() const
        {
            Vec2f v(*this);
            v.Scale(-1.f);
            return v;
        }

        Vec2f& operator=(const Vec2f& vec)
        {
            m_v.x = vec.m_v.x;
            m_v.y = vec.m_v.y;
            return *this;
        }


        //inline void Print(void) const
        //{
        //    DEBUG_OUT_A("Vec2f (%f, %f, %f)\n", this->m_v.x, this->m_v.y);
        //}


        // Static functions
        inline static float GetDot(const Vec2f& v0, const Vec2f& v1)
        {
            return XMVector2Dot(XMLoadFloat2(&v0.m_v), XMLoadFloat2(&v1.m_v)).m128_f32[0];
        }


        inline static Vec2f Sub(Vec2f& v0, Vec2f& v1)
        {
            Vec2f v(v0);
            v.Sub(v1);
            return v;
        }


        inline static Vec2f GetNormalize(const Vec2f& vec)
        {
            XMVECTOR m_v = XMVector2Normalize(GetFromFloat2(vec.m_v));
            return GetFromVector2(m_v);
        }


        inline static Vec2f GetCross(const Vec2f& v0, const Vec2f v1)
        {
            XMVECTOR m_v0 = GetFromFloat2(v0.m_v);
            XMVECTOR m_v1 = GetFromFloat2(v1.m_v);
            return GetFromVector2(XMVector2Cross(m_v0, m_v1));
        }


        inline static Vec2f GetCross(const Vec4f& v0, const Vec4f v1)
        {
            Vec2f m_v0(v0.x, v0.y);
            Vec2f m_v1(v1.x, v1.y);
            return Vec2f::GetCross(m_v0, m_v1);
        }


        inline static Vec2f Min(const Vec2f& p0, const Vec2f& p1)
        {
            Vec2f result;
            result.x = min(p0.x, p1.x);
            result.y = min(p0.y, p1.y);
            return result;
        }


        inline static Vec2f Max(const Vec2f& p0, const Vec2f& p1)
        {
            Vec2f result;
            result.x = max(p0.x, p1.x);
            result.y = max(p0.y, p1.y);
            return result;
        }


        inline static Vec2f lerp(const Vec2f& p0, const Vec2f& p1, float t)
        {
            Vec2f result;
            result.x = p0.x * (1 - t) + t * p1.x;
            result.y = p0.y * (1 - t) + t * p1.y;
            return result;
        }


        inline static XMVECTOR GetFromFloat2(XMFLOAT2 v)
        {
            return XMLoadFloat2(&v);
        }


        inline static XMFLOAT2 GetFromVector2(XMVECTOR v)
        {
            XMFLOAT2 m_v;
            XMStoreFloat2(&m_v, v);
            return m_v;
        }

        // 3D position
        __declspec(property(get = GetX, put = SetX)) float x;
        __declspec(property(get = GetY, put = SetY)) float y;

        // texture coordinate
        __declspec(property(get = GetX, put = SetX)) float u;
        __declspec(property(get = GetY, put = SetY)) float v;

        // color
        __declspec(property(get = GetX, put = SetX)) float r;
        __declspec(property(get = GetY, put = SetY)) float g;


    private:
        XMFLOAT2 m_v;
    };
}