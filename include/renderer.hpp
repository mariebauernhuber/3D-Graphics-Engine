#pragma once
#include "geometry.hpp"
#include <vector>
#include <SDL3/SDL.h>
#include "mesh.hpp"

static SDL_Window* window;
static SDL_Renderer* renderer;

enum class CullMode {
    None,
    Back,
    Front
};

void drawFilledTriangle(SDL_Renderer* renderer, vec2d p0, vec2d p1, vec2d p2);
void CalculateScreenTransforms();
void CalculateScreenProjection();
void CalculateDeltaTime();
void execOncePerSec();
void MinuteTimer();
void PrintDebugInfo();
vec2d ProjectToScreen(const vec3d &v);
bool IsTriangleInView(const triangle &tri);
void UpdateFrustumPlanes();
std::vector<triangle> ClipTriangleToFrustumOptimized(const triangle &tri);
void RenderObject(SDL_Renderer* renderer, Object3D &obj, const mat4x4 &matView, const mat4x4 &matProj);

