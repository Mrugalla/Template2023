#version 330 core
            
in vec4 fragColour;

uniform vec4 uColour;

void main()
{
	gl_FragColor = uColour;
}