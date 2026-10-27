/*
* Copyright © 2026 Tom Kertels
*/

#include "inputMappingValidator.h"

#include <iostream>
#include <string>

using Json = nlohmann::json;

namespace Engine::Core
{
    // Require a JSON object with at least one supported section, then validate each section present.
    bool InputMappingValidator::validate(const Json &config)
    {
        if (!config.is_object())
        {
            std::cout << "! Failed to read input mappings: expected an object" << std::endl;
            return false;
        }

        if (!config.contains("actions") && !config.contains("axes"))
        {
            std::cout << "! Failed to read input mappings: expected actions or axes" << std::endl;
            return false;
        }

        if (config.contains("actions") && !validateActions(config["actions"])) return false;
        if (config.contains("axes") && !validateAxes(config["axes"])) return false;
        return true;
    }

    // Validate the action binding arrays and their GLFW input code ranges.
    bool InputMappingValidator::validateActions(const Json &actions)
    {
        if (!actions.is_object())
        {
            std::cout << "! Failed to read input mappings: actions must be an object" << std::endl;
            return false;
        }

        for (const auto &[action, binding] : actions.items())
        {
            if (!binding.is_object())
            {
                std::cout << "! Failed to read input mapping for " << action << ": expected an object" << std::endl;
                return false;
            }

            bool hasBindings = false;
            for (const std::string name : {"keys", "buttons", "mouseButtons"})
            {
                if (!binding.contains(name)) continue;
                const auto &codes = binding[name];
                const uint32_t maximum = name == "keys" ? 348 : name == "buttons" ? 14 : 7;
                if (!codes.is_array())
                {
                    std::cout << "! Failed to read input mapping for " << action << ": " << name << " must be an array" << std::endl;
                    return false;
                }

                for (const auto &code : codes)
                {
                    if (!isCodeInRange(code, maximum))
                    {
                        std::cout << "! Failed to read input mapping for " << action << ": invalid " << name << " code" << std::endl;
                        return false;
                    }
                }
                hasBindings = hasBindings || !codes.empty();
            }

            // Reject empty actions so they do not silently register as no-ops.
            if (!hasBindings)
            {
                std::cout << "! Failed to read input mapping for " << action << ": no key, gamepad button, or mouse button" << std::endl;
                return false;
            }
        }

        return true;
    }

    // Validate keyboard key pairs and gamepad stick axes used by axis registration.
    bool InputMappingValidator::validateAxes(const Json &axes)
    {
        if (!axes.is_object())
        {
            std::cout << "! Failed to read input mappings: axes must be an object" << std::endl;
            return false;
        }

        for (const auto &[axis, binding] : axes.items())
        {
            if (!binding.is_object())
            {
                std::cout << "! Failed to read input axis " << axis << ": expected an object" << std::endl;
                return false;
            }

            bool hasBindings = false;
            if (binding.contains("keys"))
            {
                const auto &keyPairs = binding["keys"];
                if (!keyPairs.is_array())
                {
                    std::cout << "! Failed to read input axis " << axis << ": keys must be an array" << std::endl;
                    return false;
                }

                for (const auto &pair : keyPairs)
                {
                    if (!pair.is_object() ||
                        !pair.contains("negative") || !isCodeInRange(pair["negative"], 348) ||
                        !pair.contains("positive") || !isCodeInRange(pair["positive"], 348))
                    {
                        std::cout << "! Failed to read input axis " << axis << ": key pairs need valid negative and positive codes" << std::endl;
                        return false;
                    }
                }
                hasBindings = hasBindings || !keyPairs.empty();
            }

            if (binding.contains("gamepad"))
            {
                const auto &gamepadAxes = binding["gamepad"];
                if (!gamepadAxes.is_array())
                {
                    std::cout << "! Failed to read input axis " << axis << ": gamepad must be an array" << std::endl;
                    return false;
                }

                for (const auto &mapping : gamepadAxes)
                {
                    if (!mapping.is_object() ||
                        !mapping.contains("axis") || !isCodeInRange(mapping["axis"], 5) ||
                        (mapping.contains("invert") && !mapping["invert"].is_boolean()))
                    {
                        std::cout << "! Failed to read input axis " << axis << ": invalid gamepad axis mapping" << std::endl;
                        return false;
                    }
                }
                hasBindings = hasBindings || !gamepadAxes.empty();
            }

            // Every declared axis must have at least one usable input source.
            if (!hasBindings)
            {
                std::cout << "! Failed to read input axis " << axis << ": no key or gamepad mappings" << std::endl;
                return false;
            }
        }

        return true;
    }

    // Check integer type and range before codes are converted to uint32_t.
    bool InputMappingValidator::isCodeInRange(const Json &code, uint32_t maximum)
    {
        if (code.is_number_unsigned()) return code.get<uint64_t>() <= maximum;
        if (!code.is_number_integer()) return false;

        const auto number = code.get<int64_t>();
        return number >= 0 && number <= maximum;
    }
}
