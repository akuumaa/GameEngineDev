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

#pragma once

#include "engine/src/math/matrix.h"
#include "engine/src/math/vector3.h"
#include "engine/src/math/quaternion.h"

#include <json_fwd.hpp>
#include <string>

using Engine::Math::Matrix;
using Engine::Math::Vector3;
using Engine::Math::Quaternion;

using Json = nlohmann::json;

namespace Engine::Sim
{
    enum class CameraType
    {
        Fixed,
        Orbit
    };

    class Camera
    {
      public:
        static Camera create(const Json &config);
        
        Matrix getViewMatrix(const Vector3 &position, const Quaternion &rotation) const;

        void update(double dt);

        const std::string &getName() const { return name; }

      private:
        void clampToLimits();

        std::string name = {};
        Vector3 offset = {};
        CameraType type = CameraType::Fixed;

        double mouseSensitivity = 1.0 / 256.0;

        bool yawLimited = false;
        double yawMin = 0.0;
        double yawMax = 0.0;
        double pitchMin = -1.55;
        double pitchMax = 1.55;
        double lookRate = 2.0;

        double yaw = 0.0;
        double pitch = 0.0;
        double distance = 8.0;

        double distanceMin = 1.5;
        double distanceMax = 200.0;
        double zoomRate = 1.5;
        double zoomStep = 0.1;
    };
}