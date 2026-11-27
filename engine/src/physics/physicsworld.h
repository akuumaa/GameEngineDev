#pragma once

#include "engine/src/math/quaternion.h"
#include "engine/src/math/vector3.h"
#include "engine/src/physics/rigidbody.h"

#include <limits>

using Engine::Math::Quaternion;
using Engine::Math::Vector3;

namespace Engine::Physics
{
    /**
     * Physics World - environment all bodies live in:
     * gravity, water (buoyancy, drag, righting).
     * The ONLY place where environment forces are applied to bodies.
     */
    class PhysicsWorld
    {
      public:
        //! Environment Settings
        void setGravity(const Vector3 &value);
        const Vector3 &getGravity() const;

        // z of the water surface. Default: infinite (everything is under water)
        void setWaterSurface(double height, double transition = 0.5);
        double getWaterSurface() const;

        // 0 = above water, 1 = fully submerged, smooth in between
        double submergedFraction(const Vector3 &position) const;

        //! Simulation
        // One step: environment forces → body.integrate
        void step(RigidBody &body, Vector3 &position, Quaternion &rotation, double dt) const;

        // Splits a frame into equal sub-steps of at most maxSubstep seconds,
        // so physics behaves the same at 30 and 144 FPS
        int substepCount(double dt) const;
        void setMaxSubstep(double seconds);

      private:
        //! Environment Forces (one effect per function)
        void applyGravity(RigidBody &body) const;
        void applyBuoyancy(RigidBody &body, const Quaternion &rotation, double submerged) const;
        void applyDrag(RigidBody &body, const Quaternion &rotation, double submerged) const;

        Vector3 gravity = {0.0, 0.0, -5.0};

        double waterSurface = std::numeric_limits<double>::infinity();
        double waterTransition = 0.5;

        double maxSubstep = 1.0 / 120.0;
        int maxSubsteps = 8; // protects against huge dt (lag spikes)
    };
}