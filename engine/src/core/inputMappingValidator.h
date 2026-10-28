/*
 * Copyright © 2026 Tom Kertels
 */
 
#pragma once
 
#include <json.hpp>
 
namespace Engine::Core
{
    class InputMappingValidator
    {
      public:
        InputMappingValidator() = delete;
 
        static bool validate(const nlohmann::json &config);
 
      private:
        static bool validateActions(const nlohmann::json &actions);
        static bool validateAxes(const nlohmann::json &axes);
        static bool isNonNegativeInteger(const nlohmann::json &code);
        static bool isKeyBinding(const nlohmann::json &key);
    };
}
