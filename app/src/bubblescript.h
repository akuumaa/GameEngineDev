
#include "engine/src/sim/entity.h"
#include "engine/src/sim/script.h"
#include "engine/src/physics/rigidbodyscript.h"
#include "engine/src/physics/physicsworld.h"
#include <memory>

using Engine::Sim::Script;
using Engine::Sim::Entity;
using Engine::Physics::RigidBodyScript;
using Engine::Physics::PhysicsWorld;

#include <cstddef>

namespace App {
    class BubbleScript: public RigidBodyScript {
        public:
            void load(Entity &entity) override;
            void unload(Entity &entity) override;

            static std::size_t getActiveCount();

            explicit BubbleScript(std::shared_ptr<const PhysicsWorld> world);

        protected:
        void afterPhysics(Entity &entity, double dt, double t) override;
        
        private:
            static std::size_t activeCount;
            double age = 0.0;
    };
}