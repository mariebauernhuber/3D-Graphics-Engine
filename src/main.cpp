#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include "../include/mesh.hpp"
#include "../include/geometry.hpp"
#include "../include/renderer.hpp"

extern bool debugModeTogggled;
extern float deltaTime;
extern float targetFrameRate;
extern float realFrameRate;

extern std::vector<Object3D> objects;

extern CullMode gCullMode;

extern mat4x4 matView, matProj;

extern unsigned long long nDrawCycles;

extern vec3d vCamera;
extern vec3d vLookDir;

mesh meshCube;
mesh meshCube2;
mesh teapotMesh;
mesh teddybearMesh;

float fYaw   = 0.0f;   // left/right
float fPitch = 0.0f;   // up/down
float fMaxPitch = 1.55f;
float fMouseSensitivity = 0.0025f;

bool consoleOpen = false;

int main(int argc, char* argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        return 1;
    }

    window = SDL_CreateWindow("Amazing game", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_RELATIVE_MODE | SDL_WINDOW_MOUSE_GRABBED);
    renderer = SDL_CreateRenderer(window, NULL);

    CalculateScreenTransforms();
    CalculateScreenProjection();
   
    meshCube.LoadFromObjectFile("src/VideoShip.obj");
    meshCube2.LoadFromObjectFile("src/VideoShip.obj");
    teapotMesh.LoadFromObjectFile("src/teapot.obj");
    teddybearMesh.LoadFromObjectFile("src/teddybear.obj");

    Object3D ship;
    ship.meshData = meshCube;
    ship.position = {0.0f, 0.0f, 5.0f};
    ship.rotation = {0.0f, 0.0f, 0.0f};

    Object3D ship2;
    ship2.meshData = meshCube;
    ship2.position = {0.0f, 0.0f, 15.0f};
    ship2.rotation = {0.0f, 0.0f, 0.0f};

    Object3D ship3;
    ship3.meshData = meshCube;
    ship3.position = {0.0f, 0.0f, 25.0f};
    ship3.rotation = {0.0f, 0.0f, 0.0f};

    Object3D movingShip;
    movingShip.meshData = meshCube;
    movingShip.position = {10.0f, 0.0f, 0.0f};
    movingShip.rotation = {5.0f, 0.0f, 0.0f};

    Object3D teapot;
    teapot.meshData = teapotMesh;
    teapot.position = {25.0f, 0.0f, 25.0f};
    teapot.rotation = {0.0f, 180.0f, 0.0f};

    Object3D teddybear;
    teddybear.meshData = teddybearMesh;
    teddybear.position = {50.0f, 0.0f, 25.0f};
    teddybear.rotation = {0.0f, 180.0f, 0.0f};

    objects.push_back(ship);
    objects.push_back(ship2);
    objects.push_back(ship3);
    objects.push_back(movingShip);
    objects.push_back(teapot);
    objects.push_back(teddybear);

    SDL_SetWindowRelativeMouseMode(window, true);


    std::cout << "Init done!";

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    bool is_running = true;
    SDL_Event event;

    while(is_running){
	    while(SDL_PollEvent(&event)){
		    if (event.type == SDL_EVENT_QUIT) {
			    is_running = false;
		    }
		    if(event.type == SDL_EVENT_WINDOW_RESIZED){
			CalculateScreenTransforms();
			CalculateScreenProjection();
		    }

		    if(event.type == SDL_EVENT_KEY_DOWN){
			if(event.key.scancode == SDL_SCANCODE_ESCAPE){ is_running = false; }
			if(event.key.scancode == SDL_SCANCODE_F11){
			    if(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN){
				SDL_SetWindowFullscreen(window, 0);
			    }else{
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			    };
			    
			};
			if(event.key.scancode == SDL_SCANCODE_F8) {
				debugModeTogggled = !debugModeTogggled;
			};
			if (event.key.scancode == SDL_SCANCODE_F6) {
			    if (gCullMode == CullMode::Back) gCullMode = CullMode::Front;
			    else if (gCullMode == CullMode::Front) gCullMode = CullMode::None;
			    else gCullMode = CullMode::Back;
			}

		    }

		    if(event.type == SDL_EVENT_MOUSE_MOTION){
			fYaw   += event.motion.xrel * fMouseSensitivity;
			fPitch -= event.motion.yrel * fMouseSensitivity;

			// Clamp pitch to avoid flipping
			if (fPitch > fMaxPitch)  fPitch = fMaxPitch;
			if (fPitch < -fMaxPitch) fPitch = -fMaxPitch;
		    }

	    }

		    // 1. Update delta time
		    CalculateDeltaTime();
		    MinuteTimer();

		    // 2. Limit frame rate
		    float remainingFrameTime = (1.0f / targetFrameRate) - deltaTime;
		    if (remainingFrameTime > 0.001f){
			SDL_Delay(int(remainingFrameTime * 1000.0f));
		    }

		    // 3. Process keyboard input for camera movement
		    const bool *key_states = SDL_GetKeyboardState(NULL);
		    float fMoveSpeed = 8.0f * deltaTime;
			
		    // Forward / Back
		    if (key_states[SDL_SCANCODE_W])
			vCamera = Vector_Add(vCamera, Vector_Mul(Vector_Normalise({vLookDir.x, 0.0f, vLookDir.z}), fMoveSpeed));
		    if (key_states[SDL_SCANCODE_S])
			vCamera = Vector_Sub(vCamera, Vector_Mul(Vector_Normalise({vLookDir.x, 0.0f, vLookDir.z}), fMoveSpeed));

		    // Right / Left
		    vec3d vRight = Vector_Normalise(Vector_CrossProduct(vLookDir, {0,1,0}));
		    if (key_states[SDL_SCANCODE_D])
			vCamera = Vector_Sub(vCamera, Vector_Mul(vRight, fMoveSpeed));
		    if (key_states[SDL_SCANCODE_A])
			vCamera = Vector_Add(vCamera, Vector_Mul(vRight, fMoveSpeed));

		    // Up / Down
		    if (key_states[SDL_SCANCODE_SPACE])
			vCamera.y += 8.0f * deltaTime;
		    if (key_states[SDL_SCANCODE_LSHIFT])
			vCamera.y -= 8.0f * deltaTime;

		    // 4. Update camera orientation based on mouse look
		    vec3d vForward = {
			cosf(fPitch) * sinf(fYaw),
			sinf(fPitch),
			cosf(fPitch) * cosf(fYaw)
		    };

		    vLookDir = Vector_Normalise(vForward);

		    objects[2].rotation.z += 2.0f * deltaTime;
		    objects[1].rotation.y += 2.0f * deltaTime;
		    objects[0].rotation.x += 2.0f * deltaTime;

		    objects[4].rotation.y += 2.0f * deltaTime;

		    // 5. Update view matrix
		    vec3d vUp = {0, 1, 0};
		    matView = Matrix_PointAt(vCamera, Vector_Add(vCamera, vLookDir), vUp);
		    matView = Matrix_QuickInverse(matView);

		    UpdateFrustumPlanes();

		    // 6. Clear screen
		    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		    SDL_RenderClear(renderer);
		    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		    // 7. Render all objects
		    for(auto &obj : objects){
			RenderObject(renderer, obj, matView, matProj);
		    }

		    if(consoleOpen){
		    // Draw a semi-transparent background
		    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); 
		    SDL_FRect consoleRect = {50, 50, 400, 300};
		    SDL_RenderFillRect(renderer, &consoleRect);

		    // Here you could draw text using SDL_ttf or something simple
		    // For debugging, you could also print object positions to console
		    for(size_t i = 0; i < objects.size(); i++){
			std::cout << "Object " << i 
				  << " Pos: " << objects[i].position.x << ", " 
				  << objects[i].position.y << ", " 
				  << objects[i].position.z << "\n";
			std::cout << "Rot: " << objects[i].rotation.x << ", "
				  << objects[i].rotation.y << ", "
				  << objects[i].rotation.z << "\n";
		    }
		}

		    // 8. Present final frame
		    SDL_RenderPresent(renderer);

		    // 9. Update real frame rate
		    realFrameRate = 1.0f / deltaTime;

		    // 10. Optional debug info
		    if(debugModeTogggled)
			PrintDebugInfo();

		    // 11. Increment draw cycles
		    nDrawCycles++;
	    }

    SDL_Quit();
    return 0;
}

