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

#include <memory>
#define STB_IMAGE_IMPLEMENTATION

#include "engine/src/core/application.h"
#include "engine/src/video/texture.h"

#include <chrono>
#include <cstddef>
#include <cstring>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <ratio>
#include <stb_image.h>
#include <vector>

using Engine::Core::Application;

namespace Engine::Video
{
    std::shared_ptr<Texture> Texture::create(const std::string &filename)
    {
        return cache.get(filename, [filename]()
        {
            auto texture = std::shared_ptr<Texture>(new Texture());
            TextureData textureData = {filename};

            bool success = readFile(textureData);
            if (!success) return texture;

            genTexture(*texture, textureData);
            return texture;
        });
    }

    bool Texture::readFile(TextureData &data)
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        auto file = Application::load(data.filename, std::ios::binary);
        if (!file) return false;

        std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(*file), std::istreambuf_iterator<char>()};

        if (data.filename.ends_with(".dds"))
        {
            std::memcpy(&data.height, bytes.data() + 12, 4);
            std::memcpy(&data.width, bytes.data() + 16, 4);
            std::memcpy(&data.mipMapCount, bytes.data() + 28, 4);
            std::string fourCC(bytes.begin() + 84, bytes.begin() + 88);

            if (fourCC == "DXT1")
            {
                data.format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                data.blockSize = 8;
            }
            else if (fourCC == "ATI1")
            {
                data.format = GL_COMPRESSED_RED_RGTC1;
                data.blockSize = 8;
            }
            else if (fourCC == "ATI2")
            {
                data.format = GL_COMPRESSED_RG_RGTC2;
                data.blockSize = 16;
            }
            else
            {
                std::cout << "! Failed to read texture " + data.filename + ": unsupported format " + fourCC << std::endl;
                return false;
            }

            data.blocks.assign(bytes.begin() + 128, bytes.end());

            std::chrono::duration<double, std::milli> readTime = std::chrono::steady_clock::now() - start;
            std::cout << "  Reading texture " + data.filename + " took " + std::to_string(readTime.count()) + " ms" << std::endl;

            return true;
        }

        data.content = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &data.width, &data.height, &data.channels, 0);
        if (!data.content || data.channels == 0 || data.width == 0 || data.height == 0)
        {
            std::cout << "! Failed to decode texture " + data.filename << std::endl;
            stbi_image_free(data.content);
            return false;
        }

        std::chrono::duration<double, std::milli> readTime = std::chrono::steady_clock::now() - start;
        std::cout << "  Reading texture " + data.filename + " took " + std::to_string(readTime.count()) + " ms" << std::endl;

        return true;
    }

    void Texture::uploadBlocks(uint32_t target, const TextureData &data)
    {
        int width = data.width;
        int height = data.height;
        size_t offset = 0;

        for (int level = 0; level < data.mipMapCount; level++)
        {
            int size = ((width + 3) / 4) * ((height + 3) / 4) * data.blockSize;
            glCompressedTexImage2D(target, level, data.format, width, height, 0, size, data.blocks.data() + offset);

            offset += size;
            width /= 2;
            height /= 2;
        }
    }

    void Texture::genTexture(Texture &texture, const TextureData &data)
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        while (glGetError() != GL_NO_ERROR)
        {
        }

        uint32_t textureId = 0;

        glGenTextures(1, &textureId);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (data.format != 0)
        {
            uploadBlocks(GL_TEXTURE_2D, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, data.mipMapCount - 1);
        }

        if (data.channels == 1)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, data.width, data.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.content);
        }
        else if (data.channels == 2)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, data.width, data.height, 0, GL_RG, GL_UNSIGNED_BYTE, data.content);
        }
        else if (data.channels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.content);
        }
        else if (data.channels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.content);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data.content);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (glGetError() != GL_NO_ERROR)
        {
            std::cout << "! Failed to generate texture " + data.filename << std::endl;
            glDeleteTextures(1, &textureId);
            textureId = 0;
        }

        texture.textureId = textureId;

        std::chrono::duration<double, std::milli> genTime = std::chrono::steady_clock::now() - start;
        std::cout << "  Generating texture " + data.filename + " took " + std::to_string(genTime.count()) + " ms" << std::endl;
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &textureId);
    }
}