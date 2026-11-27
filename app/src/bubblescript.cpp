#include "app/src/bubblescript.h"


namespace App
{

    BubbleScript::BubbleScript(std::shared_ptr<const PhysicsWorld> world)
        : RigidBodyScript(std::move(world))
    {
        getBody().getProperties().buoyancyFactor = 1.1;
    }

    void BubbleScript::afterPhysics(Entity &entity, double dt, double t)
    {
        constexpr double lifetime = 10.0;

        age += dt;

        if (age >= lifetime)
        {
            entity.requestUnload();
            return;
        }
        
    }

    std::size_t BubbleScript::activeCount = 0;

    void BubbleScript::load(Entity &)
    {
        ++activeCount;
    }

    void BubbleScript::unload(Entity &)
    {
        --activeCount;
    }

    std::size_t BubbleScript::getActiveCount()
    {
        return activeCount;
    }
}