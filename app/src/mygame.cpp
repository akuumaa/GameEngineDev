
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

#include <chrono>
#include <iostream>
#include <memory>

using Engine::Core::InputHandler;
using Engine::Math::Vector3;

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

        // World Setup
        for (const Vector3 &position : {
            Vector3::forward * 25.0,
            Vector3::forward * -25.0,
            Vector3::left * 25.0,
            Vector3::left * -25.0,
            Vector3::up * 25.0,
            Vector3::up * -25.0
        })
        {
            if (auto cube = simulation->loadEntity("entities/cuberef.entity"))
            {
                cube->setPosition(position);
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
                simulation->unloadEntity(ship->getEntityId());
                ship = nullptr;
            }

            if (!spawnShip())
            {
                std::cerr << "! Failed to respawn ship"
                          << std::endl;
            }
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