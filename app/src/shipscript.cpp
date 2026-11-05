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
#include <algorithm>
#include <cmath>

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
        
        
        // Roll Mechanics
        constexpr double maxRollAngle = 30.0 * (3.14159265358979323846 / 180.0); // Convert to radians
        constexpr double rollSpring = 7.0;
        constexpr double rollDamping = 7.0;
        const bool returning = std::abs(rollInput) < 0.01; // Check if roll input is near zero
        const double targetRollAngle = maxRollAngle * std::clamp(rollInput, -1.0, 1.0);
        const double spring = returning ? 2.0 : rollSpring; // Use a weaker spring when returning to center
        const double damping = returning ? 4.0 : rollDamping; // Use a weaker damping when returning to center

        const double rollAcceleration = (targetRollAngle - rollAngle) * spring - rollAngularVelocity * damping;
        rollAngularVelocity += rollAcceleration * dt;
        rollAngle += rollAngularVelocity * dt;
        
        
        Vector3 thrusterInput = {InputHandler::getAxis("thrustX"),
                                 InputHandler::getAxis("thrustY"),
                                 InputHandler::getAxis("thrustZ")};

        Vector3 angularThrusterInput = {0.0,
                                        InputHandler::getAxis("pitch"),
                                        InputHandler::getAxis("yaw")};                       


        Vector3 position = entity.getPosition();
        Quaternion rotation = attitude * Quaternion(Vector3::forward, rollAngle);

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
        angularVelocity.x = 0.0; // Disable roll for now

        attitude = attitude * Quaternion(Vector3::left, angularVelocity.y * dt)
                            * Quaternion(Vector3::up, angularVelocity.z * dt);
        attitude = Quaternion::normalize(attitude);

        rotation = attitude * Quaternion(Vector3::forward, rollAngle);

        entity.setPosition(position + velocity * dt);
        entity.setRotation(rotation);
    }

    void ShipScript::load(Entity &entity)
    {   
        attitude = entity.getRotation();
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
