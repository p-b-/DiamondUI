#version 420 core
out vec4 colour;

uniform sampler2D text;
uniform vec4 overlayColour;

void main()
{    
    colour = overlayColour;
}