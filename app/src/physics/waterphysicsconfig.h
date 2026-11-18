#pragma once

struct WaterPhysicsConfig
{
    //! Vertical Movement
    double maxVerticalSpeed = 5.0;
    double verticalAcceleration = 3.0;
    double accelerationRampTime = 2.0;

    //! Vertical Water Resistance
    double linearDrag = 0.25;
    double quadraticDrag = 0.08;

    //! Forward Movement
    double forwardThrust = 6.96;
    double reverseThrust = 3.0;

    //! Forward Water Resistance
    double forwardLinearDrag = 0.1;
    double forwardQuadraticDrag = 0.04;

    //! Buoyancy & Ballast
    double buoyancyAcceleration = 5.0;
    double ballastChangeRate = 25.0;
    double buoyancyResponseTime = 1.5;

    //! Water Drift
    double waterDriftStrength = 0.08;
    double waterDriftFrequency = 0.8;
};