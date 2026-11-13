
#include "app/src/physics/waterphysics.h"

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

//! Vertical Physics Update
double WaterPhysics::updateVertical(double input, double dt)
{
    if (dt <= 0.0) return 0.0;

    elapsedTime += dt;
    input = std::clamp(input, -1.0, 1.0);

    const double totalAcceleration =
        updateBuoyancyForce(dt)
        + updateManualVerticalAcceleration(input, dt);

    updateVerticalVelocity(totalAcceleration, dt);

    const double neutralFactor = std::clamp(
        1.0 - std::abs(waterLevel - 50.0) / 10.0,
        0.0,
        1.0
    );

    const double driftVelocity =
        std::sin(elapsedTime * waterDriftFrequency)
        * waterDriftStrength
        * neutralFactor;

    return (verticalVelocity + driftVelocity) * dt;
}

double WaterPhysics::updateBuoyancyForce(double dt)
{
    // 0 % water means maximum upward buoyancy, 100 % maximum downward buoyancy.
    const double targetBuoyancy =
        1.0 - 2.0 * (waterLevel / 100.0);

    // Smoothly approach the target instead of changing buoyancy instantly.
    const double response =
        1.0 - std::exp(
                  -dt / std::max(buoyancyResponseTime, 0.001)
              );

    currentBuoyancy +=
        (targetBuoyancy - currentBuoyancy) * response;

    return currentBuoyancy * buoyancyAcceleration;
}

double WaterPhysics::updateManualVerticalAcceleration(double input, double dt)
{
    double direction = 0.0;
    if (input > 0.0) direction = 1.0;
    if (input < 0.0) direction = -1.0;

    // Restart the acceleration ramp when input changes direction or stops.
    if (direction != previousDirection)
    {
        accelerationProgress = 0.0;
        previousDirection = direction;
    }

    if (direction != 0.0)
    {
        accelerationProgress +=
            dt / std::max(accelerationRampTime, 0.001);

        accelerationProgress =
            std::clamp(accelerationProgress, 0.0, 1.0);
    }

    const double curve = accelerationProgress * accelerationProgress;
    return input * verticalAcceleration * curve;
}

void WaterPhysics::updateVerticalVelocity(double acceleration, double dt)
{
    const double drag =
        linearDrag * verticalVelocity
        + quadraticDrag * verticalVelocity * std::abs(verticalVelocity);

    const double totalAcceleration = acceleration - drag;

    verticalVelocity += totalAcceleration * dt;

    verticalVelocity = std::clamp(
        verticalVelocity,
        -maxVerticalSpeed,
        maxVerticalSpeed
    );

    if (std::abs(verticalVelocity) < 0.001 &&
        std::abs(totalAcceleration) < 0.001)
    {
        verticalVelocity = 0.0;
    }
}

// Return current forward velocity (m/s)
double WaterPhysics::getForwardVelocity() const
{
    return forwardVelocity;
}

//! Forward Physics Update
double WaterPhysics::updateForward(double input, double dt)
{
    if (dt <= 0.0) return 0.0;

    input = std::clamp(input, -1.0, 1.0);

    // Use weaker thrust when moving backwards
    double thrust = input * (input >= 0.0 ? forwardThrust : reverseThrust);

    //! Water Resistance
    // v * |v| keeps the drag opposite to the direction of movement
    double drag = forwardLinearDrag * forwardVelocity
                + forwardQuadraticDrag * forwardVelocity * std::abs(forwardVelocity);

    //! Velocity Update
    // No clamp: thrust and drag balance out at terminal velocity
    forwardVelocity += (thrust - drag) * dt;

    //! Forward Displacement
    return forwardVelocity * dt;
}
