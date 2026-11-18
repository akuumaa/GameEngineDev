#pragma once

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
}