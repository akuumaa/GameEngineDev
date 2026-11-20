/**
 * Copyright © 2026 Alexander Hancharenka
*/

#pragma once

#include "app/src/world/aquariumconfig.h"
#include "engine/src/sim/simulation.h"

using Engine::Sim::Simulation;

namespace App
{
    class AquariumGenerator
    {
      public:
        static void generate(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout);

        static double floorHeight(const AquariumConfig &config, const AquariumLayout &layout, double x, double y);

      private:
        static void buildFloorBlocks(Simulation &simulation, const AquariumConfig &config, const AquariumLayout &layout);
    };
}