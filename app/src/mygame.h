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
#include "engine/src/sim/entityspawner.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include <cstddef>
#include <cstdint>
#include <vector>

using Engine::Core::Application;
using Engine::Sim::Entity;
using Engine::Sim::SpawnConfig;
using Engine::Sim::SpawnShape;
using Engine::Sim::SpawnPoint;

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

      SpawnConfig fishConfig = {
          .count = 120,
          .entityPaths = {"entities/fish.entity"},
          .shape = SpawnShape::CylinderRing,
          .center = {0.0, 0.0, 0.0},
          .radiusMin = 0.0,
          .radiusMax = 400.0,
          .heightMin = -60.0,
          .heightMax = 60.0,
          .scaleMin = 0.2,
          .scaleMax = 3.0,
          .randomYaw = true,
          .randomPitchRoll = true,
          .snapToGround = false,
      };

      // SpawnPoints cubeConfig = {
      //   .entityPath = "entities/cuberef.entity",
      //   .position = {
      //            Vector3::forward * 25.0,
      //            Vector3::forward * -25.0,
      //            Vector3::left * 25.0,
      //            Vector3::left * -25.0,
      //            Vector3::up * 25.0,
      //            Vector3::up * -25.0
      //        },
      //   .rotation = {0.0,0.0,0.0},
      //   .scale = {1,1,1}
      // };

    };
}