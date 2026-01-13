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
    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    InitImGui();

    // Create core systems
    m_Renderer = std::make_unique<Renderer>();
    m_Input = std::make_unique<Input>(m_Window);
    m_Camera = std::make_unique<Camera>(glm::vec3(8.0f, 5.0f, 8.0f));
    m_World = std::make_unique<World>(1337);

    m_WorldShader = std::make_unique<Shader>("res/shaders/vertex.shader", "res/shaders/fragment.shader");
    m_AtlasTexture = std::make_unique<Texture>("res/textures/atlas.png");

    unsigned int cubeMapID = Texture::LoadCubemap("res/textures/Cubemap_Sky_04-512x512.png");
    m_Skybox = std::make_unique<Skybox>(cubeMapID);

	float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height); // Calculates the Aspect Ratio so the Screen is not stretched.
    m_Projection = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 1200.0f);
    m_Model = glm::mat4(1.0f);

	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Locks Cursor in Place. TODO: unlock on ESC
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
    m_Camera->ProcessMouse(m_Input->GetMouseInput());

    m_ClickTimer -= deltaTime;

    bool leftMouseDown = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightMouseDown = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool canClick = m_ClickTimer <= 0.0f;

    if (m_World->Raycast(m_Camera->GetPosition(), m_Camera->GetFront(), 15.0f, m_HitBlock, m_PlaceBlock))
    {
        if (canClick)
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
        m_RenderDistance
    );
}

void Game::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Renderer->Clear();

    glm::mat4 view = m_Camera->GetViewMatrix();
    glm::mat4 mvp = m_Projection * view * m_Model;

    // Render world
    m_WorldShader->Bind();
    m_WorldShader->SetUniformMat4f("u_MVP", mvp);
    m_AtlasTexture->Bind();
    m_WorldShader->SetUniform1i("u_Texture", 0);

    m_World->Render(*m_Renderer, *m_WorldShader);

    if (m_World->Raycast(m_Camera->GetPosition(), m_Camera->GetFront(), 15.0f, m_HitBlock, m_PlaceBlock)) {
        m_World->RenderBlockOutline(*m_Renderer, *m_WorldShader, m_HitBlock.x, m_HitBlock.y, m_HitBlock.z);
    }

    m_Renderer->DrawSkybox(*m_Skybox, view, m_Projection);
}

void Game::RenderImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glm::vec3 cameraPos = m_Camera->GetPosition();

    ImGui::Begin("Debug Info");
    ImGui::SetWindowSize(ImVec2(300, 100), ImGuiCond_Always);
    ImGui::Text("Camera Position: X %.2f | Y %.2f | Z %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
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