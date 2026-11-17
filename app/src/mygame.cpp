
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
#include "engine/src/math/quaternion.h"
#include "engine/src/math/random.h"
#include "engine/src/math/vector3.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <numbers>

using Engine::Core::InputHandler;
using Engine::Math::Vector3;
using Engine::Math::Quaternion;
using Engine::Math::Random;

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

        renderer->loadSkybox("models/skybox_placeholder.model");
        renderer->loadSun("models/sun.model", "models/lensflare.model");

        Random random(worldSeed);

        for (int i = 0; i < asteroidCount; i++)
        {
            double angle = random.nextDouble(0.0, 2.0 * std::numbers::pi);
            double radius = random.nextDouble(fieldRadiusMin, fieldRadiusMax);
            double height = random.nextDouble(-fieldHeight, fieldHeight);
            double size = random.nextDouble(asteroidSizeMin, asteroidSizeMax);
            double yaw = random.nextDouble(0.0, 2.0 * std::numbers::pi);
            double pitch = random.nextDouble(0.0, 2.0 * std::numbers::pi);
            double roll = random.nextDouble(0.0, 2.0 * std::numbers::pi);

            if (auto asteroid = simulation->loadEntity("entities/asteroid_placeholder.entity"))
            {
                asteroid->setPosition(Vector3(std::cos(angle) * radius, std::sin(angle) * radius, height));
                asteroid->setRotation(Quaternion(yaw, pitch, roll));
                asteroid->setScale(Vector3(size, size, size));

                asteroids.push_back(asteroid->getEntityId());
            }
        }

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
                ship->queueDestroy();
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

            size_t nearest = 0;

            for (size_t asteroidId : asteroids)
            {
                auto asteroid = simulation->getEntity(asteroidId);
                if (!asteroid) continue;

                if (nearest == 0)
                {
                    nearest = asteroidId;
                    continue;
                }

                auto nearestAsteroid = simulation->getEntity(nearest);
                if (!nearestAsteroid) continue;

                double distanceToNearest = (ship->getPosition() - nearestAsteroid->getPosition()).length();
                double distanceToCurrent = (ship->getPosition() - asteroid->getPosition()).length();

                if (distanceToCurrent < distanceToNearest)
                {
                    nearest = asteroidId;
                }
            }

            if (nearest == 0)
            {
                return;
            }

            simulation->getEntity(nearest)->queueDestroy();

            std::cout << "  Unloaded asteroid " << nearest << ", " << simulation->getEntityCount() << " entities left" << std::endl;
        });

        return true;
    }


    //! Spawn Ship
    bool MyGame::spawnShip()
    {
        ship = simulation->loadEntity("entities/ship.entity");
        if (!ship) return false;

        // Set initial position
        ship->setPosition(Vector3(0.0, 0.0, 0.0));

        // Attach movement and physics script to the ship entity
        ship->addScript(std::make_unique<ShipScript>());

        // Set ship as the active focus entity
        ship->setFocus(true);
        return true;
    }
}