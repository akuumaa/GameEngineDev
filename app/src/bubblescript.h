#pragma once

#include "engine/src/math/random.h"
#include "engine/src/math/vector3.h"
#include "engine/src/physics/physicsworld.h"
#include "engine/src/physics/rigidbodyscript.h"
#include "engine/src/sim/entity.h"

#include <cstdint>
#include <memory>

using Engine::Math::Random;
using Engine::Math::Vector3;
using Engine::Physics::BodyProperties;
using Engine::Physics::PhysicsWorld;
using Engine::Physics::RigidBodyScript;
using Engine::Sim::Entity;

namespace App
{
    /// Air bubble (idea from Alexander's feat/bubbles):
    /// rises by buoyancy, wobbles, and at the surface goes back to its air stone (pooling)
    class BubbleScript : public RigidBodyScript
    {
      public:
        BubbleScript(std::shared_ptr<const PhysicsWorld> world, const Vector3 &bubbler,
                     double surfaceHeight, uint32_t seed, double jitter);

      protected:
        void beforePhysics(Entity &entity, double dt, double t) override;   // wobble
        void afterPhysics(Entity &entity, double dt, double t) override;    // reset at surface

      private:
        static BodyProperties bubbleBody();

        Vector3 bubbler = {};      // where this bubble starts again
        double surfaceHeight = 0.0;
        double jitter = 0.0;
        double phase = 0.0;        // random start → bubbles don't wobble in sync
        double age = 0.0;
        Random random;             // own random stream per bubble (from world seed)
    };
}
