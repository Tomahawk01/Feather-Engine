#version 450 core

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TexCoords;
out vec2 FragUVs;
uniform mat4 u_Projection;

void main()
{
	gl_Position = u_Projection * vec4(in_Position, 1.0);
	FragUVs = in_TexCoords;
}
