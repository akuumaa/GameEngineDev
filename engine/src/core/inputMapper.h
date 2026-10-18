
#include <istream>
#include <json.hpp>
#include <json.hpp>

namespace Engine::Core
{
    class InputMapper
    {
      public:
        InputMapper() = delete;

        static void loadMappings(const std::string &filename);
        static void loadMappings(std::istream &mappings);

      private:
        static void loadActions(const nlohmann::json &actions);
        static void loadAxes(const nlohmann::json &axes);
    };
}