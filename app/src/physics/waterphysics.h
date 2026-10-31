
#pragma once

class WaterPhysics
{
public:
    double maxVerticalSpeed = 5.0;
    double verticalAcceleration = 3.0;
    double accelerationRampTime = 2.0;

    double linearDrag = 0.25;
    double quadraticDrag = 0.08;

    // Auftrieb und Ballast
    double buoyancyAcceleration = 5.0;
    double ballastChangeRate = 25.0; // Prozent pro Sekunde
    double buoyancyResponseTime = 1.5;

    // Leichte Wasserbewegung bei neutralem Auftrieb
    double waterDriftStrength = 0.08;
    double waterDriftFrequency = 0.8;

    void setWaterLevel(double percentage);
    void changeWaterLevel(double input, double dt);

    double getWaterLevel() const;
    double getVerticalVelocity() const;

    double updateVertical(double input, double dt);

private:
    double waterLevel = 50.0;

    double verticalVelocity = 0.0;
    double accelerationProgress = 0.0;
    double previousDirection = 0.0;

    double currentBuoyancy = 0.0;
    double elapsedTime = 0.0;
};
