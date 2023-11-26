#version 450 core

in vec2 FragUVs;
in vec4 FragColor;
out vec4 out_Color;
uniform sampler2D u_Texture;

void main()
{
	vec4 textureColor = texture(u_Texture, FragUVs);
	out_Color = textureColor * FragColor;
}
