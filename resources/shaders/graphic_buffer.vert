#version 330 core

uniform mat4 model;
uniform mat4 modelInv;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in mediump vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 FragPos;
out mediump vec2 TexCoords;
out vec3 Normal;

void main() {
    vec4 worldPos = model * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    mat3 normalMatrix = transpose(mat3(modelInv));
    Normal = normalMatrix * aNormal;
    gl_Position = projection * view * worldPos;
}
