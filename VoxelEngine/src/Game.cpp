#include "Game.h"
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "world/World.h"
#include "world/Skybox.h"
#include "Shader.h"
#include "texture.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <gtc/matrix_transform.hpp>
#include <iostream>

Game::Game(int width, int height, const char* title) : m_Window(nullptr), m_Width(width), m_Height(height), m_FOV(85.0f), m_RenderDistance(6),
    m_ClickTimer(0.0f), m_ClickCooldown(0.15f), m_LastFrame(0.0f), m_DeltaTime(0.0f), m_HitBlock(0), m_PlaceBlock(0)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // VSync

    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    Init();
}

Game::~Game()
{
    Shutdown();
}

void Game::Init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    InitImGui();

    float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height); // Calculates the Aspect Ratio so the Screen is not stretched.
    m_Projection = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 1200.0f);
    m_Model = glm::mat4(1.0f);

    // Create core systems
    m_Renderer = std::make_unique<Renderer>();
    m_Input = std::make_unique<Input>(m_Window);
    m_Camera = std::make_unique<Camera>(glm::vec3(8.0f, 5.0f, 8.0f), glm::vec3(0, 1, 0), -90.0f, 0.0f, aspect, m_FOV);
    m_World = std::make_unique<World>(1337);

    m_WorldShader = std::make_unique<Shader>("res/shaders/vertex.shader", "res/shaders/fragment.shader");

    m_CutoutShader = std::make_unique<Shader>("res/shaders/vertex.shader", "res/shaders/fragment.shader");
    m_WaterShader = std::make_unique<Shader>("res/shaders/water_vertex.shader", "res/shaders/water_fragment.shader");
	m_FogShader = std::make_unique<Shader>("res/shaders/fog_vert.shader", "res/shaders/fog_frag.shader");

    m_AtlasTexture = std::make_unique<Texture>("res/textures/atlas.png");
    m_NormalMapTexture = std::make_unique<Texture>("res/textures/NormalMap.png");

    unsigned int cubeMapID = Texture::LoadCubemap("res/textures/Cubemap_Sky_04-512x512.png");
    m_Skybox = std::make_unique<Skybox>(cubeMapID);

	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Locks Cursor in Place. TODO: unlock on ESC

    // Deferred Rendering Setup
    m_GBuffer = std::make_unique<GBuffer>();
    m_GBuffer->Create(m_Width, m_Height);
	
    // SSGI Setup
    m_SSGIShader = std::make_unique<Shader>("res/shaders/deferred_light_vertex.shader", "res/shaders/ssgi_fragment.shader");
    m_SSGIShader->Bind();
    m_SSGIShader->SetUniform1i("gPosition", 0);
    m_SSGIShader->SetUniform1i("gNormal", 1);
    m_SSGIShader->SetUniform1i("gAlbedo", 2);
	
    glGenFramebuffers(1, &m_SSGIFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSGIFBO);
	    
    glGenTextures(1, &m_SSGIColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSGIColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSGIColorBuffer, 0);	    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSGI Framebuffer not complete!" << std::endl;
	        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
    // Shadow Map Setup
    m_ShadowMap = std::make_unique<ShadowMap>();
    m_ShadowMap->Init(m_ShadowResolution, m_ShadowResolution);
    m_ShadowShader = std::make_unique<Shader>("res/shaders/shadow_vertex.shader", "res/shaders/shadow_fragment.shader");

    // Load deferred lighting shader
    m_LightingShader = std::make_unique<Shader>("res/shaders/deferred_light_vertex.shader", "res/shaders/deferred_light_fragment.shader");
    m_LightingShader->Bind();
    m_LightingShader->SetUniform1i("gPosition", 0);
    m_LightingShader->SetUniform1i("gNormal", 1);
    m_LightingShader->SetUniform1i("gAlbedo", 2);
    m_LightingShader->SetUniform1i("gMetallic", 3);
    m_LightingShader->SetUniform1i("gRoughness", 4);
    m_LightingShader->SetUniform1i("gAO", 5);
    m_LightingShader->SetUniform1i("shadowMap", 6);
    m_LightingShader->SetUniform1i("ssgiMap", 7);
}   

void Game::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // Shader Version
}

void Game::ProcessInput(float deltaTime)
{
    m_Input->UpdateMouse();
    m_Camera->ProcessKeyboard(*m_Input, deltaTime);

    if (m_CursorLocked)
        m_Camera->ProcessMouse(m_Input->GetMouseInput());

    m_ClickTimer -= deltaTime;

    bool leftMouseDown = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightMouseDown = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool canClick = m_ClickTimer <= 0.0f;

    static bool escWasDown = false;

    bool escDown = glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escDown && !escWasDown)
    {
        m_CursorLocked = !m_CursorLocked;

        glfwSetInputMode(m_Window, GLFW_CURSOR, m_CursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    escWasDown = escDown;

    if (m_World->Raycast(m_Camera->GetPosition(), m_Camera->GetFront(), 15.0f, m_HitBlock, m_PlaceBlock))
    {
        if (canClick && m_CursorLocked)
        {
            if (leftMouseDown)
            {
                m_World->SetBlock(m_HitBlock.x, m_HitBlock.y, m_HitBlock.z, BlockType::AIR);
                m_ClickTimer = m_ClickCooldown;
            }
            else if (rightMouseDown)
            {
                m_World->SetBlock(m_PlaceBlock.x, m_PlaceBlock.y, m_PlaceBlock.z, BlockType::GRASS);
                m_ClickTimer = m_ClickCooldown;
            }
        }
    }
}

void Game::Update(float deltaTime)
{
    m_World->UpdateChunksInRadius(  
        World::WorldToChunk(static_cast<int>(m_Camera->GetPosition().x)),
        World::WorldToChunk(static_cast<int>(m_Camera->GetPosition().z)),
        m_RenderDistance * 3
    );
}

void Game::Render()
{
    glm::mat4 view = m_Camera->GetViewMatrix();
    glm::mat4 mvp = m_Projection * view * m_Model;

    // Light Properties
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f));
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 0.9f);

    // --- SHADOW PASS ---
    m_Renderer->BeginShadowPass(*m_ShadowMap);
    
    // Compute Light Space Matrix (Ortho centered on camera)
    float near_plane = 1.0f, far_plane = 500.0f;
    float shadowDist = 160.0f; // Shadow distance
    glm::mat4 lightProjection = glm::ortho(-shadowDist, shadowDist, -shadowDist, shadowDist, near_plane, far_plane);
    
    // Snap light view to texels to avoid shimmering (simplified)
    glm::vec3 camPos = m_Camera->GetPosition();
    glm::mat4 lightView = glm::lookAt(camPos - lightDir * 100.0f, camPos, glm::vec3(0.0f, 1.0f, 0.0f));
    m_LightSpaceMatrix = lightProjection * lightView;

    m_ShadowShader->Bind();
    m_ShadowShader->SetUniformMat4f("u_LightSpaceMatrix", m_LightSpaceMatrix);
    m_ShadowShader->SetUniformMat4f("u_Model", m_Model);
    
    m_World->Render(*m_Renderer, *m_ShadowShader, *m_Camera, 0); // Solid
    m_World->Render(*m_Renderer, *m_ShadowShader, *m_Camera, 1); // Cutout

    m_Renderer->EndShadowPass(m_Width, m_Height);

    // --- GEOMETRY PASS ---
    m_Renderer->BeginGeometryPass(*m_GBuffer);

    // SOLID BLOCKS
    glDisable(GL_BLEND);
    glDepthMask(true);
    m_WorldShader->Bind();
    m_WorldShader->SetUniformMat4f("u_Model", m_Model);
    m_WorldShader->SetUniformMat4f("u_View", view);
    m_WorldShader->SetUniformMat4f("u_Proj", m_Projection);
    m_AtlasTexture->Bind(0);
    m_NormalMapTexture->Bind(1);
    m_WorldShader->SetUniform1i("u_Texture", 0);
    m_WorldShader->SetUniform1i("u_NormalMap", 1);
    m_World->Render(*m_Renderer, *m_WorldShader, *m_Camera, 0);

    // CUTOUT BLOCKS
    m_CutoutShader->Bind();
    m_CutoutShader->SetUniformMat4f("u_Model", m_Model);
    m_CutoutShader->SetUniformMat4f("u_View", view);
    m_CutoutShader->SetUniformMat4f("u_Proj", m_Projection);
    m_CutoutShader->SetUniform1i("u_Texture", 0);
    m_CutoutShader->SetUniform1i("u_NormalMap", 1);
    m_World->Render(*m_Renderer, *m_CutoutShader, *m_Camera, 1);

    m_Renderer->EndGeometryPass();

    /*
    // --- SSGI PASS ---
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSGIFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_SSGIShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_PositionTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_NormalTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_AlbedoTex);
    
    m_SSGIShader->SetUniformMat4f("u_View", view);
    m_SSGIShader->SetUniformMat4f("u_Proj", m_Projection);
    */

    m_Renderer->RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- LIGHTING PASS ---
    m_Renderer->BeginLightingPass();

    m_LightingShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_PositionTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_NormalTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_AlbedoTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_MetallicTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_RoughnessTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->m_AOTex);
    
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_ShadowMap->m_DepthMap);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, m_SSGIColorBuffer);

    // Lights
    m_LightingShader->SetUniform3f("dirLight.direction", lightDir.x, lightDir.y, lightDir.z);
    m_LightingShader->SetUniform3f("dirLight.color", lightColor.x, lightColor.y, lightColor.z);
    m_LightingShader->SetUniform3f("viewPos", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z);
    m_LightingShader->SetUniformMat4f("u_LightSpaceMatrix", m_LightSpaceMatrix);

    // Fog
    m_LightingShader->SetUniform3f("u_FogColor", m_FogColor.x, m_FogColor.y, m_FogColor.z);
    m_LightingShader->SetUniform1f("u_FogDensity", m_FogDensity);
    m_LightingShader->SetUniform1f("u_FogHeight", m_FogHeight);
    m_LightingShader->SetUniform1f("u_FogFalloff", m_FogFalloff);
    m_LightingShader->SetUniform1i("u_FogMode", m_FogMode);

    m_Renderer->RenderQuad();

    m_Renderer->EndLightingPass();
    
    // Copy Depth for Forward Pass
    m_Renderer->BlitDepthBuffer(*m_GBuffer, m_Width, m_Height);

    // --- TRANSPARENT PASS ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(false);

    m_WaterShader->Bind();
    m_WaterShader->SetUniformMat4f("u_Model", m_Model);
    m_WaterShader->SetUniformMat4f("u_View", view);
    m_WaterShader->SetUniformMat4f("u_Proj", m_Projection);
    
    // Bind Atlas for Water
    m_AtlasTexture->Bind(0);
    m_WaterShader->SetUniform1i("u_Texture", 0);

    m_WaterShader->SetUniform1f("u_Time", static_cast<float>(glfwGetTime()));
    m_WaterShader->SetUniform3f("viewPos", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z);
    
    // Lights & Shadows
    m_WaterShader->SetUniform3f("dirLight.direction", lightDir.x, lightDir.y, lightDir.z);
    m_WaterShader->SetUniform3f("dirLight.color", lightColor.x, lightColor.y, lightColor.z);
    m_WaterShader->SetUniformMat4f("u_LightSpaceMatrix", m_LightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_ShadowMap->m_DepthMap);
    m_WaterShader->SetUniform1i("shadowMap", 1);


    // Fog
    m_WaterShader->SetUniform3f("u_FogColor", m_FogColor.x, m_FogColor.y, m_FogColor.z);
    m_WaterShader->SetUniform1f("u_FogDensity", m_FogDensity);
    m_WaterShader->SetUniform1f("u_FogHeight", m_FogHeight);
    m_WaterShader->SetUniform1f("u_FogFalloff", m_FogFalloff);
    m_WaterShader->SetUniform1i("u_FogMode", m_FogMode);

    m_World->Render(*m_Renderer, *m_WaterShader, *m_Camera, 2);

    glDepthMask(true);
    glDisable(GL_BLEND);

    // --- SKYBOX ---
    m_Renderer->DrawSkybox(*m_Skybox, view, m_Projection);
}

void Game::RenderImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glm::vec3 cameraPos = m_Camera->GetPosition();

    ImGui::Begin("Debug Info");
    ImGui::SetWindowSize(ImVec2(400, 500), ImGuiCond_Always);
    ImGui::Text("Camera Position: X %.2f | Y %.2f | Z %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
	ImGui::Text("Current Camera Forward: %.2f | %.2f | %.2f", m_Camera->GetFront().x, m_Camera->GetFront().y, m_Camera->GetFront().z);
    ImGui::Text("Block Position: X %d | Y %d | Z %d", m_HitBlock.x, m_HitBlock.y, m_HitBlock.z);

	ImGui::Text("Current Chunk Position: X %d | Z %d", World::WorldToChunk(static_cast<int>(m_Camera->GetPosition().x)), World::WorldToChunk(static_cast<int>(m_Camera->GetPosition().z)));

    ImGui::Checkbox("Frustum Culling", &m_World->frustumCulling);

	ImGui::BeginGroup();
    ImGui::ColorEdit3("Fog Color", &m_FogColor.x);
	ImGui::SliderFloat("Fog Density", &m_FogDensity, 0.0f, 0.1f);
	ImGui::SliderFloat("Fog FallOff", &m_FogFalloff, 0.0f, 0.5f);
	ImGui::SliderFloat("Fog Height", &m_FogHeight, 0.0f, 512.0f);
	ImGui::SliderInt("Fog Mode (0 Exp, 1 Exp Height)", &m_FogMode, 0, 1);
	ImGui::EndGroup();


    ImGui::Text("Block Position: X %d | Y %d | Z %d", m_HitBlock.x, m_HitBlock.y, m_HitBlock.z);
    ImGui::Text("FPS: %.1f", 1.0f / m_DeltaTime);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::Run()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        ProcessInput(m_DeltaTime);
        Update(m_DeltaTime);
        Render();
        RenderImGui();

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void Game::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}