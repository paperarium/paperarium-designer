# version 330

uniform mat4 modelView;
uniform mat4 projection;

layout(location = 0) in vec3 aPosition;

void main() {
    gl_Position = projection * modelView * vec4(aPosition, 1.0f);
}
