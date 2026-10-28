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
#include "engine/src/math/quaternion.h"
#include "engine/src/math/vector3.h"

#include <json_fwd.hpp>
#include <string>


using Engine::Math::Matrix;
using Engine::Math::Quaternion;
using Engine::Math::Vector3;

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

        const std::string &getName() const { return name; }

      private:
        CameraType type = CameraType::Fixed;
        std::string name = {};
        Vector3 offset = {};

        double yaw = 0.0;
        double pitch = 0.0;
        double distance = 8.0;
        Vector3 position = {7.3589, -6.9258, 4.9583};
        Vector3 target = {0.0, 0.0, 0.0};
    };
}