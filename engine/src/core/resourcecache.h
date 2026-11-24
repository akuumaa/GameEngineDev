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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Core
{
    template<class T>
    class ResourceCache
    {
      public:
        std::shared_ptr<T> get(const std::string &key, std::function<std::shared_ptr<T>()> factory)
        {
            auto entry = cache.find(key);
            if (entry != cache.end())
            {
                std::shared_ptr<T> resource = entry->second.lock();
                if (resource) return resource;
            }
            std::shared_ptr<T> resource = factory();
            cache[key] = resource;
            return resource;
        }

      private:
        std::unordered_map<std::string, std::weak_ptr<T>> cache = {};
    };
}