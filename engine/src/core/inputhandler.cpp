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

#include <cstdint>
#include <cstdlib>
#include <string>
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <istream>
#include <sstream>

#include "engine/src/core/inputhandler.h"

#include <utility>
#include <algorithm>
#include <cmath>


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

    void InputHandler::mouseButtonCallback(uint32_t button, bool pressed, uint32_t mods)
    {
        if (button < mouseButtonState.size()) 
        {
            mouseButtonState[button] = pressed;
        }

        if (auto entry = mouseButtonCallbacks.find(button); pressed && entry != mouseButtonCallbacks.end())
        {
            for (const auto &callback : entry->second) 
            {
                callback();
            }
        }

        if (auto actions = mouseActions.find(button); pressed && actions != mouseActions.end())
        {
            for (const auto &action : actions->second) 
            {
                fireAction(action);
            }
        }
    }

    void InputHandler:: mouseCursorCallback(double xpos, double ypos)
    {
        if (!mouseCursorPositions.isInitialized) {
            mouseCursorPositions.previous = {xpos, ypos};
            mouseCursorPositions.isInitialized = true;
        }
        mouseCursorPositions.current = {xpos, ypos};
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
        if (button >= gamepadState.size()) return;
        gamepadActions[button].push_back(action);
    }

    void InputHandler::registerStickAxis(const std::string &axis, uint32_t input, bool inverted)
    {
        if (input > GLFW_GAMEPAD_AXIS_LAST) return;
        gamepadAxes[axis].push_back({input, inverted});
    }

    void InputHandler::registerMouseAxis(const std::string &axis, uint32_t input, bool inverted)
    {
        if (input >= mouseCursorPositions.current.size()) return;
         mouseAxes[axis].push_back({input, inverted});
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
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        {
            constexpr double gamepadDeadzone = 0.15;
            for ( const auto &[axis, mappings]: gamepadAxes)
            {
                for (auto [index, inverted]: mappings)
                {
                    float stickValue = gamepad.axes[index];
                    
                    if (std::abs(stickValue) >= gamepadDeadzone) {
                        float invertedCheckStickValue = (inverted) ? -stickValue : stickValue;

                        float valueWithDeadzone = std::copysign(1.0, invertedCheckStickValue) * ((std::abs(invertedCheckStickValue) - gamepadDeadzone) / (1 - gamepadDeadzone));
                        axes[axis] += valueWithDeadzone;
                    }
                }
            }

            for (const auto &[button, actions]: gamepadActions) {
                bool currentState = gamepad.buttons[button];
                bool storedState = gamepadState[button];

                gamepadState[button] = currentState;

                if (currentState && !storedState)
                {
                    for (const auto &action : actions)
                    {
                        fireAction(action);
                    }
                }
            }
        }

        for (const auto &[axis, mappings]: mouseAxes)
        {
            for (auto [index, inverted]: mappings)
            {
                float mouseMovement = (mouseCursorPositions.current[index] - mouseCursorPositions.previous[index]) / 10.0;
                float invertedMouseMovement = (inverted) ? -mouseMovement : mouseMovement;

                axes[axis] += invertedMouseMovement;

            }
        }
        mouseCursorPositions.previous = mouseCursorPositions.current;
        // TODO Mouse Button Loop


        for (auto &[name, value] : axes)
        {
            value = std::clamp(value, -1.0, 1.0);
        }
    }
    
    void InputHandler::onKeyPress(uint32_t code, std::function<void()> callback)
    {
        keyCallbacks[code].push_back(std::move(callback));
    }

    void InputHandler::loadGamepadMappings(std::istream &mappings)
    {
        std::ostringstream text;
        text << mappings.rdbuf();
        glfwUpdateGamepadMappings(text.str().c_str());
    }
}
