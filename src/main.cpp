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
        return true;
        if(fullDebugMode){std::cout << "Vert cache filled!\n";}
        if(fullDebugMode){std::cout << "End: LoadFromObjectFile\n\n";}
    }
};
struct mat4x4{ float m[4][4] = { 0 }; };

mesh meshCube;
mat4x4 matProj;

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

float targetFrameRate = 120;
float realFrameRate;

float deltaTime;

bool debugModeTogggled = false;

vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i){
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

mat4x4 Matrix_PointAt(vec3d &pos, vec3d &target, vec3d &up){
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

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        return SDL_APP_FAILURE;
    }
    window = SDL_CreateWindow("Test", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_RELATIVE_MODE | SDL_WINDOW_MOUSE_GRABBED);
    renderer = SDL_CreateRenderer(window, NULL);

    CalculateScreenTransforms();
    CalculateScreenProjection();
   
    meshCube.LoadFromObjectFile("src/VideoShip.obj");

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
    }

    if(event->type == SDL_EVENT_MOUSE_MOTION){
        fYaw   += event->motion.xrel * fMouseSensitivity;
        fPitch += event->motion.yrel * fMouseSensitivity;

        // Clamp pitch to avoid flipping
        if (fPitch > fMaxPitch)  fPitch = fMaxPitch;
        if (fPitch < -fMaxPitch) fPitch = -fMaxPitch;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){


    CalculateDeltaTime();
    MinuteTimer();

    float remainingFrameTime = (1.0f/targetFrameRate) - deltaTime;
    if (remainingFrameTime > 0.001f){
        SDL_Delay(int(remainingFrameTime * 1000.0f));
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Fill black background
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Set up rotation matrices (stolen from thelonecoder, temporary)
    mat4x4 matRotZ, matRotX, matRotY;
    fTheta += 0.0f * deltaTime;

    matRotZ = Matrix_MakeRotationZ(fTheta);
    matRotX = Matrix_MakeRotationX(fTheta);
    matRotY = Matrix_MakeRotationY(fTheta);

    mat4x4 matTrans;
    matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

    mat4x4 matWorld;
    matWorld = Matrix_MakeIdentity();
    matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
    matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

    vLookDir.x = cosf(fPitch) * sinf(fYaw);
    vLookDir.y = sinf(fPitch);
    vLookDir.z = cosf(fPitch) * cosf(fYaw);
    vLookDir = Vector_Normalise(vLookDir);

    vec3d vUp = { 0,1,0 };
    vec3d vTarget = Vector_Add(vCamera, vLookDir);

    mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

    mat4x4 matView = Matrix_QuickInverse(matCamera);

    std::vector<triangle> vecTrianglesToRaster;

    for(auto tri : meshCube.tris) {
        triangle triProjected, triTransformed, triViewed;
        
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
            // Convert World Space into view space
            triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
            triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
            triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

            int nClippedTriangles = 0;
            triangle clipped[2] = { 0 };
            nClippedTriangles = Triangle_ClipAgainstPlane({0.0f, 0.0f, fNear}, {0.0f, 0.0f, 1.0f}, triViewed, clipped[0], clipped[1]);

            for (int n = 0; n < nClippedTriangles; n++){
                //Project into 2D space
                triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

                //Scale into view, normalise
                triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                //Offset
                vec3d vOffsetView = { 1, 1, 0 };
                triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
                triProjected.p[0].x *= 0.5f * (float)windowWidth;
                triProjected.p[0].y *= 0.5f * (float)windowHeight;
                triProjected.p[1].x *= 0.5f * (float)windowWidth;
                triProjected.p[1].y *= 0.5f * (float)windowHeight;
                triProjected.p[2].x *= 0.5f * (float)windowWidth;
                triProjected.p[2].y *= 0.5f * (float)windowHeight;

                vecTrianglesToRaster.push_back(triProjected);
            }
        }
    }

    std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2){
        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
        return z1 > z2;
    });

    for(auto &triToRaster : vecTrianglesToRaster){
        triangle clipped[2];
        std::list<triangle> listTriangles;
        listTriangles.push_back(triToRaster);
        int nNewTriangles = 1;
        
        for(int p = 0; p < 4; p++){
            int nTrisToAdd = 0;
            while (nNewTriangles > 0){
                triangle test = listTriangles.front();
                listTriangles.pop_front();
                nNewTriangles--;
                switch(p){
                    case 0:
                        nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                     test, clipped[0], clipped[1]);
                        break;
                    case 1:
                        nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)windowHeight - 1, 0.0f }, { 0.0f, -1.0f, 0.0f },
                                     test, clipped[0], clipped[1]); 
                        break;
                    case 2:
                        nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
                                     test, clipped[0], clipped[1]); 
                        break;
                    case 3:
                        nTrisToAdd = Triangle_ClipAgainstPlane({ (float)windowWidth - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
                                     test, clipped[0], clipped[1]); 
                        break;
                }
                for (int w = 0; w < nTrisToAdd; w++){
                    listTriangles.push_back(clipped[w]);
                }
            }
            nNewTriangles = listTriangles.size();
            if(fullDebugMode){std::cout << "Made triangle list\n";}
        };

        for (auto &t : listTriangles) {
            if(debugModeTogggled){
                SDL_RenderLine(
                renderer,
                t.p[0].x, t.p[0].y,
                t.p[1].x, t.p[1].y
                );

                SDL_RenderLine(
                renderer,
                t.p[1].x, t.p[1].y,
                t.p[2].x, t.p[2].y
                );

                SDL_RenderLine(
                renderer,
                t.p[2].x, t.p[2].y,
                t.p[0].x, t.p[0].y
                );
            }else{
                drawFilledTriangle(renderer,
                { t.p[0].x, t.p[0].y },
                { t.p[1].x, t.p[1].y },
                { t.p[2].x, t.p[2].y }
                );
            };
        }
        if(fullDebugMode){std::cout << "Finished drawing once\n\n";}
        nDrawCycles++;
    };
        

    const bool *key_states = SDL_GetKeyboardState(NULL);

    float fMoveSpeed = 8.0f * deltaTime;

    // Forward / Back
    if (key_states[SDL_SCANCODE_W]){
        vCamera = Vector_Add(vCamera, Vector_Mul(vLookDir, fMoveSpeed));
    }
    if (key_states[SDL_SCANCODE_S]){
        vCamera = Vector_Sub(vCamera, Vector_Mul(vLookDir, fMoveSpeed));
    }
    // Right / Left
    vec3d vRight = Vector_Normalise(Vector_CrossProduct(vLookDir, {0,1,0}));

    if (key_states[SDL_SCANCODE_D]){
        vCamera = Vector_Sub(vCamera, Vector_Mul(vRight, fMoveSpeed));
    }
    if (key_states[SDL_SCANCODE_A]){
        vCamera = Vector_Add(vCamera, Vector_Mul(vRight, fMoveSpeed));
    }
    if (key_states[SDL_SCANCODE_SPACE]){
        vCamera.y -= 8.0f * deltaTime;
    }
    if (key_states[SDL_SCANCODE_LSHIFT]){
        vCamera.y += 8.0f * deltaTime;
    }

    SDL_RenderPresent(renderer);
    
    realFrameRate = (1.0f / deltaTime);

    if(debugModeTogggled){
        PrintDebugInfo();
    };

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
    SDL_Quit();
}

