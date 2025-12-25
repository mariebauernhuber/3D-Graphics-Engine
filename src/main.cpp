#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>

static SDL_Window* window;
static SDL_Renderer* renderer;

struct vec3d{ float x,y,z; };
struct vec2d{ float x,y; };
struct triangle{ vec3d p[3]; };
struct mesh{ std::vector<triangle> tris; };
struct mat4x4{ float m[4][4] = { 0 }; };

mesh meshCube;
mat4x4 matProj;

int windowWidth, windowHeight;
float fTheta;
float fNear = 0.1f;
float fFar = 1000.0f;
float fFov = 90.0f;
float fFovRad;
float fAspectRatio;

float targetFrameRate = 60;
float realFrameRate;

float deltaTime;

bool debugModeTogggled = false;

void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m) {
    o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
    o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
    o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
    float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

    if(w != 0.0f) {
        o.x /= w;
        o.y /= w;
        o.z /= w;
    }
}

void CalculateScreenTransforms(){
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    fAspectRatio = (float)windowHeight / (float)windowWidth;
    fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159265358979323f);
}

void CalculateScreenProjection(){
    matProj.m[0][0] = fAspectRatio * fFovRad;
    matProj.m[1][1] = fFovRad;
    matProj.m[2][2] = fFar / (fFar - fNear);
    matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matProj.m[2][3] = 1.0f;
    matProj.m[3][3] = 0.0f;
}
// Basic function to calculate deltatime (used to make things independant from framerate), run every frame
void CalculateDeltaTime(){ 
    static Uint64 lastCounter = 0;

    Uint64 currentCounter = SDL_GetPerformanceCounter();
    Uint64 frequency = SDL_GetPerformanceFrequency();

    deltaTime = 0.0f;

    if (lastCounter != 0)
    {
        deltaTime = (float)(currentCounter - lastCounter) / (float)frequency;
    }

    lastCounter = currentCounter;
}

void PrintDebugInfo(){
    if(debugModeTogggled) {
    std::cout << "targetFrameRateIntoDelta: " << (1 / targetFrameRate) << "\n";
    std::cout << "realFrameRate: " << realFrameRate << "\n";
    std::cout << "\033[2J\033[1;1H"; //ANSI CODES to clear console screen
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        return SDL_APP_FAILURE;
    }
    window = SDL_CreateWindow("Test", 1280, 720, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);

    CalculateScreenTransforms();
    CalculateScreenProjection();
    
    meshCube.tris = {
        // South
        { 0.0f, 0.0f, 0.0f,     0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f,     1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f },

        // East
        { 1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f },

        // North
        { 1.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f,     0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f },

        // West 
        { 0.0f, 0.0f, 1.0f,     0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f },

        // Top
        { 0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f,     1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f },

        // Bottom
        { 1.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f,     0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f },
    };
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    if(event->type == SDL_EVENT_QUIT) { return SDL_APP_SUCCESS; }

    if(event->type == SDL_EVENT_WINDOW_RESIZED){
        CalculateScreenTransforms();
        CalculateScreenProjection();
    }

    if(event->type == SDL_EVENT_KEY_DOWN){
        if(event->key.scancode == SDL_SCANCODE_ESCAPE){ return SDL_APP_SUCCESS; }
        if(event->key.scancode == SDL_SCANCODE_F11){
            if(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN){
                SDL_SetWindowFullscreen(window, 0);
            }else{
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            };
            
        };
        if(event->key.scancode == SDL_SCANCODE_F8) {
            debugModeTogggled = !debugModeTogggled;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){

    CalculateDeltaTime();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Fill black background
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);


    // Set up rotation matrices (stolen from thelonecoder, temporary)
    mat4x4 matRotZ, matRotX;
    fTheta += 2.0f * deltaTime;

    // Rotation Z
    matRotZ.m[0][0] = cosf(fTheta);
    matRotZ.m[0][1] = sinf(fTheta);
    matRotZ.m[1][0] = -sinf(fTheta);
    matRotZ.m[1][1] = cosf(fTheta);
    matRotZ.m[2][2] = 1;
    matRotZ.m[3][3] = 1;

    // Rotation X
    matRotX.m[0][0] = 1;
    matRotX.m[1][1] = cosf(fTheta * 0.5f);
    matRotX.m[1][2] = sinf(fTheta * 0.5f);
    matRotX.m[2][1] = -sinf(fTheta * 0.5f);
    matRotX.m[2][2] = cosf(fTheta * 0.5f);
    matRotX.m[3][3] = 1;

    for(auto tri : meshCube.tris) {
        triangle triProjected, triTranslated, triRotatedZ, triRotatedX, triRotatedZX;
        
        // Rotate in Y-Axis
        MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
        MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
        MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

        // Rotate in X-Axis
        MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
        MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
        MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);
        
        //Offset onto the screen
        triTranslated = triRotatedZX;
        triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
        triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
        triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;
        
        vec3d normal, line1, line2;
        line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
        line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
        line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

        line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
        line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
        line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

        normal.x = line1.y * line2.z - line1.z * line2.y;
        normal.y = line1.z * line2.x - line1.x * line2.z;
        normal.z = line1.x * line2.y - line1.y * line2.x;

        float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        normal.x /= l;
        normal.y /= l;
        normal.z /= l;

        if(normal.z < 0) {
            // Project Tringles (3D -> 2D)
            MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
            MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
            MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

            // Scale into view
            triProjected.p[0].x = (triProjected.p[0].x + 1.0f) * 0.5f * windowWidth;
            triProjected.p[0].y = (1.0f - (triProjected.p[0].y + 1.0f) * 0.5f) * windowHeight;

            triProjected.p[1].x = (triProjected.p[1].x + 1.0f) * 0.5f * windowWidth;
            triProjected.p[1].y = (1.0f - (triProjected.p[1].y + 1.0f) * 0.5f) * windowHeight;

            triProjected.p[2].x = (triProjected.p[2].x + 1.0f) * 0.5f * windowWidth;
            triProjected.p[2].y = (1.0f - (triProjected.p[2].y + 1.0f) * 0.5f) * windowHeight;


            SDL_RenderLine(
            renderer,
            triProjected.p[0].x, triProjected.p[0].y,
            triProjected.p[1].x, triProjected.p[1].y
            );

            SDL_RenderLine(
            renderer,
            triProjected.p[1].x, triProjected.p[1].y,
            triProjected.p[2].x, triProjected.p[2].y
            );

            SDL_RenderLine(
            renderer,
            triProjected.p[2].x, triProjected.p[2].y,
            triProjected.p[0].x, triProjected.p[0].y
            );
        };

    };


    SDL_RenderPresent(renderer);

    SDL_Delay((1.0f/targetFrameRate)*1000.0f);
    realFrameRate = (1.0f / deltaTime);

    if(debugModeTogggled){
        PrintDebugInfo();
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
    SDL_Quit();
}
