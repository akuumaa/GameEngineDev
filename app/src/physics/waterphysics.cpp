
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

    /** Calculate buoyany based on ballast water level
     * * 0% = +1.0 (maximum upward buoyancy)
     * * 50% =  0.0 (neutral buoyancy)
     * * 100% = -1.0 (maximum downward buoyancy)
     */
    double targetBuoyancy =
        1.0 - 2.0 * (waterLevel / 100.0);

    // Smoothly adjust buoyancy instead of applying it instantly
    double response =
        1.0 - std::exp(
                  -dt / std::max(buoyancyResponseTime, 0.001)
              );

    currentBuoyancy +=
        (targetBuoyancy - currentBuoyancy) * response;

    // Convert buoyancy into vertical acceleration
    double buoyancyForce = currentBuoyancy * buoyancyAcceleration;

    //! Manual vertical thrust
    // Determine input direction
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

        accelerationProgress =
            std::clamp(accelerationProgress, 0.0, 1.0);
    }

    // Quadratic acceleration curve for smooth movement
    double curve = accelerationProgress * accelerationProgress;

    // Calculate manual thrust acceleration
    double manualAcceleration = input 
        * verticalAcceleration 
        * curve;

    //! Water Resistance
    double drag = linearDrag 
        * verticalVelocity 
        + quadraticDrag 
        * verticalVelocity 
        * std::abs(verticalVelocity);

    //! Velocity Update
    // Combine buoyancy, manual thrust & water resistance
    double totalAcceleration = buoyancyForce 
        + manualAcceleration 
        - drag;

    // Integrate acceleration over delta time
    verticalVelocity += totalAcceleration * dt;

    // Limit vertical speed in both directions
    verticalVelocity = std::clamp(
        verticalVelocity,
        -maxVerticalSpeed,
        maxVerticalSpeed
    );

    // Prevent very small residual velocities
    if (std::abs(verticalVelocity) < 0.001 &&
        std::abs(totalAcceleration) < 0.001)
    {
        verticalVelocity = 0.0;
    }

    //! Water Drift Simulation

    // Apply subtle vertical dirft near neutral buoyancy (50%)
    double neutralFactor = std::clamp(
        1.0 - std::abs(waterLevel - 50.0) / 10.0,
        0.0,
        1.0
    );

    // Generate a smooth oscillating vertical drift
    double driftVelocity =
        std::sin(elapsedTime * waterDriftFrequency) * waterDriftStrength * neutralFactor;

    //! Vertical Displacement
    return (verticalVelocity + driftVelocity) * dt;
}
