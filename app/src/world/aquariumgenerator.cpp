/**
 * Copyright © 2026 Alexander Hancharenka
*/

#include "app/src/world/aquariumgenerator.h"

#include "engine/src/math/noise.h"
#include "engine/src/sim/entityspawner.h"

#include <algorithm>
#include <vector>

using Engine::Math::Noise;
using Engine::Sim::EntitySpawner;
using Engine::Sim::SpawnPoint;

namespace App
{
    void AquariumGenerator::generate(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout)
    {
        buildFloorBlocks(simulation, config, layout);
    }

    double AquariumGenerator::floorHeight(const AquariumConfig &config, const AquariumLayout &layout, double x, double y)
    {
        const FloorConfig &floor = config.floor;
        double tankHeight = config.tank.size.z;

        Noise dunes(config.seed + 1);
        Noise ripples(config.seed + 2);

        double depth = (x - layout.tank.min.x) / layout.tank.size().x;

        double height = tankHeight * floor.baseRatio
                      + tankHeight * floor.slopeRatio * depth
                      + dunes.fractal(x * floor.duneScale, y * floor.duneScale, floor.duneOctaves) * floor.duneHeight
                      + ripples.sample(x * floor.rippleScale, y * floor.rippleScale) * floor.rippleHeight;

        return std::clamp(height, 1.0, layout.floorTop);
    }

    void AquariumGenerator::buildFloorBlocks(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout)
    {
        const FloorConfig &floor = config.floor;
        Vector3 size = layout.tank.size();
        double cellX = size.x / floor.cellsX;
        double cellY = size.y / floor.cellsY;

        std::vector<SpawnPoint> blocks = {};
        blocks.reserve(static_cast<size_t>(floor.cellsX * floor.cellsY));

        for (int ix = 0; ix < floor.cellsX; ix++)
        {
            for (int iy = 0; iy < floor.cellsY; iy++)
            {
                double x = layout.tank.min.x + (ix + 0.5) * cellX;
                double y = layout.tank.min.y + (iy + 0.5) * cellY;
                double height = floorHeight(config, layout, x, y);

                blocks.push_back({
                    "entities/cuberef.entity",
                    {x, y, height * 0.5},
                    {},
                    {cellX * 0.5, cellY * 0.5, height * 0.5}
                });
            }
        }

        for (const SpawnPoint &block : blocks)
        {
            EntitySpawner::spawnAt(simulation, block);
        }
    }
}