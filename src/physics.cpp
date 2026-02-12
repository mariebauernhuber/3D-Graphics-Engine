#include "../include/physics.hpp"

btDiscreteDynamicsWorld* dynamicsWorld = nullptr;  // ← Definition

void initPhysics() {
    if (dynamicsWorld) return;  // Already init

    auto* config = new btDefaultCollisionConfiguration();
    auto* dispatcher = new btCollisionDispatcher(config);
    auto* broadphase = new btDbvtBroadphase();
    auto* solver = new btSequentialImpulseConstraintSolver();

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    dynamicsWorld->setGravity({0, -9.81f, 0});
}

void stepPhysics(float dt) {
    if (dynamicsWorld) dynamicsWorld->stepSimulation(dt, 10);
}

void cleanupPhysics() {
    if (dynamicsWorld) {
        //TODO: Remove all bodies first (loop your objects)
        delete dynamicsWorld;
        dynamicsWorld = nullptr;
    }
}
