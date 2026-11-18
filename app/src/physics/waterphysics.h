
#pragma once

#include "app/src/physics/waterphysicsconfig.h"

/**
 * Water Physics - Buoyancy, Ballast,
 * Vertical & Forward Movement
 */
class WaterPhysics : public WaterPhysicsConfig
{
public:
    //! Ballast Control
    void setWaterLevel(double percentage);
    void changeWaterLevel(double input, double dt);

    //! Vertical Movement
    double updateVertical(double input, double dt);
    double getVerticalVelocity() const;

    //! Forward Movement
    double updateForward(double input, double dt);
    double getForwardVelocity() const;

    //! Ballast State
    double getWaterLevel() const;

private:
    //! Buoyancy
    double calculateBuoyancy(double dt);

    //! Manual Vertical Movement
    double calculateManualAcceleration(double input, double dt);
    void updateAccelerationProgress(double input, double dt);

    //! Vertical Velocity
    void updateVerticalVelocity(double acceleration, double dt);

    //! Water Drift
    double calculateWaterDrift() const;

    //! Forward Movement
    double calculateForwardThrust(double input) const;

    //! Ballast State
    double waterLevel = 50.0;
    double currentBuoyancy = 0.0;

    //! Vertical Movement State
    double verticalVelocity = 0.0;
    double accelerationProgress = 0.0;
    double previousDirection = 0.0;

    //! Forward Movement State
    double forwardVelocity = 0.0;

    //! Simulation Time
    double elapsedTime = 0.0;
};
