/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2026 Enrico Leidner
 * Copyright © 2026 Alexander Hancharenka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "engine/src/math/vector3.h"
#include "engine/src/sim/simulation.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

using Engine::Math::Vector3;

namespace Engine::Sim
{
    enum class SpawnShape
    {
        CylinderRing,
        Box
    };

    /// Parameters for procedural entity generation, structured like an inspector/GUI panel.
    struct SpawnConfig
    {
        uint32_t seed = 1337;
        int count = 10;

        std::vector<std::string> entityPaths = {};

        SpawnShape shape = SpawnShape::CylinderRing;
        Vector3 center = {0.0, 0.0, 0.0};

        // CylinderRing parameters
        double radiusMin = 80.0;
        double radiusMax = 400.0;

        // Box parameters (half-extents along X and Y)
        double boxExtentX = 200.0;
        double boxExtentY = 200.0;

        // Vertical range (used when snapToGround == false)
        double heightMin = -60.0;
        double heightMax = 60.0;

        double scaleMin = 1.0;
        double scaleMax = 1.0;
        // TODO: Add more control for selected axes not just random true/false
        Vector3 minRotation = {};
        Vector3 maxRotation = {};

        bool randomYaw = true;
        bool randomPitchRoll = true;

        // TODO: Add more control over Distance between Entities
        double distanceMin = 0.0;

        bool snapToGround = false;
        double groundHeight = 0.0;
        double groundOffset = 0.0;
    };

    class EntitySpawner
    {
      public:
        using GroundHeightFunc = std::function<double(double x, double y)>;

        static std::vector<size_t> spawnAt(
            // TODO: Spawn a singel Entity at a defined position
        );
        
        static std::vector<size_t> spawnAtPositions(
            // TODO: Spawn multiple Entities at defined positions 
        );

        static std::vector<size_t> spawnProcedural(
            Simulation &simulation,
            const SpawnConfig &config
        );
    };
}
