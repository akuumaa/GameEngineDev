#pragma once

namespace App
{
    /**
     * Input Ramp - gradually increases input strength while held.
     * Resets when the input direction changes.
     */
    class InputRamp
    {
      public:
        explicit InputRamp(double rampTime = 2.0);

        // Update ramp progress and return the ramped input (input * curve)
        double update(double input, double dt);

        // Current progress 0..1
        double getProgress() const;

      private:
        double rampTime = 2.0;           // seconds to full strength (was accelerationRampTime)
        double progress = 0.0;           // was accelerationProgress
        double previousDirection = 0.0;
    };
}