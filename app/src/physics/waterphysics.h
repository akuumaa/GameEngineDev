
#pragma once

#include "app/src/physics/waterphysicsconfig.h"
#include "app/src/physics/ballasttank.h"
#include "app/src/physics/inputramp.h"

/**
 * Water Physics - Vertical & Forward Movement
 * (Ballast: BallastTank, Input ramp: InputRamp)
 */
class WaterPhysics : public WaterPhysicsConfig
{
  public:
    //! Vertical Movement
    double updateVertical(double input, double dt);
    double getVerticalVelocity() const;

    //! Forward Movement
    double updateForward(double input, double dt);
    double getForwardVelocity() const;

    //! Ballast Tank
    App::BallastTank &getBallast();
    const App::BallastTank &getBallast() const;

  private:
    //! Manual Vertical Movement
    double calculateManualAcceleration(double input, double dt);

    //! Vertical Velocity
    void updateVerticalVelocity(double acceleration, double dt);

    //! Water Drift
    double calculateWaterDrift() const;

    //! Forward Movement
    double calculateForwardThrust(double input) const;

    //! Ballast Tank (settings taken from WaterPhysicsConfig)
    App::BallastTank ballast{
      App::BallastSettings {
        ballastChangeRate,
        buoyancyResponseTime,
        buoyancyAcceleration
      }
    };

    //! Vertical Input Ramp (ramp time taken from WaterPhysicsConfig)
    App::InputRamp verticalRamp{accelerationRampTime};

    //! Vertical Movement State
    double verticalVelocity = 0.0;

    //! Forward Movement State
    double forwardVelocity = 0.0;

    //! Simulation Time
    double elapsedTime = 0.0;
};
