#version 460 core

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(binding = 0) uniform sampler2D uRaytracedTexture;

void main(){
    oColor = vec4(texture(uRaytracedTexture, iTexCoord).rgb, 1.f);
}