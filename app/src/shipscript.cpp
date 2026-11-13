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
#include "engine/src/math/vector3.h"

#include <iomanip>
#include <iostream>
#include <cmath>
#include <algorithm>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;
using Engine::Math::Vector3;

namespace App
{

    //! Ship Movement and Physics
    void ShipScript::update(Entity &entity, double dt, double /*t*/)
    {
        //! Movement Settings
        constexpr double thrust = 20.0;
        constexpr double angularThrust = 2.0;
        constexpr double maxVelocity = 20.0;
        constexpr double maxAngularVelocity = 2.0;
        constexpr double flightAssistRate = 1.5;

        updateRoll(InputHandler::getAxis("roll"), dt);

        //! Input
        Vector3 thrusterInput = {
            InputHandler::getAxis("thrustX"),
            InputHandler::getAxis("thrustY"),
            InputHandler::getAxis("thrustZ")
        };
        Vector3 angularThrusterInput = {
            0.0, // Roll is handled separately,
            InputHandler::getAxis("pitch"),
            InputHandler::getAxis("yaw")
        };

        //! Current Transform
        Vector3 position = entity.getPosition();
        Quaternion rotation = attitude * Quaternion(Vector3::forward, rollAngle);
        rotation = Quaternion::normalize(rotation);

        double verticalMovement = 0.0;
        double forwardMovement = 0.0;
        updateWaterMovement(
            thrusterInput,
            dt,
            verticalMovement,
            forwardMovement
        );

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
        velocity = velocity + rotation * acceleration * dt;
        angularVelocity = angularVelocity + angularAcceleration * dt;

        //! Rotation Update
        attitude = attitude
         * Quaternion(Vector3::left, angularVelocity.y * dt)
         * Quaternion(Vector3::up, angularVelocity.z * dt);

        attitude = Quaternion::normalize(attitude);

        rotation = attitude * Quaternion(Vector3::forward, rollAngle);
        rotation = Quaternion::normalize(rotation);

        //! Position Update
        Vector3 newPosition =
            position + velocity * dt
            + Vector3::up * verticalMovement
            + rotation * Vector3::forward * forwardMovement;

        entity.setPosition(newPosition);
        entity.setRotation(rotation);

        //! Physics Debug Output
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
                      << "Forward: " << waterPhysics.getForwardVelocity() << " m/s | "
                      << "Pos: " << entity.getPosition().x << ", " << entity.getPosition().z << " | "
                      << "Status: " << movement
                      << std::endl;
        }
    }

    void ShipScript::updateRoll(double input, double dt)
    {
        constexpr double maxRollAngle =
            30.0 * (3.14159265358979323846 / 180.0);
        constexpr double rollSpring = 7.0;
        constexpr double rollDamping = 7.0;

        const bool isReturning = std::abs(input) < 0.01;
        const double targetRollAngle =
            maxRollAngle * std::clamp(input, -1.0, 1.0);
        const double spring = isReturning ? 3.0 : rollSpring;
        const double damping = isReturning ? 4.0 : rollDamping;

        const double rollAcceleration =
            (targetRollAngle - rollAngle) * spring
            - rollAngularVelocity * damping;

        rollAngularVelocity += rollAcceleration * dt;
        rollAngle += rollAngularVelocity * dt;
    }

    void ShipScript::updateWaterMovement(
        Vector3 &thrusterInput,
        double dt,
        double &verticalMovement,
        double &forwardMovement
    )
    {
        waterPhysics.changeWaterLevel(
            InputHandler::getAxis("ballast"),
            dt
        );

        verticalMovement =
            waterPhysics.updateVertical(thrusterInput.z, dt);

        forwardMovement =
            waterPhysics.updateForward(thrusterInput.x, dt);

        // These axes are now handled by WaterPhysics.
        thrusterInput.x = 0.0;
        thrusterInput.z = 0.0;
    }

    //! Script Initialization
    void ShipScript::load(Entity &entity)
    {
        attitude = entity.getRotation();

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
