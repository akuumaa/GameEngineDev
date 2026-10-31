
#include "app/src/physics/waterphysics.h"

#include <algorithm>
#include <cmath>

void WaterPhysics::setWaterLevel(double percentage)
{
    waterLevel = std::clamp(percentage, 0.0, 100.0);
}

void WaterPhysics::changeWaterLevel(double input, double dt)
{
    if (dt <= 0.0) return;

    setWaterLevel(
        waterLevel + input * ballastChangeRate * dt
    );
}

double WaterPhysics::getWaterLevel() const
{
    return waterLevel;
}

double WaterPhysics::getVerticalVelocity() const
{
    return verticalVelocity;
}

double WaterPhysics::updateVertical(double input, double dt)
{
    if (dt <= 0.0) return 0.0;

    elapsedTime += dt;

    input = std::clamp(input, -1.0, 1.0);

    // =====================================
    // 1. Ballast und Auftrieb
    // =====================================

    // 0 %   = +1.0 (maximaler Auftrieb)
    // 50 %  =  0.0 (neutral)
    // 100 % = -1.0 (maximales Sinken)

    double targetBuoyancy =
        1.0 - 2.0 * (waterLevel / 100.0);

    // Auftrieb mit Verzögerung anpassen
    double response =
        1.0 - std::exp(
            -dt / std::max(buoyancyResponseTime, 0.001)
        );

    currentBuoyancy +=
        (targetBuoyancy - currentBuoyancy) * response;

    double buoyancyForce =
        currentBuoyancy * buoyancyAcceleration;

    // =====================================
    // 2. Manueller Vertikalschub
    // =====================================

    double direction = 0.0;

    if (input > 0.0) direction = 1.0;
    if (input < 0.0) direction = -1.0;

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

    double curve =
        accelerationProgress * accelerationProgress;

    double manualAcceleration =
        input * verticalAcceleration * curve;

    // =====================================
    // 3. Wasserwiderstand
    // =====================================

    double drag =
        linearDrag * verticalVelocity
        + quadraticDrag * verticalVelocity
        * std::abs(verticalVelocity);

    // =====================================
    // 4. Geschwindigkeit aktualisieren
    // =====================================

    double totalAcceleration =
        buoyancyForce + manualAcceleration - drag;

    verticalVelocity += totalAcceleration * dt;

    verticalVelocity = std::clamp(
        verticalVelocity,
        -maxVerticalSpeed,
        maxVerticalSpeed
    );

    // Kleine Geschwindigkeiten beruhigen
    if (std::abs(verticalVelocity) < 0.001 &&
        std::abs(totalAcceleration) < 0.001)
    {
        verticalVelocity = 0.0;
    }

    // =====================================
    // 5. Leichtes Treiben bei 50 %
    // =====================================

    // Treiben nur in der Nähe von neutralem Auftrieb
    double neutralFactor = std::clamp(
        1.0 - std::abs(waterLevel - 50.0) / 10.0,
        0.0,
        1.0
    );

    double driftVelocity =
        std::sin(elapsedTime * waterDriftFrequency)
        * waterDriftStrength
        * neutralFactor;

    // =====================================
    // 6. Vertikale Verschiebung
    // =====================================

    return (verticalVelocity + driftVelocity) * dt;
}
