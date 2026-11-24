
#include "app/src/physics/waterphysics.h"

#include "engine/src/physics/physicsutils.h"

#include <algorithm>
#include <cmath>

//! Ballast Tank
App::BallastTank &WaterPhysics::getBallast()
{
    return ballast;
}

const App::BallastTank &WaterPhysics::getBallast() const
{
    return ballast;
}

//! Physics State Getters

// Return current vertical velocity (m/s)
double WaterPhysics::getVerticalVelocity() const
{
    return verticalVelocity;
}

double WaterPhysics::getForwardVelocity() const
{
    return forwardVelocity;
}

//! Vertical Physics Update
double WaterPhysics::updateVertical(double input, double dt)
{
    if (dt <= 0.0) return 0.0;

    elapsedTime += dt;

    input = std::clamp(input, -1.0, 1.0);

    // Calculate accelerations and resistance
    double buoyancy = ballast.updateBuoyancy(dt);
    double manualAcceleration = calculateManualAcceleration(input, dt);

    double drag = PhysicsUtils::calculateDrag(
        verticalVelocity,
        linearDrag,
        quadraticDrag
    );

    // Combine accelerations
    double totalAcceleration =
        buoyancy + manualAcceleration - drag;

    updateVerticalVelocity(totalAcceleration, dt);

    // Apply water drift to displacement
    double drift = calculateWaterDrift();

    return PhysicsUtils::calculateDisplacement(
        verticalVelocity + drift,
        dt
    );
}

//! Manual Vertical Movement

double WaterPhysics::calculateManualAcceleration(double input, double dt)
{
    return verticalRamp.update(input, dt) * verticalAcceleration;
}

//! Vertical Velocity Update

void WaterPhysics::updateVerticalVelocity(double acceleration, double dt)
{
    verticalVelocity = PhysicsUtils::integrateVelocity(
        verticalVelocity,
        acceleration,
        dt
    );

    verticalVelocity = PhysicsUtils::clampVelocity(
        verticalVelocity,
        maxVerticalSpeed
    );

    // Prevent small residual velocities
    if (std::abs(verticalVelocity) < 0.001 &&
        std::abs(acceleration) < 0.001)
    {
        verticalVelocity = 0.0;
    }
}

//! Water Drift Simulation

double WaterPhysics::calculateWaterDrift() const
{
    // Strongest drift near neutral buoyancy (50%)
    double neutralFactor = std::clamp(
        1.0 - std::abs(ballast.getLevel() - 50.0) / 10.0,
        0.0,
        1.0
    );

    // Smooth oscillating vertical drift
    return std::sin(elapsedTime * waterDriftFrequency) * waterDriftStrength * neutralFactor;
}

//! Forward Physics Update
double WaterPhysics::updateForward(double input, double dt)
{
    if (dt <= 0.0) return 0.0;

    input = std::clamp(input, -1.0, 1.0);

    // Calculate forward thrust
    double thrust = calculateForwardThrust(input);

    // Calculate water resistance
    double drag = PhysicsUtils::calculateDrag(
        forwardVelocity,
        forwardLinearDrag,
        forwardQuadraticDrag
    );

    // Update forward velocity
    forwardVelocity = PhysicsUtils::integrateVelocity(
        forwardVelocity,
        thrust - drag,
        dt
    );

    // Return forward displacement
    return PhysicsUtils::calculateDisplacement(
        forwardVelocity,
        dt
    );
}

//! Forward Thrust

double WaterPhysics::calculateForwardThrust(double input) const
{
    // Reverse thrust is weaker than forward thrust
    return input * (input >= 0.0 ? forwardThrust : reverseThrust);
}
