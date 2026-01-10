#pragma once
#include "geometry.hpp"
#include <vector>
#include <string>

struct mesh{
	std::vector<triangle> tris;

	bool LoadFromObjectFile(const std::string& filename);
};

struct Object3D{
	mesh meshData;
	vec3d position;
	vec3d rotation;
	vec3d scale = {1,1,1};

	mat4x4 GetWorldMatrix();
};
