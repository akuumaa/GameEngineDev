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

#include "engine/src/video/renderer.h"

#include <GLFW/glfw3.h>
#include <glad.c>
#include <numbers>
#include <stdexcept>
#include <algorithm>

using Engine::Core::ClearCommand;
using Engine::Core::RenderItem;

namespace Engine::Video
{
    Renderer::Renderer(const std::unique_ptr<CommandBuffer> &commandBuffer, Window &window, const Settings &settings)
        : commandBuffer(commandBuffer), window(window), fieldOfView(settings.fieldOfView)
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glfwSwapInterval(settings.vsync ? 1 : 0);

        if (settings.msaa) glEnable(GL_MULTISAMPLE);

        glEnable(GL_DEPTH_TEST);

        glEnable(GL_CULL_FACE);
    }

    size_t Renderer::loadModel(const std::string &filename)
    {
        size_t modelId = nextModelId++;
        models[modelId] = Model::create(filename);

        return modelId;
    }

    void Renderer::unloadModel(size_t modelId)
    {
        if (!models.contains(modelId)) return;

        models.erase(modelId);
    }

    void Renderer::loadSkybox(const std::string &filename)
    {
        skybox = Model::create(filename);
    }

    void Renderer::loadSun(const std::string &sunFilename, const std::string &lensflareFilename)
    {
        sun = Model::create(sunFilename);
        lensflare = Model::create(lensflareFilename);

        glGenQueries(1, &sunQueryId);
    }

    void Renderer::draw()
    {
        int width = 0;
        int height = 0;
        window.getFramebufferSize(width, height);

        if (width != viewportWidth || height != viewportHeight)
        {
            viewportWidth = width;
            viewportHeight = height;
            viewportAspect = height > 0 ? (double)width / (double)height : 1.0;
            glViewport(0, 0, width, height);
            projectionMatrix = Matrix::perspective(fieldOfView * std::numbers::pi / 180.0, viewportAspect, nearPlane, farPlane);
        }

        const ClearCommand &clear = commandBuffer->clear;

        glClearColor(clear.red, clear.green, clear.blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrix fixedViewMatrix = commandBuffer->viewMatrix;
        
        fixedViewMatrix.m41 = 0.0;
        fixedViewMatrix.m42 = 0.0;
        fixedViewMatrix.m43 = 0.0;

        if (skybox)
        {
            glDepthMask(GL_FALSE);
            skybox->draw(Matrix::identity(), fixedViewMatrix, projectionMatrix);

            glDepthMask(GL_TRUE);
        }

        for (const RenderItem &item : commandBuffer->renderItems)
        {
            auto it = models.find(item.modelId);
            if (it == models.end()) continue;

            const auto &model = it->second;
            model->draw(item.transform, commandBuffer->viewMatrix, projectionMatrix);
        }

        if (sun && lensflare)
        {
            uint32_t visibleSamples = 0;
            if (glIsQuery(sunQueryId)) glGetQueryObjectuiv(sunQueryId, GL_QUERY_RESULT, &visibleSamples);

            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);

            glBeginQuery(GL_SAMPLES_PASSED, sunQueryId);
            sun->draw(Matrix::identity(), fixedViewMatrix, projectionMatrix);
            glEndQuery(GL_SAMPLES_PASSED);

            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);

            int samplesPerPixel = 0;
            glGetIntegerv(GL_SAMPLES, &samplesPerPixel);

            samplesPerPixel = std::max(samplesPerPixel, 1);

            double sunBrightness = std::clamp(
                static_cast<double>(visibleSamples) /
                    (static_cast<double>(sunPointCount) * samplesPerPixel),
                0.0,
                1.0
            );

            lensflare->setFloat("SunBrightness", sunBrightness);
            lensflare->setFloat("ViewportAspect", viewportAspect);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

            lensflare->draw(Matrix::identity(), fixedViewMatrix, projectionMatrix);

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
    }
}