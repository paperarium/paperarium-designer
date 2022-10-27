#version 330 core

uniform vec3 lightColor;

layout(location = 0) out vec4 gFragColor;

void main() {
    gFragColor = vec4(lightColor, 1.0);
}
