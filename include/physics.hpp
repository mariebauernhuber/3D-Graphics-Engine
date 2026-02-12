#pragma once

#define PHYSICS_HPP

#include <btBulletDynamicsCommon.h>

extern btDiscreteDynamicsWorld* dynamicsWorld;  // ← Declaration

void initPhysics();
void stepPhysics(float dt);
void cleanupPhysics();
