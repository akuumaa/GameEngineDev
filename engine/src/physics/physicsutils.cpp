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
        return linearCoefficient * velocity + quadraticCoefficient * velocity * std::abs(velocity);
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

    double smoothstep(double progress)
    {
        const double t = std::clamp(progress, 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    }

    double moveTowards(double current, double target, double maxDelta)
    {
        if (std::abs(target - current) <= maxDelta) return target;
        return current + (target > current ? maxDelta : -maxDelta);
    }

    //! Vector3 versions

    Vector3 calculateDrag(
        const Vector3 &velocity,
        const Vector3 &linearCoefficient,
        const Vector3 &quadraticCoefficient
    )
    {
        return {
            calculateDrag(velocity.x, linearCoefficient.x, quadraticCoefficient.x),
            calculateDrag(velocity.y, linearCoefficient.y, quadraticCoefficient.y),
            calculateDrag(velocity.z, linearCoefficient.z, quadraticCoefficient.z)
        };
    }

    Vector3 integrateVelocity(
        const Vector3 &velocity,
        const Vector3 &acceleration,
        double dt
    )
    {
        return {
            integrateVelocity(velocity.x, acceleration.x, dt),
            integrateVelocity(velocity.y, acceleration.y, dt),
            integrateVelocity(velocity.z, acceleration.z, dt)
        };
    }

    Vector3 clampVelocity(const Vector3 &velocity, const Vector3 &maxSpeed)
    {
        // <= 0 means: no limit on this axis
        auto clampAxis = [](double v, double max)
        {
            return max > 0.0 ? clampVelocity(v, max) : v;
        };

        return {
            clampAxis(velocity.x, maxSpeed.x),
            clampAxis(velocity.y, maxSpeed.y),
            clampAxis(velocity.z, maxSpeed.z)
        };
    }

    Vector3 calculateDisplacement(const Vector3 &velocity, double dt)
    {
        return {
            calculateDisplacement(velocity.x, dt),
            calculateDisplacement(velocity.y, dt),
            calculateDisplacement(velocity.z, dt)
        };
    }

    //! Vector helpers

    Vector3 multiply(const Vector3 &a, const Vector3 &b)
    {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    Vector3 cross(const Vector3 &a, const Vector3 &b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    Vector3 normalize(const Vector3 &v)
    {
        const double length = v.length();
        if (length < 1e-12) return {};
        return v * (1.0 / length);
    }
}