#include "engine/src/physics/physicsutils.h"

#include <algorithm>
#include <cmath>

namespace PhysicsUtils
{
    double calculateDrag(
        double velocity,
        double linearCoefficient,
        double quadraticCoefficient
    )
    {
        return linearCoefficient * velocity
            + quadraticCoefficient * velocity * std::abs(velocity);
    }

    double integrateVelocity(
        double velocity,
        double acceleration,
        double dt
    )
    {
        return velocity + acceleration * dt;
    }

    double clampVelocity(double velocity, double maxSpeed)
    {
        return std::clamp(velocity, -maxSpeed, maxSpeed);
    }

    double calculateDisplacement(double velocity, double dt)
    {
        return velocity * dt;
    }

    double calculateResponse(double dt, double responseTime)
    {
        return 1.0 - std::exp(
            -dt / std::max(responseTime, 0.001)
        );
    }

    double calculateAccelerationCurve(double progress)
    {
        return progress * progress;
    }
}