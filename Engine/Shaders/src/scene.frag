#version 450

layout (location = 0) in vec2 inUV;

layout (binding = 1) uniform sampler2D pngtexture;

layout (location = 0) out vec4 outFragColor;


void main()
{
    outFragColor = texture(pngtexture, inUV);
}