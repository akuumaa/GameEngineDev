#include "engine/src/physics/rigidbody.h"

#include "engine/src/physics/physicsutils.h"

#include <algorithm>

namespace Engine::Physics
{
    //! Collect forces

    void RigidBody::addForce(const Vector3 &worldForce)
    {
        // F = m * a → a = F / m
        accumulatedAcceleration = accumulatedAcceleration
            + worldForce * (1.0 / std::max(properties.mass, 1e-6));
    }

    void RigidBody::addLocalForce(const Vector3 &localForce, const Quaternion &rotation)
    {
        addForce(rotation * localForce);
    }

    void RigidBody::addAcceleration(const Vector3 &worldAcceleration)
    {
        accumulatedAcceleration = accumulatedAcceleration + worldAcceleration;
    }

    void RigidBody::addAngularAcceleration(const Vector3 &worldAngularAcceleration)
    {
        accumulatedAngularAcceleration = accumulatedAngularAcceleration + worldAngularAcceleration;
    }

    void RigidBody::addLocalAngularAcceleration(
        const Vector3 &localAngularAcceleration,
        const Quaternion &rotation
    )
    {
        addAngularAcceleration(rotation * localAngularAcceleration);
    }

    //! One step

    void RigidBody::integrate(Vector3 &position, Quaternion &rotation, double dt)
    {
        // Velocity update
        velocity = PhysicsUtils::integrateVelocity(velocity, accumulatedAcceleration, dt);
        angularVelocity = PhysicsUtils::integrateVelocity(angularVelocity, accumulatedAngularAcceleration, dt);

        accumulatedAcceleration = {};
        accumulatedAngularAcceleration = {};

        // Max speed per local axis
        const Vector3 &limit = properties.maxLocalSpeed;
        if (limit.x > 0.0 || limit.y > 0.0 || limit.z > 0.0)
        {
            const Vector3 local = PhysicsUtils::clampVelocity(getLocalVelocity(rotation), limit);
            velocity = rotation * local;
        }

        // Position update
        position = position + PhysicsUtils::calculateDisplacement(velocity, dt);

        // Rotation update: rotate around the world axis of angularVelocity
        const double speed = angularVelocity.length();
        const double angle = speed * dt;

        if (angle > 1e-12)
        {
            const Vector3 axis = angularVelocity * (1.0 / speed);
            rotation = Quaternion::normalize(Quaternion(axis, angle) * rotation);
        }
    }

    //! Access

    Vector3 RigidBody::getLocalVelocity(const Quaternion &rotation) const
    {
        // -rotation = inverse (like in your ShipScript)
        return (-rotation) * velocity;
    }

    Vector3 RigidBody::getLocalAngularVelocity(const Quaternion &rotation) const
    {
        return (-rotation) * angularVelocity;
    }
}