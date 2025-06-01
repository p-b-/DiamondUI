#version 420 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 transform;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * transform * vec4(vertex.xy, 0.0, 1.0);
}