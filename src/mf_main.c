#include <stdio.h>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>

bool is_running = true;

SDL_Window* window;

int main(int argc, char *argv[]){
	printf("Hai :3\n");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if(!SDL_Init(SDL_INIT_VIDEO)){ return 1; }

	glEnable(GL_DEBUG_OUTPUT);

	glewExperimental = GL_TRUE;

	window = SDL_CreateWindow("RipWake", 500, 500, SDL_WINDOW_OPENGL);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);

	int i = 0;

	while(is_running){
		i++;
		SDL_GL_MakeCurrent(window, gl_context);
		SDL_GL_SwapWindow(window);
		if(i>=10000){ is_running = false; }
	}

	SDL_Quit();
	return 0;
}
