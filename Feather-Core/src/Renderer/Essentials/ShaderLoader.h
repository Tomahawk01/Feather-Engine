#pragma once

#include "Shader.h"

#include <memory>

namespace Feather {

	class ShaderLoader
	{
	public:
		ShaderLoader() = delete;
		static std::shared_ptr<Shader> Create(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

	private:
		static GLuint CreateProgram(const std::string& vertexShader, const std::string& fragmentShader);
		static GLuint CompileShader(GLuint shaderType, const std::string& filepath);
		static bool CompileSuccess(GLuint shader);
		static bool IsProgramValid(GLuint program);
		static bool LinkShaders(GLuint program, GLuint vertexShader, GLuint fragmentShader);
	};

}