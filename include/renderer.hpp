#pragma once
#include "geometry.hpp"
#include <SDL3/SDL.h>
#include "mesh.hpp"

static SDL_Window* window;

enum class CullMode {
    None,
    Back,
    Front
};

void CalculateScreenTransforms(SDL_Window* window);
void CalculateScreenProjection();
void CalculateDeltaTime();
void execOncePerSec();
void MinuteTimer();
vec2d ProjectToScreen(const vec3d &v);
void InitDefaultTexture();
GLuint LoadTextureFromFile(const char* filename);
void RenderObjectAssimp(Object3D& obj, int ID, GLuint shader, GLuint texture, const mat4x4 &matView, const mat4x4 &matProj, GLuint frontFace, GLuint cullMode);
