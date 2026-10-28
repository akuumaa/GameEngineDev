/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Alexander Mutig
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

        bool ok = Application::init();
        if (!ok) return false;

        ship = simulation->loadEntity("entities/ship.entity");
        if (!ship) return false;

        ship->setPosition(Vector3(0.0, 0.0, 0.0));
        ship->addScript(std::make_unique<ShipScript>());

        for (const Vector3 &position : {Vector3::forward * 25.0, Vector3::forward * -25.0, Vector3::left * 25.0, Vector3::left * -25.0, Vector3::up * 25.0, Vector3::up * -25.0})
        {
            if (auto cube = simulation->loadEntity("entities/cuberef.entity"))
            {
                cube->setPosition(position);
            }
        }

        InputHandler::registerButtonAction("quit", GLFW_GAMEPAD_BUTTON_START);
        InputHandler::registerKeyAction("quit", GLFW_KEY_ESCAPE);

        InputHandler::registerKeyAction("reset", GLFW_KEY_SPACE);
        InputHandler::registerKeyAction("nextCamera", GLFW_KEY_C);
        InputHandler::registerButtonAction("reset", GLFW_GAMEPAD_BUTTON_A);
        InputHandler::registerButtonAction("nextCamera", GLFW_GAMEPAD_BUTTON_BACK);

        InputHandler::registerKeyAxis("yaw", GLFW_KEY_Q, GLFW_KEY_E);
        InputHandler::registerKeyAxis("pitch", GLFW_KEY_S, GLFW_KEY_W);
        InputHandler::registerKeyAxis("pitch", GLFW_KEY_DOWN, GLFW_KEY_UP);
        InputHandler::registerKeyAxis("roll", GLFW_KEY_A, GLFW_KEY_D);
        InputHandler::registerKeyAxis("roll", GLFW_KEY_LEFT, GLFW_KEY_RIGHT);

        // Camera controls
        InputHandler::registerKeyAxis("cameraYaw", GLFW_KEY_L, GLFW_KEY_J);
        InputHandler::registerKeyAxis("cameraPitch", GLFW_KEY_I, GLFW_KEY_K);
        InputHandler::registerKeyAxis("cameraZoom", GLFW_KEY_U, GLFW_KEY_O);

        InputHandler::registerStickAxis("cameraYaw", GLFW_GAMEPAD_AXIS_RIGHT_X, true);
        InputHandler::registerStickAxis("cameraPitch", GLFW_GAMEPAD_AXIS_RIGHT_Y, true);
        InputHandler::registerStickAxis("pitch", GLFW_GAMEPAD_AXIS_RIGHT_Y, true);
        InputHandler::registerGamepadAxis("roll", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
        InputHandler::registerTriggerAxis("roll", GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

        InputHandler::onAction("reset", [this]()
        {
            ship->setRotation({});
        });

        InputHandler::onAction("nextCamera", [this]()
        {
            ship->nextCamera();
        });

        ship->setFocus(true);

        std::chrono::duration<double> startupTime = std::chrono::steady_clock::now() - start;
        std::cout << "+ Startup time: " << startupTime.count() << " s" << std::endl;

        return true;
    }
}