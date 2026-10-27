/*
 * Copyright © 2026 Tom Kertels
 */

#include "inputMappingValidator.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <string_view>

using Json = nlohmann::json;

namespace Engine::Core
{
    // Require a JSON object with at least one supported section, then validate each section present.
    bool InputMappingValidator::validate(const Json &config)
    {
        if (!config.is_object())
        {
            std::cerr << "! Failed to read input mappings: expected an object" << std::endl;
            return false;
        }

        if (!config.contains("actions") && !config.contains("axes"))
        {
            std::cerr << "! Failed to read input mappings: expected actions or axes" << std::endl;
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
            std::cerr << "! Failed to read input mappings: actions must be an object" << std::endl;
            return false;
        }

        for (const auto &[action, binding] : actions.items())
        {
            if (!binding.is_object())
            {
                std::cerr << "! Failed to read input mapping for " << action << ": expected an object" << std::endl;
                return false;
            }

            bool hasBindings = false;
            for (const char *name : {"keys", "buttons", "mouseButtons"})
            {
                if (!binding.contains(name)) continue;
                const auto &codes = binding[name];
                if (!codes.is_array())
                {
                    std::cerr << "! Failed to read input mapping for " << action << ": " << name << " must be an array" << std::endl;
                    return false;
                }

                for (const auto &code : codes)
                {
                    const bool valid = std::string_view(name) == "keys"
                                           ? isKeyBinding(code)
                                           : isCodeInRange(code, std::string_view(name) == "buttons" ? GLFW_GAMEPAD_BUTTON_LAST : GLFW_MOUSE_BUTTON_LAST);
                    if (!valid)
                    {
                        std::cerr << "! Failed to read input mapping for " << action << ": invalid " << name << " code or key name" << std::endl;
                        return false;
                    }
                }
                hasBindings = hasBindings || !codes.empty();
            }

            // Reject empty actions so they do not silently register as no-ops.
            if (!hasBindings)
            {
                std::cerr << "! Failed to read input mapping for " << action << ": no key, gamepad button, or mouse button" << std::endl;
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
            std::cerr << "! Failed to read input mappings: axes must be an object" << std::endl;
            return false;
        }

        for (const auto &[axis, binding] : axes.items())
        {
            if (!binding.is_object())
            {
                std::cerr << "! Failed to read input axis " << axis << ": expected an object" << std::endl;
                return false;
            }

            bool hasBindings = false;
            if (binding.contains("keys"))
            {
                const auto &keyPairs = binding["keys"];
                if (!keyPairs.is_array())
                {
                    std::cerr << "! Failed to read input axis " << axis << ": keys must be an array" << std::endl;
                    return false;
                }

                for (const auto &pair : keyPairs)
                {
                    if (!pair.is_object() ||
                        !pair.contains("negative") || !isKeyBinding(pair["negative"]) ||
                        !pair.contains("positive") || !isKeyBinding(pair["positive"]))
                    {
                        std::cerr << "! Failed to read input axis " << axis << ": key pairs need valid negative and positive codes" << std::endl;
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
                    std::cerr << "! Failed to read input axis " << axis << ": gamepad must be an array" << std::endl;
                    return false;
                }

                for (const auto &mapping : gamepadAxes)
                {
                    if (!mapping.is_object() ||
                        !mapping.contains("axis") || !isCodeInRange(mapping["axis"], GLFW_GAMEPAD_AXIS_LAST) ||
                        (mapping.contains("invert") && !mapping["invert"].is_boolean()))
                    {
                        std::cerr << "! Failed to read input axis " << axis << ": invalid gamepad axis mapping" << std::endl;
                        return false;
                    }
                }
                hasBindings = hasBindings || !gamepadAxes.empty();
            }

            // Every declared axis must have at least one usable input source.
            if (!hasBindings)
            {
                std::cerr << "! Failed to read input axis " << axis << ": no key or gamepad mappings" << std::endl;
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

    bool InputMappingValidator::isKeyBinding(const Json &key)
    {
        return key.is_string() ? !key.get_ref<const std::string &>().empty() : isCodeInRange(key, GLFW_KEY_LAST);
    }
}
