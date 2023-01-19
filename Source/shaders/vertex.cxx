#version 330 core
            
in vec4 position;      
in vec4 sourceColour;
out vec4 fragColour;

void main()
{
	gl_Position = position;

	fragColour = sourceColour;
}