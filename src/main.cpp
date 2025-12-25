#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

static SDL_Window* window;
static SDL_Renderer* renderer;

struct vec3d{ float x,y,z,w = 1; };
struct vec2d{ float x,y; };
struct triangle{ vec3d p[3]; };
struct mesh{ 
    std::vector<triangle> tris; 

    bool LoadFromObjectFile(std::string sFilename){
        std::ifstream f(sFilename);
        if(!f.is_open()){ return false; }

        // Local cache or verts
        std::vector<vec3d> verts;

        while(!f.eof()){
            char line[128];
            f.getline(line, 128);

            std::istringstream s(line);

            char junk;

            if(line[0]=='v'){
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            };
            if (line[0]=='f'){
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            };
        }
        return true;
    }
};
struct mat4x4{ float m[4][4] = { 0 }; };

mesh meshCube;
mat4x4 matProj;

vec3d vCamera = { 0.0f, 0.0f, 0.0f };

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

vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i){
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
}

mat4x4 Matrix_MakeIdentity(){
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

vec3d Vector_Add(vec3d &v1, vec3d &v2) {
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d Vector_Sub(vec3d &v1, vec3d &v2) {
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3d Vector_Mul(vec3d &v1, float k) {
    return { v1.x * k, v1.y * k, v1.z * k };
}

vec3d Vector_Div(vec3d &v1, float k) {
    return { v1.x / k, v1.y / k, v1.z / k };
}

float Vector_DotProduct(vec3d &v1, vec3d &v2){
    return ( (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) );
}

float Vector_Length(vec3d &v){
    return sqrtf(Vector_DotProduct(v, v));
}

vec3d Vector_Normalise(vec3d &v) {
    float l = Vector_Length(v);
    return { v.x / l, v.y / l, v.z / l };
}

vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2){
    vec3d v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad){
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[1][2] = sinf(fAngleRad);
    matrix.m[2][1] = -sinf(fAngleRad);
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad){
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][2] = sinf(fAngleRad);
    matrix.m[2][0] = -sinf(fAngleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad){
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][1] = sinf(fAngleRad);
    matrix.m[1][0] = -sinf(fAngleRad);
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z){
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar){
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    mat4x4 matrix;
    matrix.m[0][0] = fAspectRatio * fFovRad;
    matrix.m[1][1] = fFovRad;
    matrix.m[2][2] = fFar / (fFar - fNear);
    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2){
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return matrix;
}

void CalculateScreenTransforms(){
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    fAspectRatio = (float)windowHeight / (float)windowWidth;
    fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159265358979323f);
}

void CalculateScreenProjection(){
    matProj = Matrix_MakeProjection(90.0f, fAspectRatio, fNear, fFar);
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
   
    meshCube.LoadFromObjectFile("src/VideoShip.obj");

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
        };
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){

    CalculateDeltaTime();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Fill black background
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Set up rotation matrices (stolen from thelonecoder, temporary)
    mat4x4 matRotZ, matRotX, matRotY;
    fTheta += 1.0f * deltaTime;

    matRotZ = Matrix_MakeRotationZ(fTheta);
    matRotX = Matrix_MakeRotationX(fTheta);
    matRotY = Matrix_MakeRotationY(fTheta);

    mat4x4 matTrans;
    matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

    mat4x4 matWorld;
    matWorld = Matrix_MakeIdentity();
    matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
    matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

    std::vector<triangle> vecTrianglesToRaster;

    for(auto tri : meshCube.tris) {
        triangle triProjected, triTransformed;
        
        triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
        triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
        triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);
        
        // Triangle normals
        vec3d normal, line1, line2;
        
        //Get lines either side of triangle
        line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
        line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
        
        //Take cross product of lines to get normal to triangle surface 
        normal = Vector_CrossProduct(line1, line2);

        // Normalise
        normal = Vector_Normalise(normal);

        vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

        if(Vector_DotProduct(normal, vCameraRay) < 0.0f){

            //Project into 2D space
            triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
            triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
            triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);

            //Scale into view, normalise
            triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
            triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
            triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

            //Offset
            vec3d vOffsetView = { 1, 1, 0 };
            triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
            triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
            triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
            triProjected.p[0].x *= 0.5f * windowWidth;
            triProjected.p[0].y *= 0.5f * windowHeight;
            triProjected.p[1].x *= 0.5f * windowWidth;
            triProjected.p[1].y *= 0.5f * windowHeight;
            triProjected.p[2].x *= 0.5f * windowWidth;
            triProjected.p[2].y *= 0.5f * windowHeight;

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
    };

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
    SDL_Quit();
}
