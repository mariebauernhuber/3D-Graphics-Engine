#pragma once
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc);
char* LoadShaderSource(const char* filePath);
void SetupScreenQuad();
