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

//debug
#include <iostream>

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
        
        // vorgegeben
        if (config.contains("yawLimit"))
        {
            double first = Math::Number::toRadians(config["yawLimit"][0].get<double>());
            double second = Math::Number::toRadians(config["yawLimit"][1].get<double>());
            camera.yawLimited = true;
            camera.yawMin = std::min(first, second);
            camera.yawMax = std::max(first, second);
        }
        
        // vorgegeben
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

        camera.clampToLimits();

        camera.yaw = camera.yawTarget;
        camera.pitch = camera.pitchTarget;
        camera.distance = camera.distanceTarget;

        return camera;
    }

    Matrix Camera::getViewMatrix(const Vector3 &position, const Quaternion &rotation) const
    {   
        /** vorgegebene Funktion
         * yaw	pitch	Ergebnis	        Blick
            0°	0°	    (1, 0, 0)	        geradeaus nach vorne
            90°	0°	    (0, 1, 0)	        nach links
            0°	45°	    (0,71, 0, -0,71)	schräg nach unten
            0°	90°	    (0, 0, -1)	        gerade nach unten
            90°	90°	    (0, 0, -1)	        gerade nach unten

            
            x = cos(pitch) * cos(yaw)
            y = cos(pitch) * sin(yaw)
            z = -sin(pitch)
         */ 
        Vector3 direction = {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), -std::sin(pitch)};

        Vector3 eye = {};
        Vector3 target = {};

        if (type == CameraType::Orbit)
        {
            // Fest bleibt das Ziel (offset) die Kamera steht im Abstand `distance`
            // davon entfernt, entgegen der Blickrichtung
            target = offset;
            eye = offset - direction * distance;
        }
        else
        {
            // Fest bleibt die Position (offset) nur der Blick (direction) dreht sich
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

        double factor = 1.0 - std::exp(-smoothingRate * dt);

        // Yaw ist zirkulär (kann unbegrenzt wachsen/wrappen): std::remainder liefert
        // den kürzesten Weg zum Ziel, egal wie weit yaw selbst schon gewandert ist.
        double yawDelta = yawTarget - yaw;
        yaw += yawDelta * factor;
        
        pitch += (pitchTarget - pitch) * factor;
        distance += (distanceTarget - distance) * factor;

        clampToLimits();

        // Glättung: der Istwert nähert sich dem Sollwert an, framerate-unabhängig,


        //DEBUG
        if (std::abs(yawDelta) > 0.2 || dt > 0.03)
        {
            std::cout << name << " dt=" << dt << " yaw=" << yaw << " target=" << yawTarget
                    << " delta=" << yawDelta << " pitch=" << pitch << std::endl;
        }

    }

    void Camera::clampToLimits()
    {
        if (yawLimited) yawTarget = std::clamp(yawTarget, yawMin, yawMax);
        //yaw = yawLimited ? std::clamp(yaw, yawMin, yawMax) : std::remainder(yaw, 2.0 * std::numbers::pi);
        pitchTarget = std::clamp(pitchTarget, pitchMin, pitchMax);
        distanceTarget = std::clamp(distanceTarget, distanceMin, distanceMax);
    }
}
