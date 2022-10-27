# version 330

uniform vec3 cameraPos;
uniform float mode;
uniform sampler2D diffuseTexture;

in vec3 Vert;
in vec3 VertNormal;
in mediump vec2 TexCoords;
in vec3 Bitangent;
in vec3 Tangent;
in vec3 Position;

layout (location = 0) out vec4 gFragColor;

void main() {
    vec3 T = normalize(cross(Bitangent, Tangent));
    vec3 I = normalize(Position - cameraPos);
    if (mode == 0)                  // 0 - Diffuse texture
        gFragColor = texture(diffuseTexture, TexCoords);
    else if (mode == 1)             // 1 - Vertex position
        gFragColor = vec4(Vert, 1.0);
    else if (mode == 2)             // 2 - Vertex normal
        gFragColor = vec4(VertNormal, 1.0);
    else if (mode == 3)             // 3 - Vertex texture coord
        gFragColor = vec4(TexCoords, 0.0, 0.0);
    else if (mode == 4)             // 4 - Bitangent
        gFragColor = vec4(Bitangent, 1.0);
    else if (mode == 5)             // 5. Tangent
        gFragColor = vec4(Tangent, 1.0);
    else if (mode == 6)             // 6. Depth
        gFragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    else if (mode == 7) {           // 7. Linear depth
        float near = 0.1;
        float far = 100.0;
        float z = gl_FragCoord.z * 2.0 - 1.0;
        float linear_depth = (2.0 * near * far) / (far + near - z * (far - near));
        gFragColor = vec4(vec3(1.0 - (linear_depth / far)), 1.0);
    }
    gFragColor = vec4(0,1,0,1.0);
}
