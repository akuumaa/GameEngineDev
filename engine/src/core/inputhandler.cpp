/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright Â© 2021-2026 Tobias Reimann
 * Copyright © 2026 Yannick Kissel
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

#include "engine/src/core/inputhandler.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <istream>
#include <sstream>
#include <utility>

namespace Engine::Core
{
    void InputHandler::keyCallback(uint32_t key, uint32_t scancode, uint32_t mods, bool pressed)
    {
        if (key < keyboardState.size()) keyboardState[key] = pressed;

        if (auto actions = keyboardActions.find(key); pressed && actions != keyboardActions.end())
        {
            for (const auto &action : actions->second) fireAction(action);
        }

        if (auto entry = keyCallbacks.find(key); pressed && entry != keyCallbacks.end())
        {
            for (const auto &callback : entry->second)
            {
                callback();
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
            for (const auto &callback : entry->second) callback();
        }
    }

    void InputHandler::registerKeyAxis(const std::string &axis, uint32_t negativeKey, uint32_t positiveKey)
    {
        if (negativeKey >= keyboardState.size() || positiveKey >= keyboardState.size()) return;
        keyboardAxes[axis].push_back({negativeKey, positiveKey});
    }

    double InputHandler::getAxis(const std::string &axis)
    {
        auto entry = axes.find(axis);
        return entry != axes.end() ? entry->second : 0.0;
    }

    void InputHandler::registerButtonAction(const std::string &action, uint32_t button)
    {
        if (button >= gamepadState.size()) return;
        gamepadActions[button].push_back(action);
    }

    void InputHandler::registerStickAxis(const std::string &axis, uint32_t input, bool invert)
    {
        if (input > GLFW_GAMEPAD_AXIS_LAST) return;
        gamepadAxes[axis].push_back({input, invert});
    }

    void InputHandler::loadGamepadMappings(std::istream &mappings)
    {
        std::ostringstream text;
        text << mappings.rdbuf();
        glfwUpdateGamepadMappings(text.str().c_str());
    }

    void InputHandler::cursorPosCallback(double x, double y)
    {
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

    void InputHandler::resetMouse()
    {
        cursorKnown = false;
        mouseMovementX = mouseMovementY = mouseMovementZ = 0.0;
        mouseDeltaX = mouseDeltaY = mouseDeltaZ = 0.0;
    }

    void InputHandler::clearKeyboardState()
    {
        keyboardState.fill(false);
        resetMouse();
    }

    void InputHandler::update()
    {
        for (auto &[name, value] : axes) value = 0.0;

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
            constexpr double gamepadDeadzone = 0.15;
            for (const auto &[axis, mappings] : gamepadAxes)
            {
                for (auto [input, invert] : mappings)
                {
                    double value = gamepad.axes[input];
                    double magnitude = std::abs(value);
                    value = magnitude <= gamepadDeadzone ? 0.0
                        : std::copysign((magnitude - gamepadDeadzone) / (1.0 - gamepadDeadzone), value);
                    axes[axis] += invert ? -value : value;
                }
            }

            for (uint32_t button = 0; button < gamepadState.size(); ++button)
            {
                bool pressed = gamepad.buttons[button] == GLFW_PRESS;
                bool wasPressed = gamepadState[button];
                gamepadState[button] = pressed;
                if (auto actions = gamepadActions.find(button); pressed && !wasPressed && actions != gamepadActions.end())
                {
                    for (const auto &action : actions->second) fireAction(action);
                }
            }
        }
        else
        {
            gamepadState.fill(false);
        }

        for (auto &[name, value] : axes) value = std::clamp(value, -1.0, 1.0);
        mouseDeltaX = std::exchange(mouseMovementX, 0.0);
        mouseDeltaY = std::exchange(mouseMovementY, 0.0);
        mouseDeltaZ = std::exchange(mouseMovementZ, 0.0);
    }
}
