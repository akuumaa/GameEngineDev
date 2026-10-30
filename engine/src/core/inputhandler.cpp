/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Alexander Hancharenka
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
        /**tasten druck und loslassen callback */
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

        if (auto entry = keyboardActions.find(key); pressed && entry != keyboardActions.end())
        {
            for (const auto &action : entry->second)
            {
                fireAction(action);
            }
        }
    }

    void InputHandler::registerKeyAxis(const std::string &axis, uint32_t negativeKey, uint32_t positiveKey)
    {
        if (negativeKey >= keyboardState.size() || positiveKey >= keyboardState.size())
        {
            return;
        }
        keyboardAxes[axis].push_back({negativeKey, positiveKey});
    }

    double InputHandler::getAxis(const std::string &axis)
    {
        return axes[axis];
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

    void InputHandler::update()
    {
        for (auto &[name,value] : axes)
        {   
            /** achsen auf 0 zurücksetzen */
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
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        {
            constexpr double gamepadDeadzone = 0.15;

            for (const auto &[axis, mappings] : gamepadAxes)
            {
                for (auto [input, invert] : mappings)
                {
                    double value = gamepad.axes[input];

                    double magnitude = std::abs(value);
                    if (magnitude < gamepadDeadzone)
                    {
                        value = 0.0;
                    }
                    else
                    {
                        double sign = value < 0.0 ? -1.0 : 1.0;
                        value = sign * (magnitude - gamepadDeadzone) / (1.0 - gamepadDeadzone);
                    }

                    if (invert) value = -value;

                    axes[axis] += value;
                }
            }

            for (uint32_t button = 0; button < gamepadState.size(); button++)
            {
                bool pressed = gamepad.buttons[button] == GLFW_PRESS;

                if (pressed && !gamepadState[button])
                {
                    if (auto actions = gamepadActions.find(button); actions != gamepadActions.end())
                    {
                        for (const auto &action : actions->second)
                        {
                            fireAction(action);
                        }
                    }
                }

                gamepadState[button] = pressed;
            }
        }

        /**begrenzung auf -1 und 1 */
        for (auto &[name, value] : axes)
        {
            /** clamp zwingt einen Wert in einen Bereich aus algorithm
             * if (value < min) return min;
             * if (value > max) return max;
             * return value
            */
            value = std::clamp(value, -1.0, 1.0);
        }

        mouseDeltaX = mouseMovementX;
        mouseDeltaY = mouseMovementY;
        mouseDeltaZ = mouseMovementZ;

        mouseMovementX = 0.0;
        mouseMovementY = 0.0;
        mouseMovementZ = 0.0;
    }

    void InputHandler::registerButtonAction(const std::string &action, uint32_t button)
    {
        gamepadActions[button].push_back(action);
    }

    void InputHandler::registerStickAxis(const std::string &axis, uint32_t input, bool invert)
    {
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
