/**
 * Copyright © 2026 Alexander Hancharenka
*/
#pragma once

#include "engine/src/math/vector3.h"
#include "engine/src/math/bounds.h"

#include <cstdint>

using Engine::Math::Bounds;
using Engine::Math::Vector3;

namespace App 
{
    struct TankConfig
    {
        Vector3 size = {800.0, 400.0, 500.0};
    };

    struct FloorConfig
    {
        double baseRatio = 0.05;
        double slopeRatio = 0.08;

        double duneScale = 0.008;
        double duneHeight = 25.0;
        int duneOctaves = 4;

        double rippleScale = 0.08;
        double rippleHeight = 2.0;

        int cellsX = 32;
        int cellsY = 16;
    };

    struct AquariumConfig
    {
        uint32_t seed = 1337;
        TankConfig tank = {};
        double waterFillRatio = 0.8;
        double floorMaxRatio = 0.2;
        double shipSpawnRatio = 0.3;
        FloorConfig floor = {};
    };
    
    struct AquariumLayout
    {
        Bounds tank = {};
        double floorTop = 0.0;
        double surfaceHeight = 0.0;
        Vector3 shipSpawn = {};

        static AquariumLayout from (const AquariumConfig &config)
        {
            AquariumLayout l;
            Vector3 half = config.tank.size * 0.5;
            l.tank = {{-half.x, -half.y, 0.0}, {half.x, half.y, config.tank.size.z}};
            l.floorTop      = config.tank.size.z * config.floorMaxRatio;
            l.surfaceHeight = config.tank.size.z * config.waterFillRatio;
            l.shipSpawn     = {0.0, 0.0, l.floorTop + (l.surfaceHeight - l.floorTop) * 0.3};
            return l;
        }

        bool isUnderwater(const Vector3 &point) const { return point.z < surfaceHeight; }
    };
}