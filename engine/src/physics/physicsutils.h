#pragma once

#include "engine/src/math/vector3.h"

using Engine::Math::Vector3;

namespace PhysicsUtils
{
    // Calculate linear and quadratic drag
    double calculateDrag(
        double velocity,
        double linearCoefficient,
        double quadraticCoefficient
    );

    // Integrate acceleration into velocity
    double integrateVelocity(
        double velocity,
        double acceleration,
        double dt
    );

    // Limit velocity to a symmetric range
    double clampVelocity(double velocity, double maxSpeed);

    // Calculate displacement from velocity
    double calculateDisplacement(double velocity, double dt);

    // Calculate exponential response factor
    double calculateResponse(double dt, double responseTime);

    // Calculate quadratic acceleration curve
    double calculateAccelerationCurve(double progress);

    // Smooth ease-in-out curve 0..1 (3t² - 2t³)
    double smoothstep(double progress);

    // Move current towards target by at most maxDelta
    double moveTowards(double current, double target, double maxDelta);

    //! Vector3 versions (call the scalar versions per axis)

    // Drag per axis, each axis with its own coefficients
    Vector3 calculateDrag(
        const Vector3 &velocity,
        const Vector3 &linearCoefficient,
        const Vector3 &quadraticCoefficient
    );

    Vector3 integrateVelocity(
        const Vector3 &velocity,
        const Vector3 &acceleration,
        double dt
    );

    // Limit per axis, component <= 0 = no limit on that axis
    Vector3 clampVelocity(const Vector3 &velocity, const Vector3 &maxSpeed);

    Vector3 calculateDisplacement(const Vector3 &velocity, double dt);

    //! Vector helpers

    // Component-wise product
    Vector3 multiply(const Vector3 &a, const Vector3 &b);

    Vector3 cross(const Vector3 &a, const Vector3 &b);

    // Unit vector, zero vector stays zero
    Vector3 normalize(const Vector3 &v);
}