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

#include "app/src/submarine/shipscript.h"

#include "engine/src/core/inputhandler.h"
#include "engine/src/math/quaternion.h"
#include "engine/src/math/vector3.h"
#include "engine/src/sim/entity.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;
using Engine::Math::Vector3;

namespace App
{
    ShipScript::ShipScript(
        std::shared_ptr<const PhysicsWorld> world,
        const SubmarineConfig &config
    )
        : RigidBodyScript(std::move(world), config.body),
          config(config),
          ballast(config.ballast),
          verticalRamp(config.verticalRampTime)
    {
    }

    //! Physics Hooks

    void ShipScript::beforePhysics(Entity &entity, double dt, double t)
    {
        applyBallast(dt);
        applyWaterThrust(entity, dt);
        applyManeuvering(entity);
        applyRoll(entity);
    }

    //! Ballast: water level → buoyancy factor
    void ShipScript::applyBallast(double dt)
    {
        ballast.changeLevel(InputHandler::getAxis("ballast"), dt);
        getBody().getProperties().buoyancyFactor = ballast.updateBuoyancy(dt);
    }

    //! Forward + Vertical Movement (was WaterPhysics::updateForward / updateVertical)
    void ShipScript::applyWaterThrust(Entity &entity, double dt)
    {
        const Quaternion rotation = entity.getRotation();

        const double forwardInput = std::clamp(InputHandler::getAxis("thrustX"), -1.0, 1.0);
        const double verticalInput = std::clamp(InputHandler::getAxis("thrustZ"), -1.0, 1.0);

        // Forward thrust follows the nose
        getBody().addLocalForce(
            Vector3::forward * calculateForwardThrust(forwardInput),
            rotation
        );

        // Vertical thrust follows world up (like before), with input ramp
        getBody().addForce(
            Vector3::up * (verticalRamp.update(verticalInput, dt) * config.verticalThrust)
        );
    }

    //! Lateral + Pitch/Yaw (was the flight assist part of update)
    void ShipScript::applyManeuvering(Entity &entity)
    {
        const Quaternion rotation = entity.getRotation();

        const double lateralInput = InputHandler::getAxis("thrustY");
        const double pitchInput = InputHandler::getAxis("pitch");
        const double yawInput = InputHandler::getAxis("yaw");

        double lateralAcceleration = lateralInput * config.thrust;
        Vector3 angularAcceleration = {
            0.0, // Roll is handled separately
            pitchInput * config.pitchThrust,
            yawInput * config.angularThrust
        };

        //! Flight Assist
        // Only lateral and rotation, so it does not brake forward/vertical movement
        if (flightAssist)
        {
            const Vector3 localVelocity = getBody().getLocalVelocity(rotation);
            const Vector3 localAngularVelocity = getBody().getLocalAngularVelocity(rotation);

            const double targetLateral = lateralInput * config.maxVelocity;

            lateralAcceleration = std::clamp(
                (targetLateral - localVelocity.y) * config.flightAssistRate,
                -config.thrust,
                config.thrust
            );

            angularAcceleration = {
                0.0,
                (pitchInput * config.maxAngularVelocity - localAngularVelocity.y) * config.flightAssistRate,
                (yawInput * config.maxAngularVelocity - localAngularVelocity.z) * config.flightAssistRate
            };

            // Limit angular acceleration
            if (angularAcceleration.length() > config.angularThrust)
            {
                angularAcceleration = angularAcceleration *
                                      (config.angularThrust / angularAcceleration.length());
            }
        }

        getBody().addLocalForce(Vector3::left * lateralAcceleration, rotation);
        getBody().addLocalAngularAcceleration(angularAcceleration, rotation);
    }

    //! Roll Spring (same values as before, now as real rotation)
    void ShipScript::applyRoll(Entity &entity)
    {
        const Quaternion rotation = entity.getRotation();
        const double rollInput = InputHandler::getAxis("roll");

        const bool isReturning = std::abs(rollInput) < 0.01;
        const double targetRollAngle = config.maxRollAngle * std::clamp(rollInput, -1.0, 1.0);
        const double spring = isReturning ? config.rollReturnSpring : config.rollSpring;
        const double damping = isReturning ? config.rollReturnDamping : config.rollDamping;

        // Current roll angle: how far the left side points up
        const Vector3 bodyLeft = rotation * Vector3::left;
        const double rollAngle = std::asin(std::clamp(bodyLeft.z, -1.0, 1.0));
        const double rollAngularVelocity = getBody().getLocalAngularVelocity(rotation).x;

        const double rollAcceleration =
            (targetRollAngle - rollAngle) * spring - rollAngularVelocity * damping;

        getBody().addLocalAngularAcceleration(Vector3::forward * rollAcceleration, rotation);
    }

    double ShipScript::calculateForwardThrust(double input) const
    {
        // Reverse thrust is weaker than forward thrust
        return input * (input >= 0.0 ? config.forwardThrust : config.reverseThrust);
    }

    //! Physics Debug Output
    // Debug-Ausgabe alle 0.25 Sekunden
    void ShipScript::afterPhysics(Entity &entity, double dt, double t)
    {
        physicsDebugTimer += dt;

        if (physicsDebugTimer < 0.25) return;
        physicsDebugTimer = 0.0;

        const double water = ballast.getLevel();
        const double speed = getBody().getVelocity().z;
        const double forward = getBody().getLocalVelocity(entity.getRotation()).x;

        const char *movement = "Schwebt";

        if (speed > 0.05)
            movement = "Steigt";
        else if (speed < -0.05)
            movement = "Sinkt";

        std::cout << std::fixed << std::setprecision(2)
                  << "[Wasserphysik] "
                  << "Ballast: " << water << "% | "
                  << "Vertikal: " << speed << " m/s | "
                  << "Forward: " << forward << " m/s | "
                  << "Pos: " << entity.getPosition().x << ", " << entity.getPosition().z << " | "
                  << "Status: " << movement
                  << std::endl;
    }

    //! Script Initialization
    void ShipScript::load(Entity &entity)
    {
        RigidBodyScript::load(entity);

        // Register camera switching callback
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
        }
        );

        // Register ballast reset callback
        ballastResetCallbackId = InputHandler::onAction(
            "resetBallast",
            [this]()
        {
            ballast.reset();
            std::cout << "+ Ballast reset to 50%" << std::endl;
        }
        );
    }

    //! Script Cleanup
    void ShipScript::unload(Entity &entity)
    {
        InputHandler::removeCallback(cameraCallbackId);
        InputHandler::removeCallback(flightAssistCallbackId);
        InputHandler::removeCallback(ballastResetCallbackId);

        RigidBodyScript::unload(entity);
    }
}