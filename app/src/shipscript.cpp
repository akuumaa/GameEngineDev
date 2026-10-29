/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright � 2026 Yannick Kissel
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
#include "engine/src/math/vector3.h"
#include "engine/src/sim/entity.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using Engine::Core::InputHandler;
using Engine::Math::Quaternion;
using Engine::Math::Vector3;

/**
 * Steuerungsachsen = Beschleunigung, Bewegung Kurven
 */

namespace App
{
    void ShipScript::update(Entity &entity, double dt, double t)
    {
        // Substeps keep acceleration and curved movement stable even at low frame rates.
        double remaining = std::clamp(dt, 0.0, 0.25);
        double throttle = InputHandler::getAxis("throttle");
        double steer = InputHandler::getAxis("steer");
        double bank = InputHandler::getAxis("bank");
        constexpr double maxRoll = std::numbers::pi / 4.0;
        while (remaining > 0.0)
        {
            double step = std::min(remaining, 1.0 / 120.0);
            remaining -= step;
            speed = std::clamp(speed + throttle * 12.0 * step, -12.0, 30.0);
            if (throttle == 0.0) speed *= std::exp(-1.5 * step);

            double targetRoll = std::clamp(-steer * 0.20 - bank * maxRoll, -maxRoll, maxRoll);
            roll += (targetRoll - roll) * (1.0 - std::exp(-6.0 * step));
            // Banking adds a stronger turn; A/D and Q/E combine for tighter curves.
            yaw = std::remainder(yaw + (steer * 0.65 + bank * 1.05) * step,
                                 2.0 * std::numbers::pi);
            pitch = std::clamp(pitch - InputHandler::getAxis("pitch") * step,
                               -std::numbers::pi / 3.0, std::numbers::pi / 3.0);
            Quaternion rotation = Quaternion::normalize(Quaternion(yaw, pitch, roll));
            entity.setRotation(rotation);
            Vector3 movement = (rotation * Vector3::forward) * speed
                             + Vector3::up * (InputHandler::getAxis("lift") * 8.0);
            entity.setPosition(entity.getPosition() + movement * step);
        }
    }
}
