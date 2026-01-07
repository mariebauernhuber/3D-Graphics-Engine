#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>


bool fullDebugMode = false;

unsigned long long nDrawCycles = 0;

static SDL_Window* window;
static SDL_Renderer* renderer;

struct vec3d{ float x,y,z,w = 1; };
struct vec2d{ float x,y; };
struct triangle{ vec3d p[3]; };
struct mesh{ 
    std::vector<triangle> tris; 

    bool LoadFromObjectFile(std::string sFilename){
        if(fullDebugMode){std::cout << "Begin: LoadFromObjectFile\n";}
        if(fullDebugMode){std::cout << "File: " << sFilename << "\n";}
        std::ifstream f(sFilename);
        if(!f.is_open()){ return false; }
        if(fullDebugMode){std::cout << "File opened!\n";}

        // Local cache or verts
        std::vector<vec3d> verts;
        if(fullDebugMode){std::cout << "Vert cache created from Object File!\n";}

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
        if(fullDebugMode){std::cout << "Vert cache filled!\n";}
        if(fullDebugMode){std::cout << "End: LoadFromObjectFile\n\n";}
        return true;
    }
};
struct mat4x4{ float m[4][4] = { 0 }; };

mesh meshCube;
mesh meshCube2;

vec3d meshPosition = {5.0f, 2.0f, 10.0f};
vec3d meshPosition2 = {20.0f, 20.0f, 20.0f};

vec3d vCamera = { 0.0f, 0.0f, 0.0f };
vec3d vLookDir;

float fYaw   = 0.0f;   // left/right
float fPitch = 0.0f;   // up/down
float fMaxPitch = 1.55f;
float fMouseSensitivity = 0.0025f;

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

bool consoleOpen = false;

bool debugModeTogggled = false;

vec3d Matrix_MultiplyVector(const mat4x4 &m, const vec3d &i){
    if(fullDebugMode){std::cout << "Begin: Matrix_MultiplyVector\n";}
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    if(fullDebugMode){std::cout << "Function returned: \n";}
    if(fullDebugMode){std::cout << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "!\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MultiplyVector\n\n";}
    return v;
}

mat4x4 Matrix_MakeIdentity(){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeIdentity\n";}
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "Matrix with {1,1,1,1} applied.\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeIdentity\n\n";}
    return matrix;
}

vec3d Vector_Add(const vec3d &v1, const vec3d &v2) {
    if(fullDebugMode){std::cout << "Begin: Vector_Add\n";}
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(fullDebugMode){std::cout << "x: " << v1.x << " + " << v2.x << " = " << v1.x + v2.x << "!\n";}
    if(fullDebugMode){std::cout << "y: " << v1.y << " + " << v2.y << " = " << v1.y + v2.y << "!\n";}
    if(fullDebugMode){std::cout << "z: " << v1.z << " + " << v2.z << " = " << v1.z + v2.z << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_Add\n\n";}
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d Vector_Sub(const vec3d &v1, const vec3d &v2) {
    if(fullDebugMode){std::cout << "Begin: Vector_Sub\n";}
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(fullDebugMode){std::cout << "x: " << v1.x << " - " << v2.x << " = " << v1.x - v2.x << "!\n";}
    if(fullDebugMode){std::cout << "y: " << v1.y << " - " << v2.y << " = " << v1.y - v2.y << "!\n";}
    if(fullDebugMode){std::cout << "z: " << v1.z << " - " << v2.z << " = " << v1.z - v2.z << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_Sub\n\n";}
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3d Vector_Mul(const vec3d &v1, float k) {
    if(fullDebugMode){std::cout << "Begin: Vector_Mul\n";}
    if(fullDebugMode){std::cout << "Operation Details: \n";}
    if(fullDebugMode){std::cout << "Float K = " << k << "!\n";}
    if(fullDebugMode){std::cout << "x: " << v1.x << " * " << k << " = " << v1.x * k << "!\n";}
    if(fullDebugMode){std::cout << "y: " << v1.y << " * " << k << " = " << v1.y * k << "!\n";}
    if(fullDebugMode){std::cout << "z: " << v1.z << " * " << k << " = " << v1.z * k << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_Mul\n\n";}
    return { v1.x * k, v1.y * k, v1.z * k };
}

vec3d Vector_Div(const vec3d &v1, float k) {
    if(fullDebugMode){std::cout << "Begin: Vector_Div\n";}
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(fullDebugMode){std::cout << "Float K = " << k << "!\n";}
    if(k != 0){
        if(fullDebugMode){std::cout << "x: " << v1.x << " / " << k << " = " << v1.x / k << "!\n";}
        if(fullDebugMode){std::cout << "y: " << v1.y << " / " << k << " = " << v1.y / k << "!\n";}
        if(fullDebugMode){std::cout << "z: " << v1.z << " / " << k << " = " << v1.z / k << "!\n";}
        if(fullDebugMode){std::cout << "End: Vector_Div\n\n";}
        return { v1.x / k, v1.y / k, v1.z / k };
    }else{
        if(fullDebugMode){std::cout << "ERROR: DEVIDING BY ZERO";}
        if(fullDebugMode){std::cout << "End: Vector_Div\n\n";}
        return {0.0f, 0.0f, 0.0f};
    }
}

float Vector_DotProduct(const vec3d &v1, const vec3d &v2){
    if(fullDebugMode){std::cout << "Begin: Vector_DotProduct\n";}
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(fullDebugMode){std::cout << "(" << v1.x << " * " << v2.x << ") + ";}
    if(fullDebugMode){std::cout << "(" << v1.y << " * " << v2.y << ") + ";}
    if(fullDebugMode){std::cout << "(" << v1.z << " * " << v2.z << ") = ";}
    if(fullDebugMode){std::cout << (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_DotProduct\n\n";}
    return ( (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) );
}

float Vector_Length(const vec3d &v){
    if(fullDebugMode){std::cout << "Begin: Vector_Length\n";}
    if(fullDebugMode){std::cout << "Calling Vector_DotProduct() to get dot product!\n";}
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(fullDebugMode){std::cout << "sqrt(" << Vector_DotProduct(v, v) << ") = " << sqrtf(Vector_DotProduct(v, v)) << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_Length\n\n";}
    return sqrtf(Vector_DotProduct(v, v));
}

vec3d Vector_Normalise(const vec3d &v) {
    if(fullDebugMode){std::cout << "Begin: Vector_Normalise\n";}
    if(fullDebugMode){std::cout << "Calling Vector_Length() to get Length\n";}
    float l = Vector_Length(v);
    if(fullDebugMode){std::cout << "Operation details: \n";}
    if(l!=0){ 
        if(fullDebugMode){std::cout << "x: " << v.x << " / " <<  l << " = " << v.x / l << "!\n";}
        if(fullDebugMode){std::cout << "y: " << v.y << " / " <<  l << " = " << v.y / l << "!\n";}
        if(fullDebugMode){std::cout << "z: " << v.z << " / " <<  l << " = " << v.z / l << "!\n";}
        return { v.x / l, v.y / l, v.z / l };
        if(fullDebugMode){std::cout << "End: Vector_Normalise\n\n";}
    }else{
        if(fullDebugMode){std::cout << "ERROR: DEVIDING BY ZERO\n";}
        return { 0.0f, 0.0f, 0.0f };
        if(fullDebugMode){std::cout << "End: Vector_Normalise\n\n";}
    }
}

vec3d Vector_CrossProduct(const vec3d &v1, const vec3d &v2){
    if(fullDebugMode){std::cout << "Begin: Vector_CrossProduct\n";}
    vec3d v;
    if(fullDebugMode){std::cout << "Created Vector3D v\n";}
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    if(fullDebugMode){std::cout << "Operation Details: \n";}
    if(fullDebugMode){std::cout << "x: " << v1.y << " * " << v2.z << " - " << v1.z << " * " << v2.y << " = " << v1.y * v2.z - v1.z * v2.y << "!\n";}
    if(fullDebugMode){std::cout << "y: " << v1.z << " * " << v2.x << " - " << v1.x << " * " << v2.z << " = " << v1.z * v2.x - v1.x * v2.z << "!\n";}
    if(fullDebugMode){std::cout << "z: " << v1.x << " * " << v2.y << " - " << v1.y << " * " << v2.x << " = " << v1.x * v2.y - v1.y * v2.x << "!\n";}
    if(fullDebugMode){std::cout << "End: Vector_CrossProduct\n\n";}
    return v;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeRotationX\n";}
    mat4x4 matrix;
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[1][2] = sinf(fAngleRad);
    matrix.m[2][1] = -sinf(fAngleRad);
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "Filled matrix\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeRotationX\n\n";}
    return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeRotationY\n";}
    mat4x4 matrix;
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][2] = sinf(fAngleRad);
    matrix.m[2][0] = -sinf(fAngleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "Filled Matrix\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeRotationY\n\n";}
    return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeRotationZ\n";}
    mat4x4 matrix;
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][1] = sinf(fAngleRad);
    matrix.m[1][0] = -sinf(fAngleRad);
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "Filled matrix\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeRotationZ\n\n";}
    return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeTranslation\n";}
    mat4x4 matrix;
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    if(fullDebugMode){std::cout << "Filled matrix\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeTranslation\n\n";}
    return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar){
    if(fullDebugMode){std::cout << "Begin: Matrix_MakeProjection\n";}
    if(fullDebugMode){std::cout << "Inputs: \nfFovDegrees: " << fFovDegrees << "\nfAspectRatio: " << fAspectRatio << "\nfNear: " << fNear << "\nfFar: " << fFar;}
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    if(fullDebugMode){std::cout << "Calculation: fFovRad: " << fFovRad << "!\n";}
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    mat4x4 matrix;
    matrix.m[0][0] = fAspectRatio * fFovRad;
    matrix.m[1][1] = fFovRad;
    matrix.m[2][2] = fFar / (fFar - fNear);
    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    if(fullDebugMode){std::cout << "Filled matrix\n";}
    if(fullDebugMode){std::cout << "End: Matrix_MakeProjection\n\n";}
    return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2){
    if(fullDebugMode){std::cout << "Begin: Matrix_MultiplyMatrix\n";}
    mat4x4 matrix;
    if(fullDebugMode){std::cout << "Made mat4x4 matrix\n";}
    for (int c = 0; c < 4; c++){
        for (int r = 0; r < 4; r++){
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
        }
    }
    if(fullDebugMode){std::cout << "End: Matrix_MultiplyMatrix\n\n";}
    return matrix;
}

mat4x4 Matrix_PointAt(const vec3d &pos, const vec3d &target, const vec3d &up){
    if(fullDebugMode){std::cout << "Begin: Matrix_PointAt\n";}
    // Calculate new forwards direction
    vec3d newForward = Vector_Sub(target, pos);
    newForward = Vector_Normalise(newForward);

    // calculate new up direction
    vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
    vec3d newUp = Vector_Sub(up, a);
    newUp = Vector_Normalise(newUp);

    vec3d newRight = Vector_CrossProduct(newUp, newForward);

    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "End: Matrix_PointAt\n\n";}
    return matrix;
}

// Only for Rotation/Translation Matrices
mat4x4 Matrix_QuickInverse(mat4x4 &m){
    if(fullDebugMode){std::cout << "Begin: Matrix_QuickInverse\n";}
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
    if(fullDebugMode){std::cout << "End: Matrix_QuickInverse\n\n";}
    return matrix;
}

vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd){
    if(fullDebugMode){std::cout << "Begin: Vector_IntersectPlane\n";}
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);
    float ad = Vector_DotProduct(lineStart, plane_n);
    float bd = Vector_DotProduct(lineEnd, plane_n);
    float t = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
    vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
    if(fullDebugMode){std::cout << "End: Vector_IntersectPlane\n\n";}
    return Vector_Add(lineStart, lineToIntersect);
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n,
                              triangle &in_tri,
                              triangle &out_tri1,
                              triangle &out_tri2)
{
    plane_n = Vector_Normalise(plane_n);

    auto dist = [&](const vec3d &p){
        return Vector_DotProduct(plane_n, p)
             - Vector_DotProduct(plane_n, plane_p);
    };

    vec3d* inside_points[3];
    int nInsidePointCount = 0;
    vec3d* outside_points[3];
    int nOutsidePointCount = 0;

    float d0 = dist(in_tri.p[0]);
    float d1 = dist(in_tri.p[1]);
    float d2 = dist(in_tri.p[2]);

    if (d0 >= 0) inside_points[nInsidePointCount++] = &in_tri.p[0];
    else         outside_points[nOutsidePointCount++] = &in_tri.p[0];

    if (d1 >= 0) inside_points[nInsidePointCount++] = &in_tri.p[1];
    else         outside_points[nOutsidePointCount++] = &in_tri.p[1];

    if (d2 >= 0) inside_points[nInsidePointCount++] = &in_tri.p[2];
    else         outside_points[nOutsidePointCount++] = &in_tri.p[2];

    if (nInsidePointCount == 0)
        return 0;

    if (nInsidePointCount == 3) {
        out_tri1 = in_tri;
        return 1;
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n,
                                              *inside_points[0],
                                              *outside_points[0]);
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n,
                                              *inside_points[0],
                                              *outside_points[1]);
        return 1;
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n,
                                              *inside_points[0],
                                              *outside_points[0]);

        out_tri2.p[0] = *inside_points[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n,
                                              *inside_points[1],
                                              *outside_points[0]);
        return 2;
    }

    return 0;
}


void drawFilledTriangle(SDL_Renderer* renderer, vec2d p0, vec2d p1, vec2d p2) {
    if(fullDebugMode){std::cout << "Begin: drawFilledTriangle\n";}
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
    if(fullDebugMode){std::cout << "End: drawFilledTriangle\n\n";}
}


void CalculateScreenTransforms(){
    if(fullDebugMode){std::cout << "Begin: CalculateScreenTransforms\n";}
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    fAspectRatio = (float)windowHeight / (float)windowWidth;
    fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159265358979323f);
    if(fullDebugMode){std::cout << "End: CalculateScreenTransforms\n\n";}
}


mat4x4 matView, matProj;

void CalculateScreenProjection(){
    if(fullDebugMode){std::cout << "Begin: CalculateScreenProjection\n";}
    matProj = Matrix_MakeProjection(90.0f, fAspectRatio, fNear, fFar);
    if(fullDebugMode){std::cout << "End: CalculateScreenProjection\n\n";}
}
// Basic function to calculate deltatime (used to make things independant from framerate), run every frame
void CalculateDeltaTime(){ 
    if(fullDebugMode){std::cout << "Begin: CalculateDeltaTime\n";}
    static Uint64 lastCounter = 0;

    Uint64 currentCounter = SDL_GetPerformanceCounter();
    Uint64 frequency = SDL_GetPerformanceFrequency();

    deltaTime = 0.0f;

    if (lastCounter != 0)
    {
        deltaTime = (float)(currentCounter - lastCounter) / (float)frequency;
    }

    lastCounter = currentCounter;
    if(fullDebugMode){std::cout << "End: CalculateDeltaTime\n\n";}
}

void execOncePerSec(){  // A function that will execute once every new second since startup
    if(fullDebugMode){std::cout << "Begin: execOncePerSec\n";}

    if(fullDebugMode){std::cout << "End: execOncePerSec\n\n";}
}

unsigned long long framesElapsedSinceStartup;
unsigned long long secondsElapsedSinceStartup;
float minutesElapsedSinceStartup;
float hoursElapsedSinceStartup;
float daysElapsedSinceStartup;

void MinuteTimer(){
    if(fullDebugMode){std::cout << "Begin: MinuteTimer\n";}
    framesElapsedSinceStartup++;

    if(framesElapsedSinceStartup == targetFrameRate){
        secondsElapsedSinceStartup++;
        framesElapsedSinceStartup = 0;
        
        minutesElapsedSinceStartup = secondsElapsedSinceStartup / 60.0f;
        hoursElapsedSinceStartup = minutesElapsedSinceStartup / 60.0f;
        daysElapsedSinceStartup = hoursElapsedSinceStartup / 24.0f;

        execOncePerSec();
    }
    if(fullDebugMode){std::cout << "End: MinuteTimer\n\n";}
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

enum class CullMode {
    None,
    Back,
    Front
};

CullMode gCullMode = CullMode::Back;

struct Object3D {
    mesh meshData;
    vec3d position;
    vec3d rotation; // Euler angles
    vec3d scale = {1, 1, 1};

    mat4x4 GetWorldMatrix() const {
        mat4x4 matRotX = Matrix_MakeRotationX(rotation.x);
        mat4x4 matRotY = Matrix_MakeRotationY(rotation.y);
        mat4x4 matRotZ = Matrix_MakeRotationZ(rotation.z);

        mat4x4 matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
        matWorld = Matrix_MultiplyMatrix(matWorld, matRotY);
        mat4x4 matTrans = Matrix_MakeTranslation(position.x, position.y, position.z);
        matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);
        return matWorld;
    }
};


std::vector<Object3D> objects;


vec2d ProjectToScreen(const vec3d &v) {
    return {
        (v.x + 1.0f) * 0.5f * (float)windowWidth,                 // X mapped to screen width
        (1.0f - (v.y + 1.0f) * 0.5f) * (float)windowHeight       // Y flipped & mapped to screen height
    };
}

// Define a simple struct for a plane in view space

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

void RenderObject(SDL_Renderer* renderer, const Object3D &obj, const mat4x4 &matView, const mat4x4 &matProj) {
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


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        return SDL_APP_FAILURE;
    }
    window = SDL_CreateWindow("Test", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_RELATIVE_MODE | SDL_WINDOW_MOUSE_GRABBED);
    renderer = SDL_CreateRenderer(window, NULL);

    CalculateScreenTransforms();
    CalculateScreenProjection();
   
    meshCube.LoadFromObjectFile("src/VideoShip.obj");
    meshCube2.LoadFromObjectFile("src/VideoShip.obj");

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

    objects.push_back(ship);
    objects.push_back(ship2);
    objects.push_back(ship3);
    objects.push_back(movingShip);

    SDL_SetWindowRelativeMouseMode(window, true);

    mat4x4 matRotZ = Matrix_MakeIdentity();
    mat4x4 matRotX = Matrix_MakeIdentity();
    mat4x4 matRotY = Matrix_MakeIdentity();
    mat4x4 matTrans = Matrix_MakeIdentity();
    mat4x4 matWorld = Matrix_MakeIdentity();

    std::cout << "Init done!";

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
        if (event->key.scancode == SDL_SCANCODE_F6) {
            if (gCullMode == CullMode::Back) gCullMode = CullMode::Front;
            else if (gCullMode == CullMode::Front) gCullMode = CullMode::None;
            else gCullMode = CullMode::Back;
        }

    }

    if(event->type == SDL_EVENT_MOUSE_MOTION){
        fYaw   += event->motion.xrel * fMouseSensitivity;
        fPitch -= event->motion.yrel * fMouseSensitivity;

        // Clamp pitch to avoid flipping
        if (fPitch > fMaxPitch)  fPitch = fMaxPitch;
        if (fPitch < -fMaxPitch) fPitch = -fMaxPitch;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
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
        vCamera = Vector_Add(vCamera, Vector_Mul(vLookDir, fMoveSpeed));
    if (key_states[SDL_SCANCODE_S])
        vCamera = Vector_Sub(vCamera, Vector_Mul(vLookDir, fMoveSpeed));

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

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result){
    SDL_Quit();
}

