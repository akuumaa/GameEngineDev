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
#include "engine/src/core/inputhandler.h"
#include "engine/src/math/vector3.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>

using Engine::Core::InputHandler;

using Engine::Math::Vector3;

namespace App
{
    MyGame::MyGame(int argc, char *argv[])
        : Application(argc, argv)
    {
        settings.title = "My Game";
    }

    bool MyGame::init()
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        InputHandler::registerKeyAxis("yaw", GLFW_KEY_E, GLFW_KEY_Q);
        InputHandler::registerKeyAxis("pitch", GLFW_KEY_DOWN, GLFW_KEY_UP);
        InputHandler::registerKeyAxis("roll", GLFW_KEY_LEFT, GLFW_KEY_RIGHT);

        InputHandler::registerKeyAction("nextCamera", GLFW_KEY_C);

        InputHandler::registerKeyAxis("cameraYaw", GLFW_KEY_L, GLFW_KEY_J);
        InputHandler::registerKeyAxis("cameraPitch", GLFW_KEY_I, GLFW_KEY_K);
        InputHandler::registerKeyAxis("cameraZoom", GLFW_KEY_U, GLFW_KEY_O);

        InputHandler::registerButtonAction("nextCamera", GLFW_GAMEPAD_BUTTON_BACK);
        InputHandler::registerStickAxis("cameraYaw", GLFW_GAMEPAD_AXIS_RIGHT_X, true);
        InputHandler::registerStickAxis("cameraPitch", GLFW_GAMEPAD_AXIS_RIGHT_Y, true);

        InputHandler::registerStickAxis("pitch", GLFW_GAMEPAD_AXIS_LEFT_Y, true);
        InputHandler::registerStickAxis("roll", GLFW_GAMEPAD_AXIS_LEFT_X);

        InputHandler::registerKeyAxis("thrustX", GLFW_KEY_S, GLFW_KEY_W);
        InputHandler::registerKeyAxis("thrustY", GLFW_KEY_D, GLFW_KEY_A);
        InputHandler::registerKeyAxis("thrustZ", GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE);

        InputHandler::registerKeyAction("respawn", GLFW_KEY_R);
        InputHandler::registerButtonAction("respawn", GLFW_GAMEPAD_BUTTON_A);

        InputHandler::registerKeyAction("toggleFlightAssist", GLFW_KEY_TAB);
        InputHandler::registerButtonAction("toggleFlightAssist", GLFW_GAMEPAD_BUTTON_X);

        bool ok = Application::init();
        if (!ok) return false;

        for (const Vector3 &position : {Vector3::forward * 25.0, Vector3::forward * -25.0, Vector3::left * 25.0, Vector3::left * -25.0, Vector3::up * 25.0, Vector3::up * -25.0})
        {
            if (auto cube = simulation->loadEntity("entities/cuberef.entity"))
            {
                cube->setPosition(position);
            }
        }

        std::chrono::duration<double> startupTime = std::chrono::steady_clock::now() - start;
        std::cout << "+ Startup time: " << startupTime.count() << " s" << std::endl;

        ok = spawnShip();
        if (!ok) return false;

        InputHandler::onAction("respawn", [this]()
        {
            if (ship) simulation->unloadEntity(ship->getEntityId());
            spawnShip();
        });

        return true;
    }

    bool MyGame::spawnShip()
    {
        ship = simulation->loadEntity("entities/ship.entity");
        if (!ship) return false;

        ship->setPosition(Vector3(0.0, 0.0, 0.0));
        ship->addScript(std::make_unique<ShipScript>());

        ship->setFocus(true);
        return true;
    }
}