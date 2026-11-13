/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Tom Kertels
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

#include "engine/src/core/application.h"
#include "engine/src/sim/entity.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>


using Engine::Core::Application;
using Engine::Sim::Entity;

namespace App
{
    class MyGame : public Application
    {
      public:
        MyGame(int argc, char *argv[]);
        bool init();

      private:
      bool spawnShip();
      std::shared_ptr<Entity> ship = {};
      std::vector<size_t> asteroids = {};

      uint32_t worldSeed = 1337;
      int asteroidCount = 10;
      double fieldRadiusMin = 80.0;
      double fieldRadiusMax = 400.0;
      double fieldHeight = 60.0;
      double asteroidSizeMin = 2.0;
      double asteroidSizeMax = 12.0;
    };
}