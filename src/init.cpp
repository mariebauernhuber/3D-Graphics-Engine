#include "mesh.hpp"
#include "init.hpp"

std::vector<Object3D> objects;

bool initVideo(){
	return 0;
}

bool initGameObjs(){
  objects.reserve(1000);

  Object3D ship;
  ship.meshData.LoadFromAssimp("src/VideoShip.obj");
  SetObjDefaults(ship);
  ship.properties["name"] = "The test ship(TM)";
  ship.position = {15.0f, 15.0f, 15.0f};
  ship.scale = {1.0f, 1.0f, 1.0f};
  ship.cullingFrontFace = GL_CW;
  ship.cullingMode = GL_BACK;
  ship.CreateBroadCollisionCircle();
  ship.createCollisionShape(Object3D::CollisionPrecision::HIGH_TRIANGLE);
  ship.createCollisionMeshVAO();
  objects.push_back(ship);
  InitializeObjectGPU(ship);

  // Shoutout to tobi :3
  Object3D dickMaster;
  dickMaster.meshData.LoadFromAssimp("src/flower.obj");
  SetObjDefaults(dickMaster);
  dickMaster.properties["name"] = "Dick Master :3";
  dickMaster.position = {20.0f, 0.0f, 0.0f};
  dickMaster.cullingFrontFace = GL_CCW;
  dickMaster.cullingMode = GL_BACK;
  dickMaster.rotationPerTick = {25.0f, 0.0f, 0.0f};
  dickMaster.CreateBroadCollisionCircle();
  dickMaster.createCollisionShape(Object3D::CollisionPrecision::HIGH_TRIANGLE);
  dickMaster.createCollisionMeshVAO();
  objects.push_back(dickMaster);
  InitializeObjectGPU(dickMaster);

  Object3D testCubeCCW;
  testCubeCCW.meshData.LoadFromAssimp("src/cube.obj");
  SetObjDefaults(testCubeCCW);
  testCubeCCW.properties["name"] = "meow";
  testCubeCCW.position = {0.0f, 5.0f, 0.0f};
  testCubeCCW.rotationPerTick = {25.0f, 25.0f, 25.0f};
  testCubeCCW.cullingFrontFace = GL_CW;
  testCubeCCW.cullingMode = GL_BACK;
  testCubeCCW.CreateBroadCollisionCircle();
  testCubeCCW.createCollisionShape(Object3D::CollisionPrecision::HIGH_TRIANGLE);
  testCubeCCW.createCollisionMeshVAO();
  objects.push_back(testCubeCCW);
  InitializeObjectGPU(testCubeCCW);
	return 0;
};

bool initAppState(){
	initVideo();
	return 0;
}
