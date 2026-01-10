#include "../include/renderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../include/mesh.hpp"

bool debugModeTogggled = false;

unsigned long long nDrawCycles = 0;

int windowWidth, windowHeight;
float fTheta;
float fNear = 0.1f;
float fFar = 1000.0f;
float fFov = 90.0f;
float fFovRad;
float fAspectRatio;
float deltaTime;

float targetFrameRate = 60;
float realFrameRate;

vec3d vCamera = { 0.0f, 0.0f, 0.0f };
vec3d vLookDir;

unsigned long long framesElapsedSinceStartup;
unsigned long long secondsElapsedSinceStartup;
float minutesElapsedSinceStartup;
float hoursElapsedSinceStartup;
float daysElapsedSinceStartup;

std::vector<Object3D> objects;

mat4x4 matView, matProj;

mat4x4 matRotZ = Matrix_MakeIdentity();
mat4x4 matRotX = Matrix_MakeIdentity();
mat4x4 matRotY = Matrix_MakeIdentity();
mat4x4 matTrans = Matrix_MakeIdentity();
mat4x4 matWorld = Matrix_MakeIdentity();

CullMode gCullMode = CullMode::Back;

void drawFilledTriangle(SDL_Renderer* renderer, vec2d p0, vec2d p1, vec2d p2) {
    // Sort vertices by y
    if (p1.y < p0.y) std::swap(p0, p1);
    if (p2.y < p0.y) std::swap(p0, p2);
    if (p2.y < p1.y) std::swap(p1, p2);

    auto interpX = [](vec2d a, vec2d b, float y) -> float {
        if (a.y == b.y) return a.x; // avoid division by zero
        return a.x + (b.x - a.x) * ((y - a.y) / (b.y - a.y));
    };

    auto drawScanline = [&](int y, float x0, float x1) {
        int ix0 = int(std::round(x0));
        int ix1 = int(std::round(x1));
        if (ix0 > ix1) std::swap(ix0, ix1);
        SDL_RenderLine(renderer, ix0, y, ix1, y);
    }; 

    int yStart, yEnd;

    // Top half of triangle
    if (p1.y != p0.y) { // avoid degenerate top flat
        yStart = int(std::ceil(p0.y));
        yEnd   = int(std::floor(p1.y));
        for (int y = yStart; y <= yEnd; y++) {
            float xa = interpX(p0, p1, float(y));
            float xb = interpX(p0, p2, float(y));
            drawScanline(y, xa, xb);
        }
    }

    // Bottom half of triangle
    if (p2.y != p1.y) { // avoid degenerate bottom flat
        yStart = int(std::ceil(p1.y));
        yEnd   = int(std::floor(p2.y));
        for (int y = yStart; y <= yEnd; y++) {
            float xa = interpX(p1, p2, float(y));
            float xb = interpX(p0, p2, float(y));
            drawScanline(y, xa, xb);
        }
    }
}

void CalculateScreenTransforms(){
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    fAspectRatio = (float)windowHeight / (float)windowWidth;
    fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159265358979323f);
}

void CalculateScreenProjection(){
    matProj = Matrix_MakeProjection(90.0f, fAspectRatio, fNear, fFar);
}


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

void execOncePerSec(){  // A function that will execute once every new second since startup

}

void MinuteTimer(){
    framesElapsedSinceStartup++;

    if(framesElapsedSinceStartup == targetFrameRate){
        secondsElapsedSinceStartup++;
        framesElapsedSinceStartup = 0;
        
        minutesElapsedSinceStartup = secondsElapsedSinceStartup / 60.0f;
        hoursElapsedSinceStartup = minutesElapsedSinceStartup / 60.0f;
        daysElapsedSinceStartup = hoursElapsedSinceStartup / 24.0f;

        execOncePerSec();
    }
}

void PrintDebugInfo(){
    if(debugModeTogggled) {
    std::cout << "Targets: \n" << "deltaTime: " << (1 / targetFrameRate) << "\n" << "frameRate: " << targetFrameRate << "\n";

    std::cout << "Real measurements: \n" << "frameRate: " << realFrameRate << "\n" << "deltaTime: " << deltaTime << "\n";

    std::cout << "Timing functions: \n" << "frameProgressToSecondTimer: " << framesElapsedSinceStartup << "\n";
    std::cout << "secondsElapsedSinceStartup: " << secondsElapsedSinceStartup << "\n";
    std::cout << "minutesElapsedSinceStartup: " << minutesElapsedSinceStartup << "\n";
    std::cout << "hoursElapsedSinceStartup: " << hoursElapsedSinceStartup << "\n";
    std::cout << "daysElapsedSinceStartup: " << daysElapsedSinceStartup << "\n\n";

    std::cout << "Draw Cycles completed: " << nDrawCycles << "\n\n";

    std::cout << "Camera Position:\n X: " << vCamera.x << "\n" << " Y: " << vCamera.y << "\n";
    std::cout << "\033[2J\033[1;1H"; //ANSI CODES to clear console screen
    
    }
}

vec2d ProjectToScreen(const vec3d &v) {
    return {
        (v.x + 1.0f) * 0.5f * (float)windowWidth,                 // X mapped to screen width
        (1.0f - (v.y + 1.0f) * 0.5f) * (float)windowHeight       // Y flipped & mapped to screen height
    };
}

bool IsTriangleInView(const triangle &tri) {
    // Check if any vertex is inside NDC cube
    for(int i=0; i<3; i++){
        if(tri.p[i].x >= -1.0f && tri.p[i].x <= 1.0f &&
           tri.p[i].y >= -1.0f && tri.p[i].y <= 1.0f &&
           tri.p[i].z >= 0.0f  && tri.p[i].z <= 1.0f) {
            return true; // At least one vertex is inside view
        }
    }
    return false; // Fully outside
}

struct Plane {
    vec3d point;
    vec3d normal;
};

std::vector<Plane> gFrustumPlanes;

void UpdateFrustumPlanes() {
    gFrustumPlanes.clear();

    float nearHeight = 2.0f * tanf(fFov * 0.5f * 3.14159265f / 180.0f) * fNear;
    float nearWidth  = nearHeight / fAspectRatio;

    float farHeight  = 2.0f * tanf(fFov * 0.5f * 3.14159265f / 180.0f) * fFar;
    float farWidth   = farHeight / fAspectRatio;

    // Near plane
    gFrustumPlanes.push_back({ {0,0,fNear}, {0,0,1} });
    // Far plane
    gFrustumPlanes.push_back({ {0,0,fFar}, {0,0,-1} });

    // Right plane
    gFrustumPlanes.push_back({ {0,0,0}, Vector_Normalise({fNear/2,0,fNear}) });
    // Left plane
    gFrustumPlanes.push_back({ {0,0,0}, Vector_Normalise({-fNear/2,0,fNear}) });
    // Top plane
    gFrustumPlanes.push_back({ {0,0,0}, Vector_Normalise({0,nearHeight/2,fNear}) });
    // Bottom plane
    gFrustumPlanes.push_back({ {0,0,0}, Vector_Normalise({0,-nearHeight/2,fNear}) });
}

std::vector<triangle> ClipTriangleToFrustumOptimized(const triangle &tri) {
    std::vector<triangle> clippedTris;
    clippedTris.push_back(tri);

    for(const auto &plane : gFrustumPlanes) {
        std::vector<triangle> newTris;
        for(auto &t : clippedTris) {
            triangle t1, t2;
            int n = Triangle_ClipAgainstPlane(plane.point, plane.normal, t, t1, t2);
            if(n == 1) newTris.push_back(t1);
            if(n == 2){ newTris.push_back(t1); newTris.push_back(t2); }
        }
        clippedTris = newTris;
        if(clippedTris.empty()) break; // Fully outside
    }

    return clippedTris;
}

void RenderObject(SDL_Renderer* renderer, Object3D &obj, const mat4x4 &matView, const mat4x4 &matProj) {
    std::vector<triangle> vecTrianglesToRaster;

    mat4x4 matWorld = obj.GetWorldMatrix();

    for(const auto &tri : obj.meshData.tris) {
        triangle triTransformed, triViewed;

        // 1. Transform to world space
        triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
        triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
        triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

        // 2. Transform to view space
        triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
        triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
        triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

        // 3. Backface culling
        vec3d normal = Vector_CrossProduct(
            Vector_Sub(triViewed.p[1], triViewed.p[0]),
            Vector_Sub(triViewed.p[2], triViewed.p[0])
        );
        normal = Vector_Normalise(normal);

        if(gCullMode == CullMode::Back && Vector_DotProduct(normal, triViewed.p[0]) >= 0.0f) continue;
        if(gCullMode == CullMode::Front && Vector_DotProduct(normal, triViewed.p[0]) < 0.0f) continue;

        // 4. Clip triangle against frustum planes
        auto clippedTris = ClipTriangleToFrustumOptimized(triViewed);

        // 5. Project and add to raster list
        for(auto &triProj : clippedTris) {
            triangle triProjected;
            triProjected.p[0] = Matrix_MultiplyVector(matProj, triProj.p[0]);
            triProjected.p[1] = Matrix_MultiplyVector(matProj, triProj.p[1]);
            triProjected.p[2] = Matrix_MultiplyVector(matProj, triProj.p[2]);

            // Perspective divide
            for(int i=0; i<3; i++){
                triProjected.p[i].x /= triProjected.p[i].w;
                triProjected.p[i].y /= triProjected.p[i].w;
                triProjected.p[i].z /= triProjected.p[i].w;
            }

            vecTrianglesToRaster.push_back(triProjected);
        }
    }

    // 6. Depth sort
    std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](const triangle &t1, const triangle &t2){
        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
        return z1 > z2;
    });

    // 7. Rasterize
    for(const auto &t : vecTrianglesToRaster){
        vec2d p0 = ProjectToScreen(t.p[0]);
        vec2d p1 = ProjectToScreen(t.p[1]);
        vec2d p2 = ProjectToScreen(t.p[2]);

        if(debugModeTogggled){
            SDL_RenderLine(renderer, p0.x, p0.y, p1.x, p1.y);
            SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
            SDL_RenderLine(renderer, p2.x, p2.y, p0.x, p0.y);
        } else {
            drawFilledTriangle(renderer, p0, p1, p2);
        }
    }
}
