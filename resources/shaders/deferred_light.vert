#version 330 core

uniform mat4 projection;
uniform mat4 mv;

layout(location = 0) in vec3 aPosition;

void main() {
    gl_Position = projection * mv * vec4(aPosition, 1.0);
}
