#pragma once

#include "engine/src/math/quaternion.h"
#include "engine/src/math/vector3.h"

using Engine::Math::Quaternion;
using Engine::Math::Vector3;

namespace Engine::Physics
{
    /**
     * What a body IS (tuning data, rarely changes).
     * Local axes like in the engine: x = forward, y = left, z = up.
     */
    struct BodyProperties
    {
        // Divides forces (addForce) and drag. Gravity/buoyancy do not depend on it.
        double mass = 1.0;

        // 1.0 = floats, > 1 = rises (bubble), < 1 = sinks (pebble).
        // The submarine changes this value via its ballast tank.
        double buoyancyFactor = 1.0;

        // Drag per LOCAL axis
        Vector3 linearDrag = {0.1, 0.1, 0.1};
        Vector3 quadraticDrag = {0.04, 0.04, 0.04};

        // Rotation damping per local axis (roll, pitch, yaw)
        Vector3 angularDrag = {1.0, 1.0, 1.0};

        // Pulls the body back to level (0 = off). Only works in water.
        double rightingStrength = 0.0;

        // Max speed per local axis, component <= 0 = no limit
        Vector3 maxLocalSpeed = {0.0, 0.0, 0.0};
    };

    /**
     * State of a physical body (velocity, angular velocity).
     * Does NOT know where forces come from – only collects and integrates them.
     */
    class RigidBody
    {
      public:
        explicit RigidBody(const BodyProperties &properties = {}) : properties(properties) {}

        //! Collect forces (applied and cleared in integrate())

        // Force in world space, divided by mass
        void addForce(const Vector3 &worldForce);

        // Force in local space (e.g. engine pushing forward)
        void addLocalForce(const Vector3 &localForce, const Quaternion &rotation);

        // Acceleration directly, ignores mass (gravity, current)
        void addAcceleration(const Vector3 &worldAcceleration);

        // Angular acceleration in world space (axis * strength)
        void addAngularAcceleration(const Vector3 &worldAngularAcceleration);

        // Angular acceleration in local axes (x = roll, y = pitch, z = yaw)
        void addLocalAngularAcceleration(const Vector3 &localAngularAcceleration, const Quaternion &rotation);

        //! One step

        // Applies collected accelerations, moves position/rotation, clears the accumulators
        void integrate(Vector3 &position, Quaternion &rotation, double dt);

        //! Access

        const Vector3 &getVelocity() const { return velocity; }
        void setVelocity(const Vector3 &v) { velocity = v; }

        // Velocity in local axes (x = forward, y = sideways, z = up)
        Vector3 getLocalVelocity(const Quaternion &rotation) const;

        const Vector3 &getAngularVelocity() const { return angularVelocity; }
        void setAngularVelocity(const Vector3 &w) { angularVelocity = w; }

        Vector3 getLocalAngularVelocity(const Quaternion &rotation) const;

        const BodyProperties &getProperties() const { return properties; }
        BodyProperties &getProperties() { return properties; } // e.g. ballast changes buoyancyFactor

      private:
        BodyProperties properties = {};

        //! State (kept between frames, world space)
        Vector3 velocity = {};
        Vector3 angularVelocity = {};

        //! Accumulators (cleared in integrate())
        Vector3 accumulatedAcceleration = {};
        Vector3 accumulatedAngularAcceleration = {};
    };
}