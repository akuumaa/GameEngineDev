
#include "app/src/physics/waterphysics.h"

#include <algorithm>
#include <cmath>

double WaterPhysics::updateVertical(double input, double dt)
{
    if (dt <= 0.0)
    {
        return 0.0;
    }

    input = std::clamp(input, -1.0, 1.0);

    // 1. Eingaberichtung ermitteln
    double direction = 0.0;

    if (input > 0.0)
    {
        direction = 1.0;
    }
    else if (input < 0.0)
    {
        direction = -1.0;
    }

    // 2. Beschleunigungskurve zurücksetzen
    // wenn die Richtung geändert wird
    if (direction != previousDirection)
    {
        accelerationProgress = 0.0;
        previousDirection = direction;
    }

    // 3. Beschleunigung langsam aufbauen
    if (direction != 0.0)
    {
        accelerationProgress +=
            dt / std::max(accelerationRampTime, 0.001);

        accelerationProgress =
            std::clamp(accelerationProgress, 0.0, 1.0);
    }

    // 4. Quadratische Beschleunigungskurve
    double curve = accelerationProgress * accelerationProgress;

    double acceleration =
        input * verticalAcceleration * curve;

    // 5. Wasserwiderstand berechnen
    double drag =
        linearDrag * verticalVelocity
        + quadraticDrag * verticalVelocity
        * std::abs(verticalVelocity);

    // 6. Geschwindigkeit aktualisieren
    verticalVelocity += (acceleration - drag) * dt;

    // 7. Maximalgeschwindigkeit begrenzen
    verticalVelocity = std::clamp(
        verticalVelocity,
        -maxVerticalSpeed,
        maxVerticalSpeed
    );

    // 8. Verschiebung für diesen Frame zurückgeben
    return verticalVelocity * dt;
}

double WaterPhysics::getVerticalVelocity() const
{
    return verticalVelocity;
}
