#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D textureSampler;
uniform vec4[4] palette;
uniform vec4 colorModifier;

void main()
{
    vec4 pixel = texture(textureSampler, TexCoords);
    vec4 paletteColor = palette[int(pixel.r * 10.0f / 2.0f)];
    FragColor = paletteColor * colorModifier;
}