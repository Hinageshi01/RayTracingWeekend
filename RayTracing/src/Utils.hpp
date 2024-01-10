#pragma once

inline constexpr float MY_PI = 3.1415926535897932384626433832795;
inline constexpr float MY_PI2 = 9.8696044010893586188344909998762;
inline constexpr float MY_PI_INV = 0.31830988618379067153776752674503;
inline constexpr float MY_PI2_INV = 0.10132118364233777144387946320973;

__forceinline inline float DegreesToRadians(float degrees)
{
    return degrees * MY_PI / 180.0f;
}
