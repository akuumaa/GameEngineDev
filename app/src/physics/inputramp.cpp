#include "app/src/physics/inputramp.h"

#include "engine/src/physics/physicsutils.h"

#include <algorithm>

namespace App
{
    InputRamp::InputRamp(double rampTime)
        : rampTime(rampTime)
    {
    }

    double InputRamp::update(double input, double dt)
    {
        double direction = 0.0;

        if (input > 0.0) direction = 1.0;
        if (input < 0.0) direction = -1.0;

        // Reset ramp when input direction changes
        if (direction != previousDirection)
        {
            progress = 0.0;
            previousDirection = direction;
        }

        // Gradually increase progress while input is active
        if (direction != 0.0)
        {
            progress +=
                dt / std::max(rampTime, 0.001);

            progress = std::clamp(
                progress,
                0.0,
                1.0
            );
        }

        double curve = PhysicsUtils::calculateAccelerationCurve(progress);

        return input * curve;
    }

    double InputRamp::getProgress() const
    {
        return progress;
    }
}