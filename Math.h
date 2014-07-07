#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

namespace math
{
    // single precision constants
    //
    const float F_PI = 3.141592653f;
    const float F_PIDIV2 = 1.570796326f;
    const float F_PIDIV4 = 0.785398163f;
    const float F_1DIVPI = 0.318309886f;
    const float F_1DIV2PI = 0.159154943f;

    const float F_SQRT_2 = 1.414213562f;
    const float F_HALF_SQRT_2 = 0.707106781f;
    const float F_EULER = 2.718281828f;

    // double precision constants
    //
    const double D_PI = 3.1415926535897932384626433832795028841971693993751058;
    const double D_PIDIV2 = 1.5707963267948966192313216916397514420985846996875529;
    const double D_PIDIV4 = 0.7853981633974483096156608458198757210492923498437764;
    const double D_1DIVPI = 0.3183098861837906715377675267450287240689192914809128;
    const double D_1DIV2PI = 0.1591549430918953357688837633725143620344596457404564;

    const double D_SQRT_2 = 1.4142135623730950488016887242096980785696718753769480;
    const double D_HALF_SQRT_2 = 0.7071067811865475244008443621048490392848359376884740;
    const double D_EULER = 2.7182818284590452353602874713526624977572470936999595;

    class Math
    {
    public:

    };
}