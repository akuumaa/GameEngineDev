/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Alexander Hancharenka
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

        std::string type = config.value("type", "fixed");
        camera.type = (type == "orbit") ? CameraType::Orbit : CameraType::Fixed;

        camera.distance = config.value("distance", camera.distance);

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

        // Soll- und Istwert starten gleich: die Kamera beginnt im Ruhezustand,
        // ganz ohne Glättungs-Bewegung, direkt in den gültigen Grenzen.
        camera.yawTarget = camera.yaw;
        camera.pitchTarget = camera.pitch;
        camera.distanceTarget = camera.distance;

        camera.clampTargetToLimits();

        camera.yaw = camera.yawTarget;
        camera.pitch = camera.pitchTarget;
        camera.distance = camera.distanceTarget;

        return camera;
    }

    Matrix Camera::getViewMatrix(const Vector3 &position, const Quaternion &rotation) const
    {
        Vector3 direction = {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), -std::sin(pitch)};

        Vector3 eye = {};
        Vector3 target = {};

        if (type == CameraType::Orbit)
        {
            // Fest bleibt das Ziel (offset); die Kamera steht im Abstand `distance`
            // davon entfernt, entgegen der Blickrichtung.
            target = offset;
            eye = offset - direction * distance;
        }
        else
        {
            // Fest bleibt die Position (offset); nur der Blick (direction) dreht sich.
            eye = offset;
            target = offset + direction;
        }

        return Matrix::lookAt(eye, target) * (-rotation).toMatrix() * Matrix::translation(-position);
    }

    void Camera::update(double dt)
    {
        yawTarget += InputHandler::getAxis("cameraYaw") * lookRate * dt;
        pitchTarget += InputHandler::getAxis("cameraPitch") * lookRate * dt;
        distanceTarget -= InputHandler::getAxis("cameraZoom") * distanceTarget * zoomRate * dt;

        // Maus-Delta wird NICHT mit dt multipliziert: es ist schon eine Bewegung,
        // kein Zustand, der pro Sekunde skaliert werden müsste.
        yawTarget -= InputHandler::getMouseDeltaX() * mouseSensitivity;
        pitchTarget += InputHandler::getMouseDeltaY() * mouseSensitivity;
        distanceTarget -= InputHandler::getMouseDeltaZ() * distanceTarget * zoomStep;

        clampTargetToLimits();

        // Glättung: der Istwert nähert sich dem Sollwert an, framerate-unabhängig,
        // weil der aufgeholte Anteil pro Frame von dt abhängt (exponentielles
        // Herannähern: nach gleicher Zeit immer gleich weit aufgeholt, egal wie
        // viele Frames dafür gebraucht wurden).
        double factor = 1.0 - std::exp(-smoothingRate * dt);

        // Yaw ist zirkulär (kann unbegrenzt wachsen/wrappen): std::remainder liefert
        // den kürzesten Weg zum Ziel, egal wie weit yaw selbst schon gewandert ist.
        double yawDelta = std::remainder(yawTarget - yaw, 2.0 * std::numbers::pi);
        yaw += yawDelta * factor;

        pitch += (pitchTarget - pitch) * factor;
        distance += (distanceTarget - distance) * factor;
    }

    void Camera::clampTargetToLimits()
    {
        yawTarget = yawLimited ? std::clamp(yawTarget, yawMin, yawMax) : std::remainder(yawTarget, 2.0 * std::numbers::pi);
        pitchTarget = std::clamp(pitchTarget, pitchMin, pitchMax);
        distanceTarget = std::clamp(distanceTarget, distanceMin, distanceMax);
    }
}
