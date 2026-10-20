/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
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
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        bool ok = Application::init();
        if (!ok) return false;

        Engine::Core::InputMapper::loadMappings("input.json");

        ship = simulation->loadEntity("entities/ship.entity");
        if (!ship) return false;

        InputHandler::onAction("reset", [this]()
        {
            ship->setRotation({});
        });

        ship->setPosition(Vector3(0.0, 0.0, 0.0));

        auto shipScript = std::make_unique<ShipScript>();
        ShipScript *shipScriptPtr = shipScript.get();
        ship->addScript(std::move(shipScript));

        InputHandler::onAction("rotationStart", [shipScriptPtr]()
        {
            shipScriptPtr->rotateWithMouse = true;
        });

        InputHandler::onActionRelease("rotationStart", [shipScriptPtr]()
        {
            shipScriptPtr->rotateWithMouse = false;
        });

        for (const Vector3 &position : {Vector3::forward * 25.0, Vector3::forward * -25.0, Vector3::left * 25.0, Vector3::left * -25.0, Vector3::up * 25.0, Vector3::up * -25.0})
        {
            if (auto cube = simulation->loadEntity("entities/cuberef.entity"))
            {
                cube->setPosition(position);
            }
        }

        std::chrono::duration<double> startupTime = std::chrono::steady_clock::now() - start;
        std::cout << "+ Startup time: " << startupTime.count() << " s" << std::endl;

        return true;
    }
}