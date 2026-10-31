
#pragma once

class WaterPhysics
{
public:
    // Geschwindigkeit in m/s
    double maxVerticalSpeed = 5.0;

    // Maximale Beschleunigung in m/s²
    double verticalAcceleration = 3.0;

    // Zeit bis zur vollen Beschleunigung
    double accelerationRampTime = 2.0;

    // Wasserwiderstand
    double linearDrag = 0.25;
    double quadraticDrag = 0.08;

    // Berechnet die vertikale Verschiebung pro Frame
    double updateVertical(double input, double dt);

    // Aktuelle vertikale Geschwindigkeit
    double getVerticalVelocity() const;

private:
    double verticalVelocity = 0.0;
    double accelerationProgress = 0.0;
    double previousDirection = 0.0;
};
