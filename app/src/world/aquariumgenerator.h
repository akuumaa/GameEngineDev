/**
 * Copyright © 2026 Alexander Hancharenka
*/

#pragma once

#include "app/src/world/aquariumconfig.h"
#include "engine/src/sim/simulation.h"
#include "engine/src/math/vector3.h"

#include <vector>

using Engine::Sim::Simulation;

namespace App
{
    class AquariumGenerator
    {
      public:
        static void generate(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout);

        static double floorHeight(const AquariumConfig &config, const AquariumLayout &layout, double x, double y);

        /// Air stone positions on the sand. Same seed → same positions.
        static std::vector<Vector3> placeBubblers(const AquariumConfig &config, const AquariumLayout &layout);

      private:
        static void buildFloorBlocks(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout);

        static void writeFloorMesh(const AquariumConfig &config, const AquariumLayout &layout, const std::string &path);
    };
}