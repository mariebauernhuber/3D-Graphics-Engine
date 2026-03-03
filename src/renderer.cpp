#include "../include/renderer.hpp"
#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <vector>
#include <cmath>
#include "../include/mesh.hpp"
#include <SDL3/SDL.h>
#include "stb_image.h"

bool debugModeTogggled = false;
extern unsigned long long nObjRenderCycles;
extern unsigned long long nTrisPushedBack;
unsigned long long nDrawCycles = 0;

extern std::vector<Object3D> objects;

extern int windowWidth, windowHeight;
float fTheta;
float fNear = 0.1f;
float fFar = 1000.0f;
float fFov = 90.0f;
float fFovRad;
float fAspectRatio;
float deltaTime;
float deltaTimeMod = 1;
float newDeltaTime;
float secondTiming = 0;

float targetFrameRate = 60;
float realFrameRate;

vec3d vCamera = { 0.0f, 0.0f, 0.0f };
vec3d vLookDir;

unsigned long long framesElapsedSinceStartup;
float secondsElapsedSinceStartup;
float minutesElapsedSinceStartup;
float hoursElapsedSinceStartup;
float daysElapsedSinceStartup;

mat4x4 matView, matProj;

mat4x4 matRotZ = Matrix_MakeIdentity();
mat4x4 matRotX = Matrix_MakeIdentity();
mat4x4 matRotY = Matrix_MakeIdentity();
mat4x4 matTrans = Matrix_MakeIdentity();
mat4x4 matWorld = Matrix_MakeIdentity();

CullMode gCullMode = CullMode::Back;

extern int targetWindowWidth, targetWindowHeight;

void CalculateScreenTransforms(SDL_Window* window){
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    SDL_GetWindowSize(window, &targetWindowWidth, &targetWindowHeight);
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

    secondTiming = SDL_GetTicks() / 1000.0f;
    if(secondTiming > 1000.0f){
	    secondTiming = 0.0f;
    }

    deltaTime = 0.0f;

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

vec2d ProjectToScreen(const vec3d &v) {
    // v.x and v.y are in range [-1, 1] after the perspective divide
    return {
        (v.x + 1.0f) * 0.5f * (float)windowWidth,          // Maps -1..1 to 0..Width
        (1.0f - (v.y + 1.0f) * 0.5f) * (float)windowHeight // Maps -1..1 to 0..Height (inverted Y)
    };
}

glm::mat4 ToGLM(const mat4x4& yourMat) {
    // GLM is column-major, so transpose your row-major data
    return glm::mat4(
        yourMat.m[0][0], yourMat.m[1][0], yourMat.m[2][0], yourMat.m[3][0],  // col 0
        yourMat.m[0][1], yourMat.m[1][1], yourMat.m[2][1], yourMat.m[3][1],  // col 1  
        yourMat.m[0][2], yourMat.m[1][2], yourMat.m[2][2], yourMat.m[3][2],  // col 2
        yourMat.m[0][3], yourMat.m[1][3], yourMat.m[2][3], yourMat.m[3][3]   // col 3
    );
}

GLuint whiteTexture = 0;  // global/init once

void InitDefaultTexture() {
    if (whiteTexture) return;
    
    unsigned char white[] = {255, 255, 255, 255};  // 1x1 RGBA white
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

//The main render function.
//This will draw the objects mesh and texturemesh from its VAO buffer to the main GL_TEXTURE0, which can then later be either drawn to a screen quad or some other textured object.
void RenderObjectAssimp(Object3D& obj, int ID, GLuint shader, GLuint texture, const mat4x4 &matView, const mat4x4 &matProj, GLuint frontFace, GLuint cullMode){
    if(obj.properties.empty()){
	    return;
    }

    glUseProgram(shader);
    glFrontFace(objects[ID].cullingFrontFace);
    glCullFace(objects[ID].cullingFrontFace);
    // Use provided texture OR fallback
    GLuint texID = texture ? texture : whiteTexture;
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 pos;
    pos[0] = objects[ID].position.x;
    pos[1] = objects[ID].position.y;
    pos[2] = objects[ID].position.z;

    if(objects[ID].rotation.x == 360.0f){
	    objects[ID].rotation.x = 0.0f;
    }else if(objects[ID].rotation.x > 360.0f){
	    objects[ID].rotation.x = 0.0f + (objects[ID].rotation.x - 360.0f);
    }

    if(objects[ID].rotation.y == 360.0f){
	    objects[ID].rotation.y = 0.0f;
    }else if(objects[ID].rotation.y > 360.0f){
	    objects[ID].rotation.y = 0.0f + (objects[ID].rotation.y - 360.0f);
    }

    if(objects[ID].rotation.z == 360.0f){
	    objects[ID].rotation.z = 0.0f;
    }else if(objects[ID].rotation.z > 360.0f){
	    objects[ID].rotation.z = 0.0f + (objects[ID].rotation.z - 360.0f);
    }

    model = glm::translate(model, pos);
    model = glm::scale(model, {objects[ID].scale.x, objects[ID].scale.y, objects[ID].scale.z});
    model = glm::rotate(model, glm::radians(objects[ID].rotation.x), {1.0f * newDeltaTime, 0.0f, 0.0f});
    model = glm::rotate(model, glm::radians(objects[ID].rotation.y), {0.0f, 1.0f * newDeltaTime, 0.0f});
    model = glm::rotate(model, glm::radians(objects[ID].rotation.z), {0.0f, 0.0f, 1.0f * newDeltaTime});

    // 1. Pass transformation matrices as uniforms
    GLint modelLoc = glGetUniformLocation(shader, "model");
    GLint viewLoc = glGetUniformLocation(shader, "view");
    GLint projLoc = glGetUniformLocation(shader, "projection");

    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &matWorld.m[0][0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &matView.m[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &matProj.m[0][0]);

    glm::mat4 modelGLM = model;
    glm::mat4 viewGLM = ToGLM(matView);  
    glm::mat4 projGLM = ToGLM(matProj);
    glm::mat4 mvpGLM = projGLM * viewGLM * modelGLM;
    
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvpGLM));
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);
    
    glBindVertexArray(obj.meshData.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glDrawArrays(GL_TRIANGLES, 0, obj.meshData.tris.size() * 3);
    glBindTexture(GL_TEXTURE_2D, 0);
}


GLuint LoadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        SDL_Log("stb_image failed for %s", filename);
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return tex;
}
