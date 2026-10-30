/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Tom Kertels, Enrico Leidner
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

#include "engine/src/core/inputhandler.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <istream>
#include <sstream>
#include <utility>

namespace Engine::Core
{
    /* BEGIN GENERAL FUNCTIONS */
    void InputHandler::update()
    {
        // Reset axes
        for (auto &[name, value] : axes)
        {
            value = 0.0;
        }

        // Keyboard Axis Updating
        for (const auto &[axis, mappings] : keyboardAxes)
        {
            for (auto [negativeKey, positiveKey] : mappings)
            {
                if (keyboardState[positiveKey]) axes[axis] += 1.0;
                if (keyboardState[negativeKey]) axes[axis] -= 1.0;
            }
        }

        GLFWgamepadstate gamepad = {};
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        {
            // Gamepad Axis Updating
            constexpr double gamepadDeadzone = 0.15;

            for (const auto &[axis, mappings] : gamepadAxes)
            {
                for (auto [input, invert] : mappings)
                {
                    if (input >= GLFW_GAMEPAD_AXIS_LAST + 1) continue;

                    double value = gamepad.axes[input];
                    if (std::abs(value) < gamepadDeadzone) continue;
                    if (invert) value = -value;

                    // Rescale so movement starts at 0 at the deadzone edge instead of jumping
                    value = std::copysign((std::abs(value) - gamepadDeadzone) / (1.0 - gamepadDeadzone), value);

                    axes[axis] += value;
                }
            }

            // Gamepad Button Updating
            for (const auto &[button, actions] : gamepadActions)
            {
                if (button >= GLFW_GAMEPAD_BUTTON_LAST + 1) continue;

                bool pressed = gamepad.buttons[button] == GLFW_PRESS;
                if (pressed != gamepadState[button])
                {
                    for (const auto &action : actions)
                    {
                        updateActionState(action, (uint64_t{3} << 32) | button, pressed);
                    }
                }

                gamepadState[button] = pressed;
            }
        }
        else
        {
            // Gamepad disconnected: release all held buttons
            for (size_t button = 0; button < gamepadState.size(); ++button)
            {
                if (!gamepadState[button]) continue;

                if (auto actions = gamepadActions.find(static_cast<uint32_t>(button)); actions != gamepadActions.end())
                {
                    for (const auto &action : actions->second)
                    {
                        updateActionState(action, (uint64_t{3} << 32) | button, false);
                    }
                }

                gamepadState[button] = false;
            }
        }

        // Clamp axes
        for (auto &[name, value] : axes)
        {
            value = std::clamp(value, -1.0, 1.0);
        }

        // Mouse Delta
        mouseDeltaX = mouseMovementX;
        mouseDeltaY = mouseMovementY;
        mouseDeltaZ = mouseMovementZ;

        mouseMovementX = 0.0;
        mouseMovementY = 0.0;
        mouseMovementZ = 0.0;
    }

    double InputHandler::getAxis(const std::string &axis)
    {
        return axes[axis];
    }

    /* BEGIN ACTION FUNCTIONS */
    void InputHandler::onAction(const std::string &action, std::function<void()> callback)
    {
        actionCallbacks[action].push_back(std::move(callback));
    }

    void InputHandler::onActionRelease(const std::string &action, std::function<void()> callback)
    {
        actionReleaseCallbacks[action].push_back(std::move(callback));
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

    void InputHandler::fireActionRelease(const std::string &action)
    {
        if (auto entry = actionReleaseCallbacks.find(action); entry != actionReleaseCallbacks.end())
        {
            for (const auto &callback : entry->second)
            {
                callback();
            }
        }
    }

    void InputHandler::updateActionState(const std::string &action, uint64_t source, bool pressed)
    {
        auto &sources = activeActionSources[action];

        if (pressed)
        {
            const bool sourceWasAdded = sources.insert(source).second;
            const bool isFirstActiveSource = sourceWasAdded && sources.size() == 1;

            if (isFirstActiveSource)
            {
                fireAction(action);
            }
        }
        else if (sources.erase(source) > 0 && sources.empty())
        {
            fireActionRelease(action);
        }
    }

    /* BEGIN KEYBOARD FUNCTIONS */
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

        if (auto actions = keyboardActions.find(key); actions != keyboardActions.end())
        {
            for (const auto &action : actions->second)
            {
                updateActionState(action, (uint64_t{1} << 32) | key, pressed);
            }
        }
    }

    void InputHandler::onKeyPress(uint32_t code, std::function<void()> callback)
    {
        keyCallbacks[code].push_back(std::move(callback));
    }

    void InputHandler::registerKeyAxis(const std::string &axis, uint32_t negativeKey, uint32_t positiveKey)
    {
        if (negativeKey >= keyboardState.size()) return;
        if (positiveKey >= keyboardState.size()) return;

        keyboardAxes[axis].push_back({negativeKey, positiveKey});
    }

    void InputHandler::registerKeyAction(const std::string &action, uint32_t key)
    {
        keyboardActions[key].push_back(action);
    }

    /* BEGIN GAMEPAD FUNCTIONS */
    void InputHandler::loadGamepadMappings(std::istream &mappings)
    {
        std::ostringstream text;
        text << mappings.rdbuf();
        glfwUpdateGamepadMappings(text.str().c_str());
    }

    void InputHandler::registerStickAxis(const std::string &axis, uint32_t input, bool invert)
    {
        gamepadAxes[axis].push_back({input, invert});
    }

    void InputHandler::registerButtonAction(const std::string &action, uint32_t button)
    {
        gamepadActions[button].push_back(action);
    }

    /* BEGIN MOUSE FUNCTIONS */
    void InputHandler::mouseButtonCallback(uint32_t button, bool pressed)
    {
        if (auto actions = mouseButtonActions.find(button); actions != mouseButtonActions.end())
        {
            for (const auto &action : actions->second)
            {
                updateActionState(action, (uint64_t{2} << 32) | button, pressed);
            }
        }
    }

    void InputHandler::registerMouseButtonAction(const std::string &action, uint32_t button)
    {
        mouseButtonActions[button].push_back(action);
    }

    void InputHandler::cursorPosCallback(double x, double y)
    {
        static double cursorX = 0.0;
        static double cursorY = 0.0;
        static bool cursorKnown = false;

        if (cursorKnown)
        {
            mouseMovementX += x - cursorX;
            mouseMovementY += y - cursorY;
        }

        cursorX = x;
        cursorY = y;
        cursorKnown = true;
    }

    void InputHandler::scrollCallback(double x, double y)
    {
        mouseMovementZ += y;
    }
}