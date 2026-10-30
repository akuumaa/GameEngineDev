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

#include "engine/src/sim/camera.h"
#include "engine/src/math/number.h"
#include "engine/src/core/inputhandler.h"

#include <algorithm>
#include <cmath>
#include <json.hpp>
#include <numbers>

using Engine::Core::InputHandler;

namespace Engine::Sim
{
    Camera Camera::create(const Json &config)
    {
        Camera camera = {};

        camera.name = config.value("name", "");

        camera.type = config.value("type", "fixed") == "orbit" ? CameraType::Orbit : CameraType::Fixed;

        camera.distance = config.value("distance", 8.0);
        camera.distanceStart = camera.distance;

        if (config.contains("offset")) camera.offset = {config["offset"][0].get<double>(), config["offset"][1].get<double>(), config["offset"][2].get<double>()};

        if (config.contains("rotation"))
        {
            camera.yaw = Math::Number::toRadians(config["rotation"][0].get<double>());
            camera.pitch = Math::Number::toRadians(config["rotation"][1].get<double>());

            camera.yawStart = camera.yaw;
            camera.pitchStart = camera.pitch;
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
            camera.pitchMin = std::max(camera.pitchMin, std::min(first, second));
            camera.pitchMax = std::min(camera.pitchMax, std::max(first, second));
        }

        camera.clampToLimits();

        return camera;
    }

    void Camera::update(double dt)
    {
        yawTarget += InputHandler::getAxis("cameraYaw") * lookRate * dt;
        pitchTarget += InputHandler::getAxis("cameraPitch") * lookRate * dt;
        distanceTarget -= InputHandler::getAxis("cameraZoom") * distance;

        yawTarget -= InputHandler::getMouseDeltaX() * mouseSensitivity;
        pitchTarget += InputHandler::getMouseDeltaY() * mouseSensitivity;
        distanceTarget -= InputHandler::getMouseDeltaZ() * distance * zoomStep;

        yaw = yaw + (yawTarget - yaw) * lookRate * dt;
        pitch = pitch + (pitchTarget - pitch) * lookRate * dt;
        distance = distance + (distanceTarget - distance) * zoomRate * dt;

        clampToLimits();
    }


    void Camera::clampToLimits()
    {
        if (yawLimited)
        {
            yaw = std::clamp(yaw, yawMin, yawMax);
            yawTarget = std::clamp(yawTarget, yawMin, yawMax);
        }
        else
        {
            double wrapped = std::remainder(yaw, 2.0 * std::numbers::pi);
            yawTarget += wrapped - yaw;
            yaw = wrapped;
        }

        pitch = std::clamp(pitch, pitchMin, pitchMax);
        pitchTarget = std::clamp(pitchTarget, pitchMin, pitchMax);

        distance = std::clamp(distance, distanceMin, distanceMax);
        distanceTarget = std::clamp(distanceTarget, distanceMin, distanceMax);
    }

    void Camera::reset()
    {
        yaw = yawStart;
        pitch = pitchStart;
        distance = distanceStart;
        yawTarget = yawStart;
        pitchTarget = pitchStart;
        distanceTarget = distanceStart;

        clampToLimits();
    }

    Matrix Camera::getViewMatrix(const Vector3 &position, const Quaternion &rotation) const
    {
        Vector3 direction = {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), -std::sin(pitch)};

        Vector3 eye = {};
        Vector3 target = {};

        if (type == CameraType::Fixed)
        {
            eye = position + offset;
            target = position + offset + direction;
        } 
        else if (type == CameraType::Orbit)
        {
            eye = position + offset - direction * distance;
            target = position + offset;
        }

        return Matrix::lookAt(eye, target) * (-rotation).toMatrix() * Matrix::translation(-position);
    }
}
