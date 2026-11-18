
#include "app/src/physics/waterphysics.h"
#include "engine/src/physics/physicsutils.h"

#include <algorithm>
#include <cmath>

//! Ballast Control
// Set ballst water level and limit it to 0-100%
void WaterPhysics::setWaterLevel(double percentage)
{
    waterLevel = std::clamp(percentage, 0.0, 100.0);
}

// Increase or decrease ballast water based on input and delta time
void WaterPhysics::changeWaterLevel(double input, double dt)
{
    if (dt <= 0.0) return;

    setWaterLevel(
        waterLevel + input * ballastChangeRate * dt
    );
}

//! Physics State Getters
// Return current ballast water level (0-100%)
double WaterPhysics::getWaterLevel() const
{
    return waterLevel;
}

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
    double buoyancy = calculateBuoyancy(dt);
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

//! Buoyancy

double WaterPhysics::calculateBuoyancy(double dt)
{
    // 0% = maximum upward buoyancy
    // 50% = neutral buoyancy
    // 100% = maximum downward buoyancy
    double targetBuoyancy =
        1.0 - 2.0 * (waterLevel / 100.0);

    // Smoothly adjust buoyancy
    double response = PhysicsUtils::calculateResponse(
        dt,
        buoyancyResponseTime
    );

    currentBuoyancy +=
        (targetBuoyancy - currentBuoyancy) * response;

    // Convert buoyancy to acceleration
    return currentBuoyancy * buoyancyAcceleration;
}

//! Manual Vertical Movement

double WaterPhysics::calculateManualAcceleration(double input, double dt)
{
    updateAccelerationProgress(input, dt);

    double curve = PhysicsUtils::calculateAccelerationCurve(
        accelerationProgress
    );

    return input * verticalAcceleration * curve;
}

//! Acceleration Ramp

void WaterPhysics::updateAccelerationProgress(double input, double dt)
{
    double direction = 0.0;

    if (input > 0.0) direction = 1.0;
    if (input < 0.0) direction = -1.0;

    // Reset acceleration ramp when input direction changes
    if (direction != previousDirection)
    {
        accelerationProgress = 0.0;
        previousDirection = direction;
    }

    // Gradually increase acceleration while input is active
    if (direction != 0.0)
    {
        accelerationProgress +=
            dt / std::max(accelerationRampTime, 0.001);

        accelerationProgress = std::clamp(
            accelerationProgress,
            0.0,
            1.0
        );
    }
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
        1.0 - std::abs(waterLevel - 50.0) / 10.0,
        0.0,
        1.0
    );

    // Smooth oscillating vertical drift
    return std::sin(elapsedTime * waterDriftFrequency)
        * waterDriftStrength
        * neutralFactor;
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
    return input * (
        input >= 0.0 ? forwardThrust : reverseThrust
    );
}
