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

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "engine/src/core/inputhandler.h"

#include <utility>
#include <algorithm>
#include <istream>
#include <sstream>


namespace Engine::Core
{
    void InputHandler::keyCallback(uint32_t key, uint32_t scancode, uint32_t mods, bool pressed)
    {
        if (key < keyboardState.size())
        {
            keyboardState[key] = pressed;
        }

        if (auto entry = keyCallbacks.find(key); pressed && entry != keyCallbacks.end())
        {
            for (const auto &callback : entry->second)
            {
                callback();
            }
        }

        if (auto actions = keyboardActions.find(key); pressed && actions != keyboardActions.end())
        {
            for (const auto &action : actions->second)
            {
                fireAction(action);
            }
        }
    }

    void InputHandler::onKeyPress(uint32_t code, std::function<void()> callback)
    {
        keyCallbacks[code].push_back(std::move(callback));
    }

    void InputHandler::registerKeyAction(const std::string &action, uint32_t key)
    {
        keyboardActions[key].push_back(action);
    }

    void InputHandler::onAction(const std::string &action, std::function<void()> callback)
    {
        actionCallbacks[action].push_back(std::move(callback));
    }

    void InputHandler::fireAction(const std::string &action)
    {
        if (auto entry = actionCallbacks.find(action); entry != actionCallbacks.end())
        {
            for (const auto &callback : entry->second)
            {
                callback();
            }
        }
    }

    void InputHandler::registerButtonAction(const std::string &action, uint32_t button)
    {
        buttonActions[button].push_back(action);
    }

    void InputHandler::registerStickAxis(const std::string &axis, uint32_t stickAxis, bool inverted)
    {
        if (stickAxis >= GLFW_GAMEPAD_AXIS_LAST) return;
        stickAxes[axis].push_back({stickAxis, inverted});
    }

    void InputHandler::registerTriggerAxis(const std::string &axis, uint32_t negativeTrigger, uint32_t positiveTrigger)
    {
        if (negativeTrigger >= GLFW_GAMEPAD_BUTTON_LAST) return;
        if (positiveTrigger >= GLFW_GAMEPAD_BUTTON_LAST) return;
        triggerAxes[axis].push_back({negativeTrigger, positiveTrigger});
    }

    void InputHandler::registerKeyAxis(const std::string &axis, uint32_t negativeKey, uint32_t positiveKey)
    {
        if (negativeKey >= keyboardState.size()) return;
        if (positiveKey >= keyboardState.size()) return;
        keyboardAxes[axis].push_back({negativeKey, positiveKey});
    }

    double InputHandler::getAxis(const std::string &axis)
    {
        return axes[axis];
    }

    void InputHandler::update()
    {
        for (auto &[name, value] : axes)
        {
            value = 0.0;
        }

        for (const auto &[axis, mappings] : keyboardAxes)
        {
            for (auto [negativeKey, positiveKey] : mappings)
            {
                if (keyboardState[positiveKey]) axes[axis] += 1.0;
                if (keyboardState[negativeKey]) axes[axis] -= 1.0;
            }
        }

        GLFWgamepadstate gamepad = {};
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad)) {

            constexpr double stickDeadzone = 0.15;
            constexpr double gamepadTriggerDeadzone = 0.05;

            for (const auto &[axis, mappings] : stickAxes)
            {
                for (auto [stickAxis, inverted] : mappings)
                {
                    double value = gamepad.axes[stickAxis];
                    if (std::abs(value) < stickDeadzone) value = 0.0;
                    if (inverted) value = -value;
                    axes[axis] += value;
                }
            }

            auto readTrigger = [&](uint32_t button) -> double
            {
                double value = (gamepad.axes[button] + 1.0) / 2.0;
                return value < gamepadTriggerDeadzone ? 0.0 : value;
            };

            for (const auto &[axis, mappings] : triggerAxes)
            {
                for (auto [negativeTrigger, positiveTrigger] : mappings)
                {
                    axes[axis] += readTrigger(positiveTrigger) - readTrigger(negativeTrigger);
                }
            }
        }

            


        for (auto &[name, value] : axes)
        {
            value = std::clamp(value, -1.0, 1.0);
        }
    }

    void InputHandler::loadGamepadMappings(std::istream &mappings) 
    {
        std::ostringstream text;
        text << mappings.rdbuf();
        glfwUpdateGamepadMappings(text.str().c_str());
    }
}
