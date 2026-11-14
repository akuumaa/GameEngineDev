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
#include "engine/src/math/quaternion.h"
#include "engine/src/math/random.h"
#include "engine/src/math/vector3.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numbers>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;
using Engine::Math::Random;
using Engine::Math::Vector3;
namespace App
{
    MyGame::MyGame(int argc, char *argv[])
        : Application(argc, argv)
    {
        settings.title = "Low Poly Citzien";
        settings.vsync = false;
    }

    bool MyGame::init()
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        bool ok = Application::init();
        if (!ok) return false;
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

        ok = spawnShip();
        if (!ok) return false;

        // SHIP ACTION REGISTRATION
        InputHandler::registerKeyAxis("roll", GLFW_KEY_Q, GLFW_KEY_E);
        InputHandler::registerKeyAction("nextCamera", GLFW_KEY_C);
        InputHandler::registerKeyAxis("thrustX", GLFW_KEY_S, GLFW_KEY_W);
        InputHandler::registerKeyAxis("thrustY", GLFW_KEY_D, GLFW_KEY_A);
        InputHandler::registerKeyAxis("thrustZ", GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE);
        InputHandler::registerKeyAction("respawn", GLFW_KEY_R);
        InputHandler::registerKeyAction("toggleFlightAssist", GLFW_KEY_TAB);
        InputHandler::registerKeyAction("unloadAsteroid", GLFW_KEY_X);

        InputHandler::registerStickAxis("pitch", GLFW_GAMEPAD_AXIS_LEFT_Y, true);
        InputHandler::registerStickAxis("roll", GLFW_GAMEPAD_AXIS_LEFT_X);
        InputHandler::registerButtonAction("nextCamera", GLFW_GAMEPAD_BUTTON_BACK);
        InputHandler::registerStickAxis("cameraYaw", GLFW_GAMEPAD_AXIS_RIGHT_X, true);
        InputHandler::registerStickAxis("cameraPitch", GLFW_GAMEPAD_AXIS_RIGHT_Y, true);
        InputHandler::registerButtonAction("respawn", GLFW_GAMEPAD_BUTTON_A);
        InputHandler::registerTriggerAxis("thrustX", GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);
        InputHandler::registerButtonAction("toggleFlightAssist", GLFW_GAMEPAD_BUTTON_X);
        InputHandler::registerButtonAction("unloadAsteroid", GLFW_GAMEPAD_BUTTON_B);

        InputHandler::registerKeyAxis("cameraYaw", GLFW_KEY_L, GLFW_KEY_J);
        InputHandler::registerKeyAxis("cameraPitch", GLFW_KEY_I, GLFW_KEY_K);
        InputHandler::registerKeyAxis("cameraZoom", GLFW_KEY_U, GLFW_KEY_O);

        InputHandler::onAction("respawn", [this]()
        {
            if (ship) simulation->unloadEntity(ship->getEntityId());
            spawnShip();
        });

        InputHandler::onAction("unloadAsteroid", [this]()
        {
            if (!ship) return;

            size_t nearest = 0;

            // TODO: Über asteroids iterieren. Die ID des Asteroiden mit dem kleinsten
            //       Abstand zum Schiff in nearest merken. IDs, zu denen es keine
            //       Entity mehr gibt, überspringen.

            // TODO: Gibt es keinen Asteroiden mehr, ist hier Schluss.
            //       Sonst den Asteroiden mit der ID nearest entfernen,
            //       so wie das Schiff beim Respawn.

            std::cout << "  Unloaded asteroid " << nearest << ", " << simulation->getEntityCount() << " entities left" << std::endl;
        });

        std::chrono::duration<double> startupTime = std::chrono::steady_clock::now() - start;
        std::cout << "+ Startup time: " << startupTime.count() << " s" << std::endl;

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