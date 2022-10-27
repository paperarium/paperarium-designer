# version 330 core

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in mediump vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 Vert;
out vec3 VertNormal;
out mediump vec2 TexCoords;
out vec3 Bitangent;
out vec3 Tangent;
out vec3 Position;

void main() {
    Vert = aPosition;
    VertNormal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords;
    Bitangent = aBitangent;
    Tangent = aTangent;
    Position = vec3(modelMatrix * vec4(aPosition, 0.0));
    gl_Position = projMatrix * mvMatrix * vec4(aPosition, 0.0);
}
