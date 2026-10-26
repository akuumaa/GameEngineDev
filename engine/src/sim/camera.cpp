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
        Camera camera = {};

        camera.name = config.value("name", "");

        if (config.contains("offset")) camera.offset = {config["offset"][0].get<double>(), config["offset"][1].get<double>(), config["offset"][2].get<double>()};

        if (config.contains("rotation"))
        {
            camera.yaw = Math::Number::toRadians(config["rotation"][0].get<double>());
            camera.pitch = Math::Number::toRadians(config["rotation"][1].get<double>());
        }

        // TODO: "type" einlesen. "orbit" ergibt CameraType::Orbit.
        //       Alles andere und ein fehlender Eintrag ergeben CameraType::Fixed.

        // TODO: "distance" einlesen, falls vorhanden.
        //       Tipp: config.value(key, fallback) ist eine sehr angenehme Möglichkeit,
        //       Werte aus dem Json zu lesen.

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

    Matrix Camera::getViewMatrix(const Vector3 &position, const Quaternion &rotation) const
    {
        Vector3 direction = {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), -std::sin(pitch)};

        Vector3 eye = {};
        Vector3 target = {};

        // TODO Fixed: Die Kamera steht bei offset und blickt in Richtung direction.
        //             Das entspricht dem bisherigen Aufruf von lookAt().

        // TODO Orbit: Die Kamera blickt auf offset. Sie steht im Abstand distance
        //             so, dass der Blick von eye nach target in Richtung direction zeigt.
        //             direction ist ein Einheitsvektor.

        return Matrix::lookAt(offset, offset + direction) * (-rotation).toMatrix() * Matrix::translation(-position);
    }

    void Camera::update(double dt)
    {
        yaw += InputHandler::getAxis("cameraYaw") * lookRate * dt;
        pitch += InputHandler::getAxis("cameraPitch") * lookRate * dt;
        distance -= InputHandler::getAxis("cameraZoom") * distance * zoomRate * dt;

        yaw -= InputHandler::getMouseDeltaX() * mouseSensitivity;
        pitch += InputHandler::getMouseDeltaY() * mouseSensitivity;
        distance -= InputHandler::getMouseDeltaZ() * distance * zoomStep;

        clampToLimits();
    }

    void Camera::clampToLimits()
    {
        yaw = yawLimited ? std::clamp(yaw, yawMin, yawMax) : std::remainder(yaw, 2.0 * std::numbers::pi);
        pitch = std::clamp(pitch, pitchMin, pitchMax);
        distance = std::clamp(distance, distanceMin, distanceMax);
    }

}
