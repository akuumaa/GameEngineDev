/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Tom Kertels
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

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;
using Engine::Math::Vector3;

namespace App
{
    void ShipScript::update(Entity &entity, double dt, double t)
    {
        double rollInput = InputHandler::getAxis("roll");
        double pitchInput = InputHandler::getAxis("pitch");
        double yawInput = InputHandler::getAxis("yaw");

        const auto [mouseDeltaX, mouseDeltaY] = InputHandler::consumeMouseDelta();

        double mouseRotationSensitivity = rotateWithMouse ? 0.002 : 0.0;

        double pitchStep = pitchInput * dt * rotationSpeed + mouseDeltaY * mouseRotationSensitivity;
        double yawStep   = yawInput * dt * rotationSpeed + mouseDeltaX * mouseRotationSensitivity;
        double rollStep  = rollInput * dt * rotationSpeed;

        Quaternion rotation = entity.getRotation();
        rotation = rotation
                * Quaternion(Vector3::forward, rollStep)
                * Quaternion(Vector3::left, pitchStep)
                * Quaternion(Vector3::up, yawStep);

        rotation = Quaternion::normalize(rotation);

        entity.setRotation(rotation);
    }
}
