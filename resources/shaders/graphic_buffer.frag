#version 330 core

uniform bool useFlatColor;
uniform vec3 flatColor;
uniform sampler2D diffuseTexture;

in mediump vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gFragColor;

void main() {
    gFragColor = vec4(texture(diffuseTexture, TexCoords).rgb, 1);
    gPosition = FragPos;
    gNormal = normalize(Normal);
}
