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

#pragma once

#include "app/src/submarine/ballasttank.h"
#include "app/src/submarine/inputramp.h"
#include "engine/src/math/vector3.h"
#include "engine/src/physics/rigidbodyscript.h"

#include <cstddef>
#include <memory>

using Engine::Math::Vector3;
using Engine::Physics::BodyProperties;
using Engine::Physics::PhysicsWorld;
using Engine::Physics::RigidBodyScript;
using Engine::Sim::Entity;

namespace App
{
    //! Submarine Body (values from the old WaterPhysicsConfig)
    inline BodyProperties submarineBody()
    {
        BodyProperties body;
        body.mass = 1.0;
        body.linearDrag = {0.1, 0.0, 0.25};     // x: forwardLinearDrag, y: none (like before), z: linearDrag
        body.quadraticDrag = {0.04, 0.0, 0.08}; // x: forwardQuadraticDrag, z: quadraticDrag
        body.angularDrag = {0.0, 0.0, 0.0};     // like before: rotation is only damped by flight assist
        body.rightingStrength = 0.0;            // like before: no self-righting
        body.maxLocalSpeed = {0.0, 0.0, 5.0};   // was maxVerticalSpeed
        return body;
    }

    //! Submarine Settings (values from the old ShipScript + WaterPhysicsConfig)
    struct SubmarineConfig
    {
        //! Maneuvering (lateral + rotation)
        double thrust = 20.0;
        double angularThrust = 2.0;
        double maxVelocity = 20.0;
        double maxAngularVelocity = 2.0;
        double flightAssistRate = 1.5;

        //! Roll Spring
        double maxRollAngle = 30.0 * (3.14159265358979323846 / 180.0);
        double rollSpring = 7.0;
        double rollDamping = 7.0;
        double rollReturnSpring = 3.0; // used when roll input is released
        double rollReturnDamping = 4.0;

        //! Water Movement
        double forwardThrust = 6.96;
        double reverseThrust = 3.0;
        double verticalThrust = 3.0;   // was verticalAcceleration
        double verticalRampTime = 2.0; // was accelerationRampTime

        //! Ballast & Body
        BallastSettings ballast = {};
        BodyProperties body = submarineBody();
    };

    /**
     * Ship Script - only controls the submarine.
     * Velocity, water and integration come from RigidBodyScript + PhysicsWorld,
     * the ship only adds its own forces.
     */
    class ShipScript : public RigidBodyScript
    {
      public:
        explicit ShipScript(
            std::shared_ptr<const PhysicsWorld> world,
            const SubmarineConfig &config = {}
        );

        void load(Entity &entity) override;
        void unload(Entity &entity) override;

      protected:
        //! Physics Hooks
        // Input → forces (per sub-step)
        void beforePhysics(Entity &entity, double dt, double t) override;

        // Debug output (per frame)
        void afterPhysics(Entity &entity, double dt, double t) override;

      private:
        //! One task per function
        void applyBallast(double dt);
        void applyWaterThrust(Entity &entity, double dt); // forward + vertical
        void applyManeuvering(Entity &entity);            // lateral + pitch/yaw (+ flight assist)
        void applyRoll(Entity &entity);                   // roll spring

        double calculateForwardThrust(double input) const;

        SubmarineConfig config = {};

        //! Control State
        BallastTank ballast;
        InputRamp verticalRamp;
        bool flightAssist = true;

        //! Callbacks & Debug
        size_t cameraCallbackId = 0;
        size_t flightAssistCallbackId = 0;
        size_t ballastResetCallbackId = 0;
        double physicsDebugTimer = 0.0;
    };
}