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

#include "app/src/shipscript.h"

#include "engine/src/core/inputhandler.h"
#include "engine/src/math/quaternion.h"
#include "engine/src/sim/entity.h"

#include <iomanip>
#include <iostream>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;

namespace App
{

    //! Ship Movement and Physics
    void ShipScript::update(Entity &entity, double dt, double t)
    {
        //! Movement Settings
        constexpr double thrust = 20.0;
        constexpr double angularThrust = 2.0;
        constexpr double maxVelocity = 20.0;
        constexpr double maxAngularVelocity = 2.0;
        constexpr double flightAssistRate = 1.5;

        //! Input
        Vector3 thrusterInput = {
            InputHandler::getAxis("thrustX"),
            InputHandler::getAxis("thrustY"),
            InputHandler::getAxis("thrustZ")
        };
        Vector3 angularThrusterInput = {
            InputHandler::getAxis("roll"),
            InputHandler::getAxis("pitch"),
            InputHandler::getAxis("yaw")
        };

        //! Current Transform
        Vector3 position = entity.getPosition();
        Quaternion rotation = entity.getRotation();

        //! Water Physics
        // Update ballast tank water level
        double ballastInput = InputHandler::getAxis("ballast");

        waterPhysics.changeWaterLevel(ballastInput, dt);

        // Calculate vertical movement using buoyancy and water resistance
        double verticalInput = InputHandler::getAxis("thrustZ");

        double verticalMovement =
            waterPhysics.updateVertical(verticalInput, dt);

        // Vertical movement is handled exclusively by WaterPhysics
        thrusterInput.z = 0.0;

        //! Movement Acceleration
        Vector3 acceleration = thrusterInput * thrust;
        Vector3 angularAcceleration = angularThrusterInput * angularThrust;

        //! Flight Assist
        if (flightAssist)
        {
            Vector3 targetVelocity = rotation * (thrusterInput * maxVelocity);
            Vector3 targetAngularVelocity = angularThrusterInput * maxAngularVelocity;

            // Beschleunigung im Weltkoordinatensystem berechnen
            Vector3 worldAcceleration =
                (targetVelocity - velocity) * flightAssistRate;

            // In das lokale Koordinatensystem umrechnen
            Quaternion inverseRotation = -rotation;
            acceleration = inverseRotation * worldAcceleration;

            angularAcceleration =
                (targetAngularVelocity - angularVelocity) * flightAssistRate;

            // Beschleunigung begrenzen
            if (acceleration.length() > thrust)
            {
                acceleration = acceleration * (thrust / acceleration.length());
            }

            // Winkelbeschleunigung begrenzen
            if (angularAcceleration.length() > angularThrust)
            {
                angularAcceleration = angularAcceleration *
                                      (angularThrust / angularAcceleration.length());
            }
        }

        //! Velocity Update
        // Apply translational acceleration
        velocity = velocity + rotation * acceleration * dt;

        // Apply roational acceleration
        angularVelocity = angularVelocity + angularAcceleration * dt;

        //! Rotation Update
        rotation = rotation * Quaternion(Vector3::forward, angularVelocity.x * dt) * Quaternion(Vector3::left, angularVelocity.y * dt) * Quaternion(Vector3::up, angularVelocity.z * dt);

        // Keep quaternion normalized
        rotation = Quaternion::normalize(rotation);

        //! Position Update
        Vector3 newPosition =
            position + velocity * dt + Vector3::up * verticalMovement;

        // Apply Calculated transform
        entity.setPosition(newPosition);
        entity.setRotation(rotation);

        //! Physics Debug Output
        // Debug-Ausgabe alle 0.25 Sekunden
        physicsDebugTimer += dt;

        if (physicsDebugTimer >= 0.25)
        {
            physicsDebugTimer = 0.0;

            double water = waterPhysics.getWaterLevel();
            double speed = waterPhysics.getVerticalVelocity();

            const char *movement = "Schwebt";

            if (speed > 0.05)
                movement = "Steigt";
            else if (speed < -0.05)
                movement = "Sinkt";

            std::cout << std::fixed << std::setprecision(2)
                      << "[Wasserphysik] "
                      << "Ballast: " << water << "% | "
                      << "Vertikal: " << speed << " m/s | "
                      << "Status: " << movement
                      << std::endl;
        }
    }

    //! Script Initialization
    void ShipScript::load(Entity &entity)
    {
        // Register camera switchting callback
        cameraCallbackId = InputHandler::onAction(
            "nextCamera",
            [&entity]()
        {
            entity.nextCamera();
            std::cout << "+ Switched Camera" << std::endl;
        }
        );

        // Register flight assist toggle callback
        flightAssistCallbackId = InputHandler::onAction(
            "toggleFlightAssist", 
            [this]()
        {
            flightAssist = !flightAssist;
            std::cout 
            << "+ Flight Assist: " 
            << (flightAssist ? "ON" : "OFF") 
            << std::endl;
        });
    }

    //! Script Cleanup

    void ShipScript::unload(Entity &entity)
    {
        InputHandler::removeCallback(cameraCallbackId);
        InputHandler::removeCallback(flightAssistCallbackId);
    }

}
