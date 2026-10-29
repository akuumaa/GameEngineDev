/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright � 2026 Yannick Kissel
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

#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>
#include <functional>
#include <unordered_map>
#include <vector>

/**
 * Deklaration der Input-Handler-Klassen. Aktionen, Achsen, Tastenzustände, Controller und Maus 
 */
namespace Engine::Core
{
    class InputHandler
    {
      public:
        InputHandler() = delete;

        static void keyCallback(uint32_t key, uint32_t scancode, uint32_t mods, bool pressed);
        static void onKeyPress(uint32_t code, std::function<void()> callback);

        static void registerKeyAction(const std::string &action, uint32_t key);
        static void onAction(const std::string &action, std::function<void()> callback);
        static void fireAction(const std::string &action);
        static void registerKeyAxis(const std::string &axis, uint32_t negativeKey, uint32_t positiveKey);
        static double getAxis(const std::string &axis);
        static void update();
        static void cursorPosCallback(double x, double y);
        static void scrollCallback(double x, double y);
        static double getMouseDeltaX() { return mouseDeltaX; }
        static double getMouseDeltaY() { return mouseDeltaY; }
        static double getMouseDeltaZ() { return mouseDeltaZ; }
        static void resetMouse();
        static void clearKeyboardState();
        static void registerButtonAction(const std::string &action, uint32_t button);
        static void registerStickAxis(const std::string &axis, uint32_t input, bool invert = false);
        static void loadGamepadMappings(std::istream &mappings);

      private:
        static inline double mouseMovementX = 0.0;
        static inline double mouseMovementY = 0.0;
        static inline double mouseMovementZ = 0.0;
        static inline double mouseDeltaX = 0.0;
        static inline double mouseDeltaY = 0.0;
        static inline double mouseDeltaZ = 0.0;
        static inline double cursorX = 0.0;
        static inline double cursorY = 0.0;
        static inline bool cursorKnown = false;
        static inline std::unordered_map<std::string, std::vector<std::function<void()>>> actionCallbacks = {};
        static inline std::unordered_map<uint32_t, std::vector<std::string>> keyboardActions = {};
        static inline std::unordered_map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> keyboardAxes = {};
        static inline std::array<bool, 349> keyboardState = {};
        static inline std::unordered_map<std::string, double> axes = {};
        static inline std::unordered_map<uint32_t, std::vector<std::string>> gamepadActions = {};
        static inline std::unordered_map<std::string, std::vector<std::pair<uint32_t, bool>>> gamepadAxes = {};
        static inline std::array<bool, 15> gamepadState = {};

        static inline std::unordered_map<uint32_t, std::vector<std::function<void()>>> keyCallbacks = {};
    };
}
