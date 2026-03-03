#include "../include/editor.hpp"
#include <SDL3/SDL_init.h>
#include "../imgui/imgui.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

extern float deltaTime;
extern float deltaTimeMod;
extern float newDeltaTime;
extern float targetFrameRate;
extern float realFrameRate;
extern bool is_running;
extern GLuint fbo, textureColorBuffer;
extern vec3d vCamera;
extern mat4x4 matView, matProj;
extern vec3d clearColor;
glm::mat4 gridMatrix = glm::mat4(1.0f); 
bool isImGuiGameViewportInFocus = false;
bool isImGuiGameViewportInMouseLock = false;
extern float secondsElapsedSinceStartup;
extern float secondTiming;
extern vec3d playerMovement;
extern bool cullingEnabled;

long unsigned int selectedIndex = 0;

void DrawObjectEditor(std::vector<Object3D>& objects) {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        ImGui::Begin("Game Viewport (press F1 to toggle focus)");
    	ImGui::Image(textureColorBuffer, ImVec2(ImGui::GetContentRegionAvail().x, (ImGui::GetContentRegionAvail().x /16)*9), ImVec2(0, 0), ImVec2(1, -1));
        ImGui::End();
    
        ImGui::Begin("Temp window");
        ImGui::End();
    
        ImGui::Begin("Scene tree");
        // Object selector listbox
        ImGui::BeginListBox(" ", ImVec2(500, 1000));
        for (long unsigned int i = 0; i < objects.size(); ++i) {
            std::string displayName = "Unnamed Object " + std::to_string(i);
            auto nameIt = objects[i].properties.find("name");
            if (nameIt != objects[i].properties.end()) {
                displayName = nameIt->second;
            }
            if (ImGui::Selectable(displayName.c_str(), selectedIndex == i)) {
                selectedIndex = 0;
            }
        }
        ImGui::EndListBox();
        ImGui::End();
    
	// GENERAL SETTINGS WINDOW
        ImGui::Begin("Settings", NULL, ImGuiViewportFlags());

    	if(ImGui::Button("Exit App")){
		is_running = false;
	}

	if(ImGui::Button("Toggle Culling")){
		cullingEnabled = !cullingEnabled;
	}

	ImGui::DragFloat3("Clearcolor", &clearColor.x);

    	ImGui::DragFloat("deltaTimeForCalc", &newDeltaTime);
    	ImGui::DragFloat("Actual deltaTime", &deltaTime);
	ImGui::DragFloat("DeltaTime Modifier", &deltaTimeMod);
	ImGui::DragFloat("Target Framerate", &targetFrameRate);
	ImGui::DragFloat("SecondsElapsed", (float*)&secondTiming);
	ImGui::DragFloat("frameRate", (float*)&realFrameRate);
	ImGui::End();

	// OBJECT EDITOR
	ImGui::Begin("Object Editor", NULL, ImGuiViewportFlags());
		int x = selectedIndex;
		ImGui::DragInt("SelectedIndex", &x, 1);
		selectedIndex = x;
		if (ImGui::CollapsingHeader("Object3D Editor")) {
		// Object selector listbox
		//
		if (ImGui::Button("Add Object")) {
		    objects.emplace_back();
		    selectedIndex = 0;
		}
		if (ImGui::Button("Delete Selected") && objects.size() > 0) {
		    objects.erase(objects.begin() + selectedIndex);
			selectedIndex = 0;
		    }

        // Edit selected object
        if (objects.size() >= 1) {
            // Transform editing section
            if (ImGui::CollapsingHeader("Transform")) {
                ImGui::DragFloat3("Position", &objects[selectedIndex].position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &objects[selectedIndex].rotation.x, 0.1f);
                ImGui::DragFloat3("Rotation / Tick", &objects[selectedIndex].rotationPerTick.x, 0.1f);
                ImGui::DragFloat3("Position / Tick", &objects[selectedIndex].positionPerTick.x, 0.1f);
                ImGui::DragFloat3("Scale", &objects[selectedIndex].scale.x, 0.1f);
		ImGui::DragFloat3("FarthestPos", &objects[selectedIndex].farthestPoint.x, 1.0f);
		if(ImGui::Button("GL_BACK")){objects[selectedIndex].cullingMode = GL_BACK;}
		if(ImGui::Button("GL_FRONT")){objects[selectedIndex].cullingMode = GL_FRONT;}
		if(ImGui::Button("Both")){objects[selectedIndex].cullingMode = GL_FRONT_AND_BACK;}
		if(ImGui::Button("GL_CCW")){objects[selectedIndex].cullingFrontFace = GL_CCW;}
		if(ImGui::Button("GL_CW")){objects[selectedIndex].cullingFrontFace = GL_CW;}
            }
        } else {
            ImGui::Text("No objects available");
        }
    }

	if(ImGui::CollapsingHeader("Camera and Player Settings")){
		ImGui::DragFloat3("Camera Pos", &vCamera.x, 0.1f);
		ImGui::DragFloat3("Player Movement", &playerMovement.x, 0.1f);
	}

	ImGui::End();
}
