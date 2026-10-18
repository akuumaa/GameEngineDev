#include "inputMapper.h"
#include "inputhandler.h"
#include "inputMappingValidator.h"
#include "engine/src/core/application.h"

#include <iostream>
#include <json.hpp>
#include <sstream>

using Json = nlohmann::json;

namespace Engine::Core
{
    void InputMapper::loadMappings(const std::string &filename)
    {
        std::unique_ptr<std::istream> mappings = Application::load(filename);
        if (!mappings) return;

        loadMappings(*mappings);
    }

    void InputMapper::loadMappings(std::istream &mappings)
    {
        std::ostringstream text;
        text << mappings.rdbuf();

        const Json config = Json::parse(text.str(), nullptr, false);
        if (config.is_discarded())
        {
            std::cout << "! Failed to parse input mappings" << std::endl;
            return;
        }

        if (!InputMappingValidator::validate(config)) return;

        if (config.contains("actions")) loadActions(config["actions"]);
        if (config.contains("axes")) loadAxes(config["axes"]);
    }

    void InputMapper::loadActions(const Json &actions)
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
            if (binding.contains("buttons"))
            {
                for (const auto &button : binding["buttons"])
                {
                    InputHandler::registerButtonAction(action, button.get<uint32_t>());
                }
            }
        }
    }

    void InputMapper::loadAxes(const Json &axes)
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
                        pair["positive"].get<uint32_t>());
                }
            }
            if (binding.contains("gamepad"))
            {
                for (const auto &mapping : binding["gamepad"])
                {
                    InputHandler::registerStickAxis(
                        axis,
                        mapping["axis"].get<uint32_t>(),
                        mapping.value("invert", false));
                }
            }
        }
    }
}
