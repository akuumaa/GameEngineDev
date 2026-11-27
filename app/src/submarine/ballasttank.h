#pragma once

namespace App
{
    struct BallastSettings
    {
        double changeRate = 25.0;           // % per second (was ballastChangeRate)
        double responseTime = 1.5;          // inertia of buoyancy (was buoyancyResponseTime)
        double maxDeviation = 1.0;
    };

    /**
     * Ballast Tank - water level and resulting buoyancy factor
     * (gravity and buoyancy themselves are applied by the PhysicsWorld)
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
        // Smoothly follow the water level, return buoyancy factor
        double updateBuoyancy(double dt);
        double getBuoyancyFactor() const;

        //! State
        double getLevel() const;

      private:
        BallastSettings settings = {};

        double level = 50.0;
        double bouyancyFactor = 1.0;
    };
}