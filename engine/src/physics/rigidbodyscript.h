#pragma once

#include "engine/src/physics/physicsworld.h"
#include "engine/src/physics/rigidbody.h"
#include "engine/src/sim/script.h"

#include <memory>

using Engine::Sim::Entity;
using Engine::Sim::Script;

namespace Engine::Physics
{
    /**
     * Rigid Body Script - gives ANY entity physics:
     *     entity->addScript(std::make_unique<RigidBodyScript>(world, properties));
     *
     * update() is the same for everyone (template method). Per sub-step:
     *   1. beforePhysics(entity, dt, t)  ← own forces (engine, input, ballast)
     *   2. world->step(...)              ← gravity, buoyancy, drag + integration
     *   3. entity.setPosition / setRotation
     * Then once per frame:
     *   4. afterPhysics(entity, frameDt, t) ← debug output, camera
     *
     * Subclasses (e.g. ShipScript) only override the hooks.
     */
    class RigidBodyScript : public Script
    {
      public:
        explicit RigidBodyScript(
            std::shared_ptr<const PhysicsWorld> world,
            const BodyProperties &properties = {}
        );

        // final: the physics flow is the same for all bodies
        void update(Entity &entity, double dt, double t) final;

        void load(Entity &entity) override;
        void unload(Entity &entity) override;

        RigidBody &getBody();
        const RigidBody &getBody() const;

      protected:
        //! Hooks
        // dt = length of the sub-step
        virtual void beforePhysics(Entity &entity, double dt, double t);

        // dt = length of the whole frame
        virtual void afterPhysics(Entity &entity, double dt, double t);

      private:
        std::shared_ptr<const PhysicsWorld> world; // shared: lives as long as any script needs it
        RigidBody body;
    };
}