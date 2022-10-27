# version 330 core

uniform float mode = 0;
uniform sampler2D ambient;
uniform sampler2D diffuse;

in mediump vec2 TexCoords;

layout (location = 0) out vec4 gFragColor;

void main() {
    if (mode == 0)
        gFragColor = vec4(texture(ambient, TexCoords).rgb, 1.0);
    else if (mode == 1)
        gFragColor = vec4(texture(diffuse, TexCoords).rgb, 1.0);
}
