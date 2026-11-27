#include "engine/src/physics/rigidbodyscript.h"

#include "engine/src/sim/entity.h"

namespace Engine::Physics
{
    RigidBodyScript::RigidBodyScript(
        std::shared_ptr<const PhysicsWorld> world,
        const BodyProperties &properties
    )
        : world(std::move(world)), body(properties)
    {
    }

    //! Physics Flow

    void RigidBodyScript::update(Entity &entity, double dt, double t)
    {
        if (dt <= 0.0 || !world) return;

        // Split the frame into equal sub-steps (same behavior at any FPS)
        const int steps = world->substepCount(dt);
        const double stepDt = dt / steps;

        for (int i = 0; i < steps; ++i)
        {
            const double stepTime = t - dt + stepDt * (i + 1);

            // 1. Own forces (engine, input, ballast)
            beforePhysics(entity, stepDt, stepTime);

            // 2. Environment + integration
            Vector3 position = entity.getPosition();
            Quaternion rotation = entity.getRotation();

            world->step(body, position, rotation, stepDt);

            // 3. Apply to entity
            entity.setPosition(position);
            entity.setRotation(rotation);
        }

        // 4. Once per frame
        afterPhysics(entity, dt, t);
    }

    //! Script Lifecycle

    void RigidBodyScript::load(Entity &entity)
    {
    }

    void RigidBodyScript::unload(Entity &entity)
    {
    }

    //! Access

    RigidBody &RigidBodyScript::getBody()
    {
        return body;
    }

    const RigidBody &RigidBodyScript::getBody() const
    {
        return body;
    }

    //! Hooks (default: do nothing)

    void RigidBodyScript::beforePhysics(Entity &entity, double dt, double t)
    {
    }

    void RigidBodyScript::afterPhysics(Entity &entity, double dt, double t)
    {
    }
}