#pragma once

#include <GL/glew.h>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "world/Skybox.h"

class Skybox;
class VertexArray;
class IndexBuffer;
class Shader;

#include <glm.hpp>
#include "GBuffer.h"
#include "ShadowMap.h"

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);



class Renderer
{
private:
	unsigned int m_QuadVAO = 0;
	unsigned int m_QuadVBO = 0;

public:
    void Clear() const;
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void DrawSkybox(const Skybox& skybox, const glm::mat4& view, const glm::mat4& proj) const;
	
	// Deferred Rendering
	void BeginGeometryPass(const GBuffer& gBuffer);
	void EndGeometryPass() const;
	void BeginLightingPass() const;
	void EndLightingPass() const;
    void BlitDepthBuffer(const GBuffer& gBuffer, int width, int height);

    // Shadow Pass
    void BeginShadowPass(const ShadowMap& shadowMap);
    void EndShadowPass(int viewportWidth, int viewportHeight) const;

	void RenderQuad();

	const GBuffer* m_CurrentGBuffer = nullptr;
};