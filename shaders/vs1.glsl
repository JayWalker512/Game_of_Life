#version 330
layout(location = 0) in vec4 position;
uniform vec4 scale;
uniform vec4 translation;
uniform vec4 rgb;
smooth out vec4 fragColor;
void main()
{
	vec4 flippedYandScaled = vec4(2.0, -2.0, 1.0, 1.0);
	vec4 uLeftTranslation = vec4(-1.0, 1.0, 0, 0);
	vec4 initialTranslation = (position * flippedYandScaled) + uLeftTranslation;
	gl_Position = (initialTranslation * scale) + translation;
	fragColor = rgb; 
	//0 alpha does nothing? need enabled?
}
