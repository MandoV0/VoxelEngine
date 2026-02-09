#pragma once

#define GLFW_INCLUDE_NONE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <memory>
#include "GBuffer.h"
#include "ShadowMap.h"


class Renderer;
class Camera;
class Input;
class World;
class Skybox;
class Shader;
class Texture;

class Game
{
public:
    Game(int width, int height, const char* title);
    ~Game();

    void Run();

private:
    void Init();
    void InitImGui();
    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();
    void RenderImGui();
    void Shutdown();

    GLFWwindow* m_Window;
    int m_Width;
    int m_Height;

    // Core
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<Input> m_Input;
    std::unique_ptr<World> m_World;
    std::unique_ptr<Skybox> m_Skybox;

	// Shaders & Textures
    std::unique_ptr<Shader> m_WorldShader;
    std::unique_ptr<Shader> m_SkyboxShader;
    std::unique_ptr<Shader> m_CutoutShader;
    std::unique_ptr<Shader> m_WaterShader;
    std::unique_ptr<Shader> m_FogShader;

    std::unique_ptr<Texture> m_AtlasTexture;
    std::unique_ptr<Texture> m_NormalMapTexture;

    // Camera Matrix
    glm::mat4 m_Projection;
    glm::mat4 m_Model;

    // Game state
    float m_FOV;
    int m_RenderDistance;
    float m_ClickTimer;
    float m_ClickCooldown;
    glm::ivec3 m_HitBlock;
    glm::ivec3 m_PlaceBlock;

    // Times
    float m_LastFrame;
    float m_DeltaTime;

    bool m_CursorLocked = true;

    glm::vec3 m_FogColor = glm::vec3(0.5f, 0.6f, 0.7f);
    float m_FogDensity = 0.015f;
	float m_FogFalloff = 0.12f;
	float m_FogHeight = 64.0f;
    int m_FogMode = 1; // 0 = exp, 1 = height exp

	    // Deferred Rendering
	    std::unique_ptr<GBuffer> m_GBuffer;
	    std::unique_ptr<Shader> m_LightingShader;
	
	    // SSGI
	    std::unique_ptr<Shader> m_SSGIShader;
	    unsigned int m_SSGIFBO, m_SSGIColorBuffer;
	
	    // Shadows
	    std::unique_ptr<ShadowMap> m_ShadowMap;
    std::unique_ptr<Shader> m_ShadowShader;
    glm::mat4 m_LightSpaceMatrix;
    int m_ShadowResolution = 4096;
};