#include "app/src/physics/ballasttank.h"

#include "engine/src/physics/physicsutils.h"

#include <algorithm>

namespace App
{
    BallastTank::BallastTank(const BallastSettings &settings)
        : settings(settings)
    {
    }

    //! Ballast Control
    void BallastTank::setLevel(double percentage)
    {
        level = std::clamp(percentage, 0.0, 100.0);
    }

    void BallastTank::changeLevel(double input, double dt)
    {
        if (dt <= 0.0) return;

        setLevel(
            level + input * settings.changeRate * dt
        );
    }

    void BallastTank::reset()
    {
        setLevel(50.0);
    }

    //! Buoyancy
    double BallastTank::updateBuoyancy(double dt)
    {
        // 0% = maximum upward buoyancy
        // 50% = neutral buoyancy
        // 100% = maximum downward buoyancy
        double targetBuoyancy =
            1.0 - 2.0 * (level / 100.0);

        // Smoothly adjust buoyancy
        double response = PhysicsUtils::calculateResponse(
            dt,
            settings.responseTime
        );

        currentBuoyancy +=
            (targetBuoyancy - currentBuoyancy) * response;

        // Convert buoyancy to acceleration
        return currentBuoyancy * settings.buoyancyAcceleration;
    }

    //! State
    double BallastTank::getLevel() const
    {
        return level;
    }
}