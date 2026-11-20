/**
 * Copyright © 2026 Alexander Hancharenka
*/

#pragma 

#include "engine/src/math/vector3.h"
#include <algorithm>

namespace Engine::Math
{
    struct Bounds
    {
        Vector3 min = {};
        Vector3 max = {};

        Vector3 size() const { return max - min; }
        Vector3 center() const { return (min + max) * 0.5; }

        bool contains(const Vector3 &point) const
        {
            return point.x >= min.x && point.x <= max.x
                && point.y >= min.y && point.y <= max.y
                && point.z >= min.z && point.z <= max.z;
        }
        
        Vector3 clamp(const Vector3 &point) const
        {
            return {
                std::clamp(point.x, min.x, max.x),
                std::clamp(point.y, min.y, max.y),
                std::clamp(point.z, min.z, max.z),
            };
        }
    };
}