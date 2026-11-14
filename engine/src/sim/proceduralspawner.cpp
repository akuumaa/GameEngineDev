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

#include "engine/src/sim/proceduralspawner.h"

#include "engine/src/math/quaternion.h"
#include "engine/src/math/random.h"

#include <cmath>
#include <numbers>

using Engine::Math::Quaternion;
using Engine::Math::Random;
using Engine::Math::Vector3;

namespace Engine::Sim
{
    std::vector<size_t> ProceduralSpawner::spawn(
        Simulation &simulation,
        const SpawnConfig &config)
    {
        std::vector<size_t> spawnedIds = {};
        if (config.count <= 0 || config.entityPaths.empty())
        {
            return spawnedIds;
        }

        spawnedIds.reserve(static_cast<size_t>(config.count));
        Random random(config.seed);
        
        for (int i = 0; i < config.count; i++)
        {
            // Horizontal (X, Y) placement based on shape
            double x = config.center.x;
            double y = config.center.y;

            if (config.shape == SpawnShape::CylinderRing)
            {
                double angle = random.nextDouble(0.0, 2.0 * std::numbers::pi);
                double radius = random.nextDouble(config.radiusMin, config.radiusMax);
                x += std::cos(angle) * radius;
                y += std::sin(angle) * radius;
            }
            else if (config.shape == SpawnShape::Box)
            {
                x += random.nextDouble(-config.boxExtentX, config.boxExtentX);
                y += random.nextDouble(-config.boxExtentY, config.boxExtentY);
            }

            // Vertical (Z) placement: free 3D volume vs. ground-snapped
            double z = config.center.z;
            if (config.snapToGround)
            {
                // TODO: Logic for snapping Generation to Groundterrain if needed
            }
            else
            {
                z += random.nextDouble(config.heightMin, config.heightMax);
            }

            // Scale & Rotation
            double size = random.nextDouble(config.scaleMin, config.scaleMax);
            double yaw = config.randomYaw ? random.nextDouble(0.0, 2.0 * std::numbers::pi) : 0.0;
            double pitch = config.randomPitchRoll ? random.nextDouble(0.0, 2.0 * std::numbers::pi) : 0.0;
            double roll = config.randomPitchRoll ? random.nextDouble(0.0, 2.0 * std::numbers::pi) : 0.0;

            // Model / Entity selection
            size_t pathIndex = 0;
            if (config.entityPaths.size() > 1)
            {
                pathIndex = static_cast<size_t>(
                    random.nextInt(0, static_cast<int>(config.entityPaths.size()) - 1));
            }

            // Instantiate Entity
            if (auto entity = simulation.loadEntity(config.entityPaths[pathIndex]))
            {
                entity->setPosition(Vector3(x, y, z));
                entity->setRotation(Quaternion(yaw, pitch, roll));
                entity->setScale(Vector3(size, size, size));

                spawnedIds.push_back(entity->getEntityId());
            }
        }

        return spawnedIds;
    }
}
