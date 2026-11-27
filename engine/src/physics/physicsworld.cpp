#include "engine/src/physics/physicsworld.h"

#include "engine/src/physics/physicsutils.h"

#include <algorithm>
#include <cmath>

namespace Engine::Physics
{
    //! Environment Settings

    void PhysicsWorld::setGravity(const Vector3 &value)
    {
        gravity = value;
    }

    const Vector3 &PhysicsWorld::getGravity() const
    {
        return gravity;
    }

    void PhysicsWorld::setWaterSurface(double height, double transition)
    {
        waterSurface = height;
        waterTransition = transition;
    }

    double PhysicsWorld::getWaterSurface() const
    {
        return waterSurface;
    }

    double PhysicsWorld::submergedFraction(const Vector3 &position) const
    {
        // No surface set → everything is under water
        if (std::isinf(waterSurface)) return waterSurface > 0.0 ? 1.0 : 0.0;

        const double depth = waterSurface - position.z;

        if (waterTransition <= 0.0) return depth >= 0.0 ? 1.0 : 0.0;

        // depth = -transition/2 → 0, depth = +transition/2 → 1
        return PhysicsUtils::smoothstep(depth / waterTransition + 0.5);
    }

    //! Simulation

    void PhysicsWorld::step(RigidBody &body, Vector3 &position, Quaternion &rotation, double dt) const
    {
        if (dt <= 0.0) return;

        const double submerged = submergedFraction(position);

        applyGravity(body);

        if (submerged > 0.0)
        {
            applyBuoyancy(body, rotation, submerged);
            applyDrag(body, rotation, submerged);
        }

        body.integrate(position, rotation, dt);
    }

    int PhysicsWorld::substepCount(double dt) const
    {
        if (dt <= 0.0 || maxSubstep <= 0.0) return 1;

        const int count = static_cast<int>(std::ceil(dt / maxSubstep));
        return std::clamp(count, 1, maxSubsteps);
    }

    void PhysicsWorld::setMaxSubstep(double seconds)
    {
        maxSubstep = seconds;
    }

    //! Environment Forces

    void PhysicsWorld::applyGravity(RigidBody &body) const
    {
        body.addAcceleration(gravity);
    }

    void PhysicsWorld::applyBuoyancy(RigidBody &body, const Quaternion &rotation, double submerged) const
    {
        const BodyProperties &properties = body.getProperties();

        // Push against gravity: buoyancyFactor 1.0 cancels gravity exactly
        body.addAcceleration(gravity * (-properties.buoyancyFactor * submerged));

        // Righting: tilt axis turns the body's "up" towards world "up"
        if (properties.rightingStrength != 0.0)
        {
            const Vector3 bodyUp = rotation * Vector3::up;
            const Vector3 tiltAxis = PhysicsUtils::cross(bodyUp, Vector3::up);

            body.addAngularAcceleration(tiltAxis * (properties.rightingStrength * submerged));
        }
    }

    void PhysicsWorld::applyDrag(RigidBody &body, const Quaternion &rotation, double submerged) const
    {
        const BodyProperties &properties = body.getProperties();
        const double inverseMass = 1.0 / std::max(properties.mass, 1e-6);

        // Linear: calculate in local axes, rotate back to world
        const Vector3 localDrag = PhysicsUtils::calculateDrag(
            body.getLocalVelocity(rotation),
            properties.linearDrag,
            properties.quadraticDrag
        );

        body.addAcceleration(rotation * localDrag * (-submerged * inverseMass));

        // Angular: also per local axis (roll, pitch, yaw)
        const Vector3 localAngularDrag = PhysicsUtils::multiply(
            body.getLocalAngularVelocity(rotation),
            properties.angularDrag
        );

        body.addAngularAcceleration(rotation * localAngularDrag * -submerged);
    }
}