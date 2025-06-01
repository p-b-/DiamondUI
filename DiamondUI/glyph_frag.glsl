#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D glyphTexture;

void main()
{             
    FragColor = texture(glyphTexture, TexCoords).rgba;
}