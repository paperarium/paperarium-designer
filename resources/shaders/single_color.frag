# version 330

uniform vec3 flatColor;
uniform float alpha;

layout (location = 0) out vec4 gFragColor;

void main() {
    gFragColor = vec4(flatColor, alpha);
}
