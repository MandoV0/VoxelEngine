#pragma once

#include <string>
#include <unordered_map>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

class Shader
{
private:
	std::string m_FilePathVertex;
	std::string m_FilePathFragment;
	unsigned int m_RendererID;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;

public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	void Bind() const;
	void Unbind() const;
	
	// Set uniforms
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
	void SetUniform1i(const std::string& name, int value) const;
	void SetUniform1f(const std::string& name, float value) const;
	void SetUniform2f(const std::string& name, float v1, float v2) const;
	void SetUniform3f(const std::string& name, float v1, float v2, float v3) const;
	void SetUniformMat4f(const std::string& name, const glm::mat4& mat) const;

private:
	int GetUniformLocation(const std::string& name) const;
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	std::string LoadShader(const std::string& path);

};