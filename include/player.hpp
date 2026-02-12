#pragma once
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include "physics.hpp"
#include <SDL3/SDL_keyboard.h>
#include <geometry.hpp>

struct Player {
    vec3d position = {0, 5, 0};
    vec3d velocity = {0, 0, 0};
    bool grounded = false;
    
    btRigidBody* rigidBody = nullptr;
    btCollisionShape* capsuleShape = nullptr;  // Player capsule
    
    void init() {
        // Capsule shape (0.5m radius, 1.8m height)
        capsuleShape = new btCapsuleShape(0.5f, 1.8f);
        
        btTransform startTrans;
        startTrans.setIdentity();
        startTrans.setOrigin({position.x, position.y, position.z});
        
        btScalar mass = 1.0f;
        btVector3 inertia(0, 0, 0);
        capsuleShape->calculateLocalInertia(mass, inertia);
        
        auto* motionState = new btDefaultMotionState(startTrans);
        btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, capsuleShape, inertia);
        rigidBody = new btRigidBody(info);
        
        dynamicsWorld->addRigidBody(rigidBody);
    }
    
    void update(float dt) {
        // Read physics position back to visual
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        position.x = trans.getOrigin().x();
        position.y = trans.getOrigin().y();
        position.z = trans.getOrigin().z();
        
	const bool *key_states = SDL_GetKeyboardState(NULL);
        btVector3 force(0, 0, 0);
        if (key_states[SDL_SCANCODE_W]) force += {0, 0, -5};
        if (key_states[SDL_SCANCODE_S]) force += {0, 0, 5};
        if (key_states[SDL_SCANCODE_A]) force += {-5, 0, 0};
        if (key_states[SDL_SCANCODE_D]) force += {5, 0, 0};
        if (key_states[SDL_SCANCODE_SPACE] && grounded) velocity.y = 8.0f;
        
        rigidBody->applyCentralForce(force);
        rigidBody->setAngularFactor({0,0,0});  // No rotation
    }
};
