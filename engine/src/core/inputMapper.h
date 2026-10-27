/*
 * Copyright © 2026 Tom Kertels
 */

#pragma once

#include <istream>
#include <string>

namespace Engine::Core
{
    class InputMapper
    {
      public:
        InputMapper() = delete;

        // Call once per input setup; registrations accumulate in InputHandler.
        static bool loadMappings(const std::string &filename);
        static bool loadMappings(std::istream &mappings);
    };
}