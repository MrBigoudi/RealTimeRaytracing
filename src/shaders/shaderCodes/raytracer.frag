#version 460 core

// input texture coords
layout(location = 0) in vec2 iTexCoord;

// output fragment color
layout(location = 0) out vec4 oColor;

// uniform texture containing the result of the raytracer
layout(binding = 0) uniform sampler2D raytracedTexture;

void main(){
    // oColor = texture(raytracedTexture, iTexCoord);
    oColor = vec4(0.f,1.f,0.f,1.f);
}