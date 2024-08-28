#version 450 core

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TexCoords;
layout (location = 2) in vec4 in_Color;

out vec2 FragUVs;
out vec4 FragColor;
uniform mat4 uProjection;

void main()
{
	gl_Position = uProjection * vec4(in_Position, 1.0);
	FragUVs = in_TexCoords;
	FragColor = in_Color;
}
