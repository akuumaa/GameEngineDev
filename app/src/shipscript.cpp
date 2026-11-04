/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Tom Kertels, Alexander Mutig
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

#include <iostream>
#include <ostream>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;

namespace App
{
    void ShipScript::update(Entity &entity, double dt, double t)
    {
        double rollInput = InputHandler::getAxis("roll");
        double pitchInput = InputHandler::getAxis("pitch");
        double yawInput = InputHandler::getAxis("yaw");

        constexpr double thrust = 20.0;
        constexpr double angularThrust = 2.0;
        constexpr double maxVelocity = 20.0;
        constexpr double maxAngularVelocity = 2.0;
        constexpr double flightAssistRate = 1.5;
    

        Vector3 thrusterInput = {InputHandler::getAxis("thrustX"),
                                 InputHandler::getAxis("thrustY"),
                                 InputHandler::getAxis("thrustZ")};

        Vector3 angularThrusterInput = {InputHandler::getAxis("roll"),
                                        InputHandler::getAxis("pitch"),
                                        InputHandler::getAxis("yaw")};                       

        double pitchStep = pitchInput * dt * rotationSpeed;
        double yawStep   = yawInput * dt * rotationSpeed;
        double rollStep  = rollInput * dt * rotationSpeed;

        Vector3 position = entity.getPosition();
        Quaternion rotation = entity.getRotation();

        Vector3 acceleration = thrusterInput * thrust;
        Vector3 angularAcceleration = angularThrusterInput * angularThrust;

        if (flightAssist)
        {
            const Vector3 targetVelocity = rotation * (thrusterInput * maxVelocity);
            const Vector3 targetAngularVelocity = angularThrusterInput * maxAngularVelocity;

            acceleration = (targetVelocity - velocity) * flightAssistRate;
            angularAcceleration = (targetAngularVelocity - angularVelocity) * flightAssistRate;

            const double accelerationMagnitude = acceleration.length();
            if (accelerationMagnitude > thrust)
            {
                acceleration = acceleration * (thrust / accelerationMagnitude);
            }

            const double angularAccelerationMagnitude = angularAcceleration.length();
            if (angularAccelerationMagnitude > angularThrust)
            {
                angularAcceleration = angularAcceleration * (angularThrust / angularAccelerationMagnitude);
            }
        }

        velocity = velocity + acceleration * dt;
        angularVelocity = angularVelocity + angularAcceleration * dt;

        rotation = rotation
                * Quaternion(Vector3::forward, angularVelocity.x * dt)
                * Quaternion(Vector3::left, angularVelocity.y * dt)
                * Quaternion(Vector3::up, angularVelocity.z * dt);

        rotation = Quaternion::normalize(rotation);

        entity.setPosition(position + velocity * dt);
        entity.setRotation(rotation);
    }

    void ShipScript::load(Entity &entity)
    {   
        cameraCallbackId = InputHandler::onAction("nextCamera", [&entity]()
        {
            entity.nextCamera();
            std::cout << "+ Switch to camera " << std::endl;
        });

        flightAssistCallbackId = InputHandler::onAction("toggleFlightAssist", [this]()
        {
            flightAssist = !flightAssist;
            std::cout << "+ Flight Assist " << (flightAssist ? "ON" : "OFF") << std::endl;
        });
    }

    void ShipScript::unload(Entity &entity)
    {
        InputHandler::removeCallback(cameraCallbackId);
        InputHandler::removeCallback(flightAssistCallbackId);
    }
}
