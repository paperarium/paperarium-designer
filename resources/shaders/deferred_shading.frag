# version 330 core

struct Light {
    bool isActive;
    vec3 Position;
    vec3 Color;
    float Intensity;
    float Range;
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 32;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gDiffuseColor;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

in mediump vec2 TexCoords;

layout (location = 0) out vec4 gFragColor;

void main() {
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gDiffuseColor, TexCoords).rgb;

    // calculate lighting as usual
    vec3 lighting = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for (int i = 0; i < NR_LIGHTS; ++i) {
        if (lights[i].Intensity > 0.01 && lights[i].isActive) {
            // calc. distance between light source and curr. fragment
            float distance = (length(lights[i].Position - FragPos) - lights[i].Range) / lights[i].Intensity;
            if (distance < 0) distance = 0;
            if (distance < lights[i].Radius && lights[i].isActive) {
                // diffuse
                vec3 lightDir = normalize(lights[i].Position - FragPos);
                vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

                // attenuation
                float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
                diffuse *= attenuation;
                lighting += diffuse;
            }
        }
    }

    // output the fragcolor
    gFragColor = vec4(lighting, 1.0);
}
