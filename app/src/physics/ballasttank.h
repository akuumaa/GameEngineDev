#pragma once

namespace App
{
    struct BallastSettings
    {
        double changeRate = 25.0;           // % per second (was ballastChangeRate)
        double responseTime = 1.5;          // inertia of buoyancy (was buoyancyResponseTime)
        double buoyancyAcceleration = 5.0;  // max buoyancy acceleration (was buoyancyAcceleration)
    };

    /**
     * Ballast Tank - water level and resulting buoyancy
     */
    class BallastTank
    {
      public:
        explicit BallastTank(const BallastSettings &settings = {});

        //! Ballast Control
        // Set water level and limit it to 0-100%
        void setLevel(double percentage);

        // Increase or decrease water based on input and delta time
        void changeLevel(double input, double dt);

        // Reset to neutral buoyancy (50%)
        void reset();

        //! Buoyancy
        // Smoothly follow the water level, return buoyancy acceleration
        double updateBuoyancy(double dt);

        //! State
        double getLevel() const;

      private:
        BallastSettings settings = {};

        double level = 50.0;
        double currentBuoyancy = 0.0;
    };
}