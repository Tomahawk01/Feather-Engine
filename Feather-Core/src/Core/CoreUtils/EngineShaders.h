#pragma once

namespace Feather {

	static const char* basicShaderVert = R"(
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
)";

	static const char* basicShaderFrag = R"(
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
)";

	static const char* circleShaderVert = R"(
#version 450 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aUVs;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aLineThickness;

out vec2 fragUVs;
out vec4 fragColor;
out float fragLineThickness;

uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
    fragUVs = aUVs;
    fragColor = aColor;
    fragLineThickness = aLineThickness;
}
)";

	static const char* circleShaderFrag = R"(
#version 450 core

in vec2 fragUVs;
in vec4 fragColor;
in float fragLineThickness;

out vec4 color;

void main()
{
    float distance = 1 - length(fragUVs); // calculates the length of the given vector
    float wd = fwidth(distance); // fwidth — return the sum of the absolute value of derivatives in x and y
    float circle = smoothstep(0, wd, distance); // smoothstep will perform an interpolation between the two values

    circle *= smoothstep(fragLineThickness + wd, fragLineThickness, distance);

    if (circle == 0.0)
        discard;

    color = fragColor;
    color.a *= circle;
}
)";

    static const char* fontShaderVert = R"(
#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uvs;

uniform mat4 uProjection;

out vec4 vertexColor;
out vec2 vertexUVs;

void main()
{
	gl_Position = uProjection * vec4(position.x, position.y, 0.0, 1.0);
	vertexColor = color;
	vertexUVs = uvs;
}
)";

	static const char* fontShaderFrag = R"(
#version 450 core

in vec4 vertexColor;
in vec2 vertexUVs;

uniform sampler2D atlas;

out vec4 finalColor;

void main()
{
	vec4 pixelColor = vec4(1, 1, 1, texture(atlas, vertexUVs).r);
	finalColor = pixelColor * vertexColor;
}
)";

	static const char* colorShaderVert = R"(
#version 450 core

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec4 vertexColor;

out vec4 fragmentColor;

uniform mat4 uProjection;

void main()
{
	gl_Position.xy = (uProjection * vec4(vertexPosition, 0.0, 1.0)).xy;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;

	fragmentColor = vertexColor;
}
)";

	static const char* colorShaderFrag = R"(
#version 450 core

in vec4 fragmentColor;

out vec4 color;

void main()
{
	color = fragmentColor;
}
)";

}
