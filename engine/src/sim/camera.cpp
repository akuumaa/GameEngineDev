/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright Â© 2021-2026 Tobias Reimann
 * Copyright © 2026 Yannick Kissel
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

#include "engine/src/sim/camera.h"

#include "engine/src/core/inputhandler.h"
#include "engine/src/math/number.h"

#include <algorithm>
#include <cmath>
#include <json.hpp>
#include <numbers>

using Engine::Core::InputHandler;

namespace Engine::Sim
{
    Camera Camera::create(const Json &config)
    {
        Camera camera;
        camera.name = config.value("name", "");
        camera.type = config.value("type", "fixed") == "orbit" ? CameraType::Orbit : CameraType::Fixed;
        camera.distance = config.value("distance", 8.0);
        if (config.contains("offset"))
            camera.offset = {config["offset"][0].get<double>(), config["offset"][1].get<double>(), config["offset"][2].get<double>()};
        if (config.contains("rotation"))
        {
            camera.yaw = Math::Number::toRadians(config["rotation"][0].get<double>());
            camera.pitch = Math::Number::toRadians(config["rotation"][1].get<double>());
        }
        if (config.contains("yawLimit"))
        {
            double first = Math::Number::toRadians(config["yawLimit"][0].get<double>());
            double second = Math::Number::toRadians(config["yawLimit"][1].get<double>());
            camera.yawLimited = true;
            camera.yawMin = std::min(first, second);
            camera.yawMax = std::max(first, second);
        }
        if (config.contains("pitchLimit"))
        {
            double first = Math::Number::toRadians(config["pitchLimit"][0].get<double>());
            double second = Math::Number::toRadians(config["pitchLimit"][1].get<double>());
            // Even out-of-range configurations must not invert the clamp interval.
            camera.pitchMin = std::clamp(std::min(first, second), -1.55, 1.55);
            camera.pitchMax = std::clamp(std::max(first, second), -1.55, 1.55);
        }
        camera.stopSmoothing();
        camera.clampToLimits();
        camera.yaw = camera.targetYaw;
        camera.pitch = camera.targetPitch;
        camera.distance = camera.targetDistance;
        return camera;
    }

    Matrix Camera::getViewMatrix(const Vector3 &position, const Quaternion &rotation) const
    {
        Vector3 direction = {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), -std::sin(pitch)};
        Vector3 eye = type == CameraType::Orbit ? offset - direction * distance : offset;
        Vector3 target = type == CameraType::Orbit ? offset : offset + direction;
        return Matrix::lookAt(eye, target) * (-rotation).toMatrix() * Matrix::translation(-position);
    }

    void Camera::stopSmoothing()
    {
        targetYaw = yaw;
        targetPitch = pitch;
        targetDistance = distance;
    }

    void Camera::update(double dt)
    {
        dt = std::max(0.0, dt);
        targetYaw += InputHandler::getAxis("cameraYaw") * lookRate * dt;
        targetPitch += InputHandler::getAxis("cameraPitch") * lookRate * dt;
        targetYaw -= InputHandler::getMouseDeltaX() * mouseSensitivity;
        targetPitch += InputHandler::getMouseDeltaY() * mouseSensitivity;
        // Exponential zoom stays positive and scales consistently across frame rates.
        targetDistance *= std::exp(std::clamp(-InputHandler::getAxis("cameraZoom") * zoomRate * dt
                                            +InputHandler::getMouseDeltaZ() * std::log1p(-zoomStep), -50.0, 50.0));
        clampToLimits();
        double blend = -std::expm1(-smoothingRate * dt);
        double yawDifference = targetYaw - yaw;
        if (!yawLimited) yawDifference = std::remainder(yawDifference, 2.0 * std::numbers::pi);
        yaw += yawDifference * blend;
        if (!yawLimited) yaw = std::remainder(yaw, 2.0 * std::numbers::pi);
        pitch += (targetPitch - pitch) * blend;
        distance += (targetDistance - distance) * blend;
    }

    void Camera::clampToLimits()
    {
        targetYaw = yawLimited ? std::clamp(targetYaw, yawMin, yawMax)
                              : std::remainder(targetYaw, 2.0 * std::numbers::pi);
        targetPitch = std::clamp(targetPitch, pitchMin, pitchMax);
        targetDistance = std::clamp(targetDistance, distanceMin, distanceMax);
    }
}
