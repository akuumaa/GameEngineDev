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

#include "engine/src/sim/entityspawner.h"

#include "engine/src/math/quaternion.h"
#include "engine/src/math/random.h"
#include "engine/src/math/vector3.h"

#include <cmath>
#include <cstddef>
#include <numbers>
#include <vector>

using Engine::Math::Quaternion;
using Engine::Math::Random;
using Engine::Math::Vector3;

namespace Engine::Sim
{
    std::vector<size_t> EntitySpawner::spawnProcedural(
        Simulation &simulation,
        const SpawnConfig &config
    )
    {
        std::vector<SpawnPoint> points = EntitySpawner::spawnPointGen(config);
        std::vector<size_t> spawnedIds = {};

        spawnedIds.reserve(points.size());

        if (config.count <= 0 || config.entityPaths.empty())
        {
            return spawnedIds;
        }

        spawnedIds.reserve(static_cast<size_t>(config.count));

        spawnedIds = EntitySpawner::spawnEntities(simulation, points);

        return spawnedIds;
    }

    size_t EntitySpawner::spawnAt(
        Simulation &simulation,
        const SpawnPoint &spawnPoint
    )
    {
        size_t spawnedId = 0;

        // Instantiate Entity
        if (auto entity = simulation.loadEntity(spawnPoint.entityPath))
        {
            entity->setPosition(spawnPoint.position);
            entity->setRotation(spawnPoint.rotation);
            entity->setScale(spawnPoint.scale);
            spawnedId = entity->getEntityId();
        }
        return spawnedId;
    }

    std::vector<size_t> EntitySpawner::spawnEntities(
        Simulation &simulation,
        const std::vector<SpawnPoint> &spawnPoint
    )
    {
        std::vector<size_t> spawnedIds = {};
        spawnedIds.reserve(spawnPoint.size());

        for (const SpawnPoint &spawnPoint : spawnPoint)
        {
            size_t spawnedId = EntitySpawner::spawnAt(simulation, spawnPoint);
            if (spawnedId != 0)
            {
                spawnedIds.push_back(spawnedId);
            }
        }

        return spawnedIds;
    }

    std::vector<SpawnPoint> EntitySpawner::spawnPointGen(
        const SpawnConfig &config
    )
    {
        std::vector<SpawnPoint> spawnPoints = {};
        spawnPoints.reserve(static_cast<size_t>(config.count));

        if (config.count <= 0 || config.entityPaths.empty())
        {
            return spawnPoints;
        };

        Random random(config.seed);

        std::vector<Vector3> clusterCenterPositions = EntitySpawner::positionGen(config, random);

        spawnPoints = EntitySpawner::clusterGen(config, clusterCenterPositions, random);
        return spawnPoints;
    }

    std::vector<Vector3> EntitySpawner::positionGen(
        const SpawnConfig &config,
        Random &random
    )
    {
        std::vector<Vector3> validPositions = {};
        validPositions.reserve(static_cast<size_t>(config.count));
        constexpr int maxAttempts = 15;

        for (int i = 0; i < config.count; i++)
        {
            for (int attempt = 0; attempt < maxAttempts; attempt++)
            {
                Vector3 positionCandidate = EntitySpawner::randomPositionInShape(config, random);
                if (isValidDistance(positionCandidate, validPositions, config.distanceMin))
                {
                    validPositions.push_back(positionCandidate);
                    break;
                }
            }
        }
        return validPositions;
    }

    bool EntitySpawner::isValidDistance(
        const Vector3 &positionCandidate,
        const std::vector<Vector3> &validPositions,
        const double minEntityDistance
    )
    {
        if (minEntityDistance <= 0.0)
        {
            return true;
        }
        
        for (const Vector3 &position : validPositions)
        {
            if ((positionCandidate - position).length() < minEntityDistance)
            {
                return false;
            }
            return true;
        }
    }

    Vector3 EntitySpawner::randomPositionInShape(const SpawnConfig &config, Math::Random &random)
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

        return {x, y, z};
    }

    std::vector<SpawnPoint> EntitySpawner::clusterGen(
        const SpawnConfig &config,
        const std::vector<Vector3> &clusterCenterPositions,
        Random &random
    )
    {
        std::vector<SpawnPoint> spawnPoints = {};

        for (const Vector3 &centerPos : clusterCenterPositions)
        {
            std::vector<Vector3> finalPositions = {};

            if (config.clusterCountMax <= 1)
            {
                finalPositions.push_back(centerPos);
            }
            else
            {
                // Reuse noiseGen with a mini config centered at `centerPos`!
                SpawnConfig localClusterConfig = config;
                localClusterConfig.center = centerPos;
                localClusterConfig.count = random.nextInt(config.clusterCountMin, config.clusterCountMax);
                localClusterConfig.shape = SpawnShape::CylinderRing;
                localClusterConfig.radiusMin = 0.0;
                localClusterConfig.radiusMax = config.clusterRadius;
                localClusterConfig.heightMin = -config.clusterRadius * 0.5;
                localClusterConfig.heightMax = config.clusterRadius * 0.5;
                finalPositions = EntitySpawner::positionGen(localClusterConfig, random);
            }
            // Convert each position into a SpawnPoint (with random rotation, scale, and model)
            for (const Vector3 &pos : finalPositions)
            {
                double size = random.nextDouble(config.scaleMin, config.scaleMax);
                double yaw = random.nextDouble(config.rotationMin.x, config.rotationMax.x);
                double pitch = random.nextDouble(config.rotationMin.y, config.rotationMax.y);
                double roll = random.nextDouble(config.rotationMin.z, config.rotationMax.z);
                size_t pathIndex = 0;
                if (config.entityPaths.size() > 1)
                {
                    pathIndex = static_cast<size_t>(
                        random.nextInt(0, static_cast<int>(config.entityPaths.size()) - 1)
                    );
                }
                spawnPoints.push_back({.entityPath = config.entityPaths[pathIndex], .position = pos, .rotation = Quaternion(yaw, pitch, roll), .scale = Vector3(size, size, size)});
            }
        }
        return spawnPoints;
    }

}
