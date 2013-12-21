#version 330
layout(location = 0) in vec4 position;
smooth out vec4 theColor;
void main()
{
	vec4 flippedYandScaled = vec4(2.0, -2.0, 1.0, 1.0);
	vec4 uLeftTranslation = vec4(-1.0, 1.0, 0, 0);
	gl_Position = (position * flippedYandScaled) + uLeftTranslation;
	theColor = vec4(1.0 - position.z, 0.0, 0.0, 0.0);
}
