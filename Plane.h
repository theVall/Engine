#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Vec3f.h"

using namespace DirectX;

namespace math
{
    class Plane
    {
    public:
        // Constructors
        Plane()
        {
        }

        Plane(const Plane &)
        {
        }

        ~Plane()
        {
        }


        // Initialization
        // Initialize with three vectors.
        void Init(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2)
        {
            Vec3f t0 = p1 - p0;
            Vec3f t1 = p2 - p0;
            Vec3f cross = Vec3f::GetCross(t0, t1);
            cross.Normalize();
            float radius = Vec3f::GetDot(cross, p0);
            Init(cross, radius);
        }

        // Initialize with normal and radius
        void Init(const Vec3f& normal, float radius)
        {
            m_normal = normal;
            m_normal.Normalize();
            m_radius = radius;
        }

        // Initialize with four coefficients
        void Init(float a, float b, float c, float d)
        {
            m_normal.Set(a, b, c);
            float l = m_normal.Length();
            m_normal.Normalize();
            m_radius = d/l;
        }


        // Getter
        float GetDistance(const Vec3f& point) const
        {
            return point.Dot(m_normal) + m_radius;
        }

        bool IsInside(const Vec3f& point) const
        {
            return GetDistance(point) >= 0.0f;
        }

        bool IsInside(const Vec3f& point, float radius) const
        {
            return GetDistance(point) <= radius;
        }

        Vec3f GetNormaal() const
        {
            return m_normal;
        }

        float GetRadius() const
        {
            return m_radius;
        }

    private:

        float m_radius;
        math::Vec3f m_normal;
    };
}
