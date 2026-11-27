/**
 * Copyright © 2026 Alexander Hancharenka
*/

#include "app/src/world/aquariumgenerator.h"
#include "engine/src/math/noise.h"
#include "engine/src/sim/entityspawner.h"
#include "engine/src/math/random.h"

//#include <numbers>
#include <algorithm>
#include <vector>

using Engine::Math::Noise;
using Engine::Sim::EntitySpawner;
//using Engine::Sim::SpawnPoint;
using Engine::Math::Random;

namespace App
{
    void AquariumGenerator::generate(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout)
    {
        buildFloorBlocks(simulation, config, layout);
    }

    std::vector<Vector3> AquariumGenerator::placeBubblers(const AquariumConfig &config, const AquariumLayout &layout)
    {
        const BubbleConfig &bubbles = config.bubbles;
        Random random(config.seed + 4);   // own seed layer for air stones

        std::vector<Vector3> positions = {};
        positions.reserve(static_cast<size_t>(bubbles.bubblerCount));

        for (int i = 0; i < bubbles.bubblerCount; i++)
        {
            double x = random.nextDouble(layout.tank.min.x + bubbles.margin, layout.tank.max.x - bubbles.margin);
            double y = random.nextDouble(layout.tank.min.y + bubbles.margin, layout.tank.max.y - bubbles.margin);
            positions.push_back({x, y, floorHeight(config, layout, x, y)});   // on the sand
        }
        return positions;
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
        /*
        const FloorConfig &floor = config.floor;
        Vector3 size = layout.tank.size();
        double cellX = size.x / floor.cellsX;
        double cellY = size.y / floor.cellsY;

        std::vector<SpawnPoint> blocks = {};
        blocks.reserve(static_cast<size_t>(floor.cellsX * floor.cellsY));

        Random random(config.seed + 3);

        for (int ix = 0; ix < floor.cellsX; ix++)
        {
            for (int iy = 0; iy < floor.cellsY; iy++)
            {
                // Cell center + small random offset → no visible grid
                double x = layout.tank.min.x + (ix + 0.5) * cellX + random.nextDouble(-0.3, 0.3) * cellX;
                double y = layout.tank.min.y + (iy + 0.5) * cellY + random.nextDouble(-0.3, 0.3) * cellY;
                double height = floorHeight(config, layout, x, y);

                // Slightly bigger than the cell → pebbles overlap, no gaps
                double size = random.nextDouble(0.6, 0.8);
                double yaw = random.nextDouble(0.0, 2.0 * std::numbers::pi);

                blocks.push_back({
                    floor.entity,
                    {x, y, height * 0.5},
                    Quaternion(yaw, 0.0, 0.0),
                    {cellX * size, cellY * size * 0.85, height * 0.9}
                });
            }
        }

        for (const SpawnPoint &block : blocks)
        {
            EntitySpawner::spawnAt(simulation, block);
        }*/

        Vector3 half = layout.tank.size() * 0.5;
        double height = config.tank.size.z * config.floor.baseRatio;

        EntitySpawner::spawnAt(simulation, {
            config.floor.entity,
            {0.0, 0.0, height},
            {},
            {half.x, half.y, 1.0}
        });
    }
}