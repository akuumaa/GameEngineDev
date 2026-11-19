#include "app/src/bubblescript.h"

#include "engine/src/math/vector3.h"
#include "engine/src/sim/entity.h"


namespace App
{
    void BubbleScript::update(Entity &entity, double dt, double t)
    {
        constexpr double riseSpeed = 0.5;
        constexpr double lifetime = 10.0;

        age += dt;

        if (age >= lifetime)
        {
            entity.requestUnload();
            return;
        }

        Vector3 position = entity.getPosition();
        position = position + Vector3::up * (riseSpeed * dt);
        entity.setPosition(position);
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