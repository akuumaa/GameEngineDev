/*
 * Copyright © 2026 Tom Kertels
 */

#include "inputMapper.h"

#include "engine/src/core/application.h"
#include "inputMappingValidator.h"
#include "inputhandler.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <charconv>
#include <iostream>
#include <json.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

using Json = nlohmann::json;

namespace Engine::Core
{
    namespace
    {
        void loadActions(const Json &actions);
        void loadAxes(const Json &axes);

        bool parseKey(const Json &key, uint32_t &code)
        {
            if (key.is_number_unsigned())
            {
                const uint64_t value = key.get<uint64_t>();
                if (value > GLFW_KEY_LAST) return false;
                code = static_cast<uint32_t>(value);
                return true;
            }

            if (key.is_number_integer())
            {
                const int64_t value = key.get<int64_t>();
                if (value < 0 || value > GLFW_KEY_LAST) return false;
                code = static_cast<uint32_t>(value);
                return true;
            }

            if (!key.is_string()) return false;
            const std::string name = key.get<std::string>();

            if (name.size() == 1 && ((name[0] >= 'A' && name[0] <= 'Z') || (name[0] >= '0' && name[0] <= '9')))
            {
                code = static_cast<uint32_t>(name[0]);
                return true;
            }

            if (name.size() >= 2 && name[0] == 'F')
            {
                const std::string_view suffix(name.data() + 1, name.size() - 1);
                unsigned int number = 0;
                const auto [end, error] = std::from_chars(suffix.data(), suffix.data() + suffix.size(), number);
                if (error != std::errc{} || end != suffix.data() + suffix.size() ||
                    (suffix.size() > 1 && suffix.front() == '0') || number < 1 || number > 25)
                {
                    return false;
                }
                code = GLFW_KEY_F1 + number - 1;
                return true;
            }

            if (name.size() == 4 && name.compare(0, 3, "KP_") == 0 && name[3] >= '0' && name[3] <= '9')
            {
                code = GLFW_KEY_KP_0 + static_cast<uint32_t>(name[3] - '0');
                return true;
            }

            static const std::unordered_map<std::string, uint32_t> keys = {
                {"SPACE", GLFW_KEY_SPACE},
                {"APOSTROPHE", GLFW_KEY_APOSTROPHE},
                {"COMMA", GLFW_KEY_COMMA},
                {"MINUS", GLFW_KEY_MINUS},
                {"PERIOD", GLFW_KEY_PERIOD},
                {"SLASH", GLFW_KEY_SLASH},
                {"SEMICOLON", GLFW_KEY_SEMICOLON},
                {"EQUAL", GLFW_KEY_EQUAL},
                {"LEFT_BRACKET", GLFW_KEY_LEFT_BRACKET},
                {"BACKSLASH", GLFW_KEY_BACKSLASH},
                {"RIGHT_BRACKET", GLFW_KEY_RIGHT_BRACKET},
                {"GRAVE_ACCENT", GLFW_KEY_GRAVE_ACCENT},
                {"WORLD_1", GLFW_KEY_WORLD_1},
                {"WORLD_2", GLFW_KEY_WORLD_2},
                {"ESCAPE", GLFW_KEY_ESCAPE},
                {"ENTER", GLFW_KEY_ENTER},
                {"TAB", GLFW_KEY_TAB},
                {"BACKSPACE", GLFW_KEY_BACKSPACE},
                {"INSERT", GLFW_KEY_INSERT},
                {"DELETE", GLFW_KEY_DELETE},
                {"RIGHT", GLFW_KEY_RIGHT},
                {"LEFT", GLFW_KEY_LEFT},
                {"DOWN", GLFW_KEY_DOWN},
                {"UP", GLFW_KEY_UP},
                {"PAGE_UP", GLFW_KEY_PAGE_UP},
                {"PAGE_DOWN", GLFW_KEY_PAGE_DOWN},
                {"HOME", GLFW_KEY_HOME},
                {"END", GLFW_KEY_END},
                {"CAPS_LOCK", GLFW_KEY_CAPS_LOCK},
                {"SCROLL_LOCK", GLFW_KEY_SCROLL_LOCK},
                {"NUM_LOCK", GLFW_KEY_NUM_LOCK},
                {"PRINT_SCREEN", GLFW_KEY_PRINT_SCREEN},
                {"PAUSE", GLFW_KEY_PAUSE},
                {"KP_DECIMAL", GLFW_KEY_KP_DECIMAL},
                {"KP_DIVIDE", GLFW_KEY_KP_DIVIDE},
                {"KP_MULTIPLY", GLFW_KEY_KP_MULTIPLY},
                {"KP_SUBTRACT", GLFW_KEY_KP_SUBTRACT},
                {"KP_ADD", GLFW_KEY_KP_ADD},
                {"KP_ENTER", GLFW_KEY_KP_ENTER},
                {"KP_EQUAL", GLFW_KEY_KP_EQUAL},
                {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT},
                {"LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL},
                {"LEFT_ALT", GLFW_KEY_LEFT_ALT},
                {"LEFT_SUPER", GLFW_KEY_LEFT_SUPER},
                {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
                {"RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
                {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT},
                {"RIGHT_SUPER", GLFW_KEY_RIGHT_SUPER},
                {"MENU", GLFW_KEY_MENU},
            };

            const auto entry = keys.find(name);
            if (entry == keys.end()) return false;

            code = entry->second;
            return true;
        }

        bool resolveKeyNames(Json &config)
        {
            auto resolve = [](Json &key)
            {
                uint32_t code = 0;
                if (!parseKey(key, code)) return false;
                key = code;
                return true;
            };

            if (config.contains("actions"))
            {
                for (auto &[action, binding] : config["actions"].items())
                {
                    if (!binding.contains("keys")) continue;
                    for (auto &key : binding["keys"])
                    {
                        if (resolve(key)) continue;
                        std::cerr << "! Failed to read input mapping for " << action << ": unknown key " << key << std::endl;
                        return false;
                    }
                }
            }

            if (config.contains("axes"))
            {
                for (auto &[axis, binding] : config["axes"].items())
                {
                    if (!binding.contains("keys")) continue;
                    for (auto &pair : binding["keys"])
                    {
                        for (const char *name : {"negative", "positive"})
                        {
                            Json &key = pair[name];
                            if (resolve(key)) continue;
                            std::cerr << "! Failed to read input axis " << axis << ": unknown key " << key << std::endl;
                            return false;
                        }
                    }
                }
            }

            return true;
        }

    }

    bool InputMapper::loadMappings(const std::string &filename)
    {
        std::unique_ptr<std::istream> mappings = Application::load(filename);
        if (!mappings) return false;

        return loadMappings(*mappings);
    }

    bool InputMapper::loadMappings(std::istream &mappings)
    {
        std::ostringstream text;
        text << mappings.rdbuf();

        Json config = Json::parse(text.str(), nullptr, false);
        if (config.is_discarded())
        {
            std::cerr << "! Failed to parse input mappings" << std::endl;
            return false;
        }

        if (!InputMappingValidator::validate(config)) return false;
        if (!resolveKeyNames(config)) return false;

        if (config.contains("actions")) loadActions(config["actions"]);
        if (config.contains("axes")) loadAxes(config["axes"]);
        return true;
    }

    namespace
    {
        void loadActions(const Json &actions)
        {
            for (const auto &[action, binding] : actions.items())
            {
                if (binding.contains("keys"))
                {
                    for (const auto &key : binding["keys"])
                    {
                        InputHandler::registerKeyAction(action, key.get<uint32_t>());
                    }
                }
                if (binding.contains("mouseButtons"))
                {
                    for (const auto &button : binding["mouseButtons"])
                    {
                        InputHandler::registerMouseButtonAction(action, button.get<uint32_t>());
                    }
                }
                if (binding.contains("buttons"))
                {
                    for (const auto &button : binding["buttons"])
                    {
                        InputHandler::registerButtonAction(action, button.get<uint32_t>());
                    }
                }
            }
        }

        void loadAxes(const Json &axes)
        {
            for (const auto &[axis, binding] : axes.items())
            {
                if (binding.contains("keys"))
                {
                    for (const auto &pair : binding["keys"])
                    {
                        InputHandler::registerKeyAxis(
                            axis,
                            pair["negative"].get<uint32_t>(),
                            pair["positive"].get<uint32_t>()
                        );
                    }
                }
                if (binding.contains("gamepad"))
                {
                    for (const auto &mapping : binding["gamepad"])
                    {
                        InputHandler::registerStickAxis(
                            axis,
                            mapping["axis"].get<uint32_t>(),
                            mapping.value("invert", false)
                        );
                    }
                }
            }
        }
    }
}
