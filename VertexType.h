#pragma once

#include "Vec3f.h"
#include "Vec4f.h"

using namespace math;

class VertexType
{
public:
    VertexType() {};
    ~VertexType() {};

public:
    Vec3f position;
    Vec3f texture;
    Vec3f normal;
    Vec4f color;
};

