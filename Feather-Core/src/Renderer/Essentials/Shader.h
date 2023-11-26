#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Feather {

	class Shader
	{
	public:
		Shader();
		Shader(GLuint program, const std::string vertexPath, const std::string& fragmentPath);
		~Shader();

		void SetUniformInt(const std::string& name, int value);
		void SetUniformMat4(const std::string& name, glm::mat4& mat);
		// TODO: Add more setters and getters

		void Enable();
		void Disable();

		inline const GLuint ShaderProgramID() const { return m_ShaderProgramID; }

	private:
		GLuint m_ShaderProgramID;
		std::string m_VertexPath, m_FragmentPath;

		std::unordered_map<std::string, GLuint> m_UniformLocationMap;
	private:
		GLuint GetUniformLocation(const std::string& uniformName);
	};

}
