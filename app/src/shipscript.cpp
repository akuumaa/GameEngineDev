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

#include <iostream>

using Engine::Core::InputHandler;

using Engine::Math::Quaternion;

namespace App
{

    void ShipScript::update(Entity &entity, double dt, double t)
    {

        constexpr double thrust = 20.0;
        constexpr double angularThrust = 2.0;

        Vector3 thrusterInput = {InputHandler::getAxis("thrustX"), InputHandler::getAxis("thrustY"), InputHandler::getAxis("thrustZ")};
        Vector3 angularThrusterInput = {InputHandler::getAxis("roll"), InputHandler::getAxis("pitch"), InputHandler::getAxis("yaw")};

        Vector3 position = entity.getPosition();
        Quaternion rotation = entity.getRotation();

        Vector3 acceleration = thrusterInput * thrust;
        Vector3 angularAcceleration = angularThrusterInput * angularThrust;

        velocity = velocity + rotation * acceleration * dt;
        angularVelocity = angularVelocity + angularAcceleration * dt;

        rotation = rotation * Quaternion(Vector3::forward, angularVelocity.x * dt) * Quaternion(Vector3::left, angularVelocity.y * dt) * Quaternion(Vector3::up, angularVelocity.z * dt);

        rotation = Quaternion::normalize(rotation);

        entity.setPosition(position + velocity * dt);
        entity.setRotation(rotation);
    }

    void ShipScript::load(Entity &entity)
    {
        cameraCallbackId = InputHandler::onAction("nextCamera", [&entity]()
        {
            entity.nextCamera();
            std::cout << "+ Switched Camera" << std::endl;
        });
    }

    void ShipScript::unload(Entity &entity)
    {
        InputHandler::removeCallback(cameraCallbackId);
    }

}
