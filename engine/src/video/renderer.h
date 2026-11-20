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

#include "engine/src/core/commandbuffer.h"
#include "engine/src/core/settings.h"
#include "engine/src/core/window.h"
#include "engine/src/math/matrix.h"
#include "engine/src/video/model.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

using Engine::Core::CommandBuffer;
using Engine::Core::Settings;
using Engine::Core::Window;
using Engine::Math::Matrix;

namespace Engine::Video
{
    class Renderer
    {
      public:
        Renderer(const std::unique_ptr<CommandBuffer> &commandBuffer, Window &window, const Settings &settings);
        void draw();
        size_t loadModel(const std::string &filename);
        void unloadModel(size_t modelId);

        //! Skybox
        void loadSkybox(const std::string &filename);

        //! Sun and Occlusion Query
        void loadSun(const std::string &sunFilename, const std::string &lensflareFilename);

      private:
        const std::unique_ptr<CommandBuffer> &commandBuffer;

        const Window &window;

        size_t nextModelId = 1;
        std::unordered_map<size_t, std::shared_ptr<Model>> models = {};

        int viewportWidth = 0;
        int viewportHeight = 0;
        double viewportAspect = 1.0;
        double fieldOfView = 0.0;
        double nearPlane = 0.1;
        double farPlane = 1000.0;
        Matrix projectionMatrix = {};

        //! Skybox
        std::shared_ptr<Model> skybox = {};

        //! Sun and Occlusion Query
        std::shared_ptr<Model> sun = {};
        std::shared_ptr<Model> lensflare = {};
        uint32_t sunQueryId = 0;
        uint32_t sunPointCount = 240;
    };
}