
/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Stefan Ganswint
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

#include "app/src/mygame.h"
#include "app/src/shipscript.h"
#include "engine/src/core/inputMapper.h"
#include "engine/src/core/inputhandler.h"
#include "engine/src/math/vector3.h"
#include "engine/src/sim/entityspawner.h"
#include "app/src/world/aquariumgenerator.h"

#include <chrono>
#include <iostream>
#include <memory>

using Engine::Core::InputHandler;
using Engine::Math::Vector3;
using Engine::Sim::EntitySpawner;

namespace App
{
    MyGame::MyGame(int argc, char *argv[])
        : Application(argc, argv)
    {
        settings.title = "Awesome Game";
    }

    bool MyGame::init()
    {
        std::chrono::steady_clock::time_point start =
            std::chrono::steady_clock::now();

        // Initialize engine systems
        bool ok = Application::init();
        if (!ok)
        {
            return false;
        }

        // Load all keyboard and gamepad mappings
        if (!Engine::Core::InputMapper::loadMappings("input.json"))
        {
            std::cerr << "! Failed to load input mappings"
                      << std::endl;
            return false;
        }

        renderer->loadSkybox("models/skybox.model");
        renderer->loadSun("models/sun.model", "models/lensflare.model");

        // aquarium config load
        layout = AquariumLayout::from(aquariumConfig);
        AquariumGenerator::generate(*simulation, aquariumConfig, layout);

        // fish inside layout 
        double waterHalfHeight = (layout.surfaceHeight - layout.floorTop) * 0.5;
        fishConfig.shape      = SpawnShape::Box;
        fishConfig.center     = {0.0, 0.0, layout.floorTop + waterHalfHeight};
        fishConfig.boxExtentX = layout.tank.size().x * 0.5 - 10.0;
        fishConfig.boxExtentY = layout.tank.size().y * 0.5 - 10.0;
        fishConfig.heightMin  = -waterHalfHeight;
        fishConfig.heightMax  =  waterHalfHeight;

        asteroids = EntitySpawner::spawnProcedural(*simulation, fishConfig);

        std::chrono::duration<double> startupTime =
            std::chrono::steady_clock::now() - start;

        std::cout
            << "+ Startup time: "
            << startupTime.count()
            << " s"
            << std::endl;

        //! Spawn Ship
        ok = spawnShip();
        if (!ok) return false;

        //! Respawn Callback
        // Remvoe the exsisting ship and create a new instance
        // when the respawn action is triggered
        InputHandler::onAction("respawn", [this]()
        {
            if (ship)
            {
                ship->queueUnload();
                ship = nullptr;
            }

            if (!spawnShip())
            {
                std::cerr << "! Failed to respawn ship"
                          << std::endl;
            }
        });

        InputHandler::onAction("unloadAsteroid", [this]()
        {
            if (!ship) return;

            std::shared_ptr<Entity> nearest = nullptr;
            double nearestDistance = 0.0;

            for (const std::shared_ptr<Entity> &asteroid : simulation->findEntitiesWithTag("asteroid"))
            {
                double distance = (ship->getPosition() - asteroid->getPosition()).length();

                if (!nearest || distance < nearestDistance)
                {
                    nearest = asteroid;
                    nearestDistance = distance;
                }
            }

            if (!nearest) return;

            nearest->queueUnload();

            std::cout << "  Queued asteroid " << nearest->getEntityId() << " for removal" << std::endl;
        });

        return true;
    }

    //! Spawn Ship
    bool MyGame::spawnShip()
    {
        ship = simulation->loadEntity("entities/ship2.entity");
        if (!ship) return false;

        // Set initial position
        //ship->setPosition(Vector3(0.0, 0.0, 0.0));
        ship->setPosition(layout.shipSpawn);

        // Attach movement and physics script to the ship entity
        ship->addScript(std::make_unique<ShipScript>());

        // Set ship as the active focus entity
        ship->setFocus(true);
        return true;
    }
}