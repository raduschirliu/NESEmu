#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D textureSampler;
uniform vec3[4] palette;

void main()
{
    vec4 pixel = texture(textureSampler, TexCoords);
    vec3 paletteColor = palette[int(pixel.r * 10.0f / 2.0f)];
    FragColor = vec4(paletteColor, 1.0f);
}