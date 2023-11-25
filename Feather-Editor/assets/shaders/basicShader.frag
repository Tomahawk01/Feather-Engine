#version 450 core

in vec2 FragUVs;
out vec4 out_Color;
uniform sampler2D u_Texture;

void main()
{
	out_Color = texture(u_Texture, FragUVs);
}
