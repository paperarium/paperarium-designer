#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 2) in mediump vec2 aTexCoords;

out mediump vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPosition, 1.0);
}
