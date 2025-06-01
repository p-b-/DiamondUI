#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceTransform;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // Normal = mat3(transpose(inverse(model))) * aNormal;  
    // Transform normal into world space, for lighting, but ignoring any sheer/translations (this is what the transpose(inverse()) does)
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.FragPosLightSpace = lightSpaceTransform * vec4(vs_out.FragPos, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

