#include "debug.h"
#include "math.h"
#include "shader-utils.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>

bool is_running = true;

int windowWidth = 500;
int windowHeight = 500;

extern GLuint screenShaderProgram;
extern GLuint quadVAO, quadVBO;

SDL_Window* window;

GLuint fbo, textureColorBuffer, rbo;

int main(int argc, char *argv[]){
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return 1;

	glEnable(GL_DEBUG_OUTPUT);

	glewExperimental = GL_TRUE;

	window = SDL_CreateWindow("RipWake", windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	if(!window) return 1;

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if(!gl_context) return 1;

	testErrFatal(1, SDL_GL_MakeCurrent(window, gl_context));

	if(glewInit() != GLEW_OK) return 1;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ERROR: FRAMEBUFFER UNCOMPLETE\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to default screen

	SetupScreenQuad();

	m_v4 clrCol = {0.0f, 0.0f, 0.0f, 1.0f};

	while(is_running){
		clrCol.x = clrCol.x + 0.0001f;

		glViewport(0, 0, windowWidth, windowHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(clrCol.x, clrCol.y, clrCol.z, clrCol.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//if(wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//if(cullingEnabled) glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glUseProgram(screenShaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glUniform1i(glGetUniformLocation(screenShaderProgram, "screenTexture"), 0);

		glClear(GL_COLOR_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		//if(glGetError() != 0){
		//    std::cout << "GLERROR: " << glGetError() << std::endl;
		//}

		SDL_GL_MakeCurrent(window, gl_context);
		SDL_GL_SwapWindow(window);

		if(clrCol.x >=1.0f){
			is_running = false;
		}
	}

	SDL_Quit();
	return 0;
}
