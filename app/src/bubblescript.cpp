#include "app/src/bubblescript.h"

#include <cmath>

namespace App
{
    BubbleScript::BubbleScript(std::shared_ptr<const PhysicsWorld> world, const Vector3 &bubbler,
                               double surfaceHeight, uint32_t seed, double jitter)
        : RigidBodyScript(std::move(world), bubbleBody()),
          bubbler(bubbler), surfaceHeight(surfaceHeight), jitter(jitter), random(seed)
    {
        phase = random.nextDouble(0.0, 6.28);
    }

    BodyProperties BubbleScript::bubbleBody()
    {
        BodyProperties body;
        body.buoyancyFactor = 2.0;           // twice gravity → rises
        body.linearDrag = {0.2, 0.2, 0.2};   // top speed ≈ 25 units/s
        body.quadraticDrag = {0.0, 0.0, 0.0};
        return body;
    }

    void BubbleScript::beforePhysics(Entity &entity, double dt, double t)
    {
        // Small sideways wobble while rising
        getBody().addAcceleration({
            std::sin(age * 4.0 + phase) * 3.0,
            std::cos(age * 3.0 + phase) * 3.0,
            0.0
        });
    }

    void BubbleScript::afterPhysics(Entity &entity, double dt, double t)
    {
        age += dt;
        if (entity.getPosition().z < surfaceHeight - 1.0) return;

        // Reached the surface → back to the air stone instead of being deleted (pooling)
        entity.setPosition({
            bubbler.x + random.nextDouble(-jitter, jitter),
            bubbler.y + random.nextDouble(-jitter, jitter),
            bubbler.z
        });
        getBody().setVelocity({});            // otherwise it keeps its rising speed
        phase = random.nextDouble(0.0, 6.28);
    }
}
