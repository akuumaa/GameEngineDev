
#include "engine/src/sim/entity.h"
#include "engine/src/sim/script.h"

using Engine::Sim::Script;
using Engine::Sim::Entity;

#include <cstddef>

namespace App {
    class BubbleScript: public Script {
        public:
            void update(Entity &entity, double dt, double t) override;

            void load(Entity &entity) override;
            void unload(Entity &entity) override;

            static std::size_t getActiveCount();
        private:
            static std::size_t activeCount;
            double age = 0.0;
    };
}