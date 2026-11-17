/*
 * Copyright © 2026 Tom Kertels
 */

#pragma once

#include <cstdint>
#include <istream>
#include <json.hpp>
#include <string>
#include <optional>
#include <unordered_map>

namespace Engine::Core
{
    class InputMapper
    {
      public:
        InputMapper() = delete;

        static bool loadMappings(const std::string &filename);
        static bool loadMappings(std::istream &mappings);

      private:
        static const std::unordered_map<std::string, uint32_t> keyNames;
        static bool checkGlfwRanges(const nlohmann::json &config);
        static bool resolveKeyNames(nlohmann::json &config);
        static bool replaceWithKeyCode(nlohmann::json &key);
        static std::optional<uint32_t> parseKey(const nlohmann::json &key);
        static bool isInRange(const nlohmann::json &code, uint32_t maximum);
        
        static void loadActions(const nlohmann::json &actions);
        static void loadAxes(const nlohmann::json &axes);
    };
}