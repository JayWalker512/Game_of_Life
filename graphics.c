#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>

static float *AllocateQuadArray(const int numQuads); //used in NewQuadDataBuffer
static GLuint *AllocateIndexArray(const int numQuads); //used in NewQuadDataBuffer
static GLuint CreateShader(GLenum eShaderType, const char *strShaderFile);
static void WriteVec4(float *floatArr, float x, float y, float z, float w);
//static float RandFloat(float min, float max);

SDL_Window *InitSDL(int winWidth, int winHeight, const char *title, char bFull)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
			return NULL;
	
	SDL_Window *window;
	if (bFull)
	{
		window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			winWidth, winHeight,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			winWidth, winHeight,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	}
			
	return window;
}

SDL_GLContext InitSDL_GL(SDL_Window *window)
{
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (glContext == NULL)
	{
		printf("There was an error creating the OpenGL context!\n");
		return NULL;
	}
	
	const unsigned char *version = glGetString(GL_VERSION);
	if (version == NULL) 
	{
		printf("There was an error creating the OpenGL context!\n");
		return NULL;
	}
	
	SDL_GL_MakeCurrent(window, glContext);

	//MUST make a context AND make it current BEFORE glewInit()!
	glewExperimental = GL_TRUE;
	GLenum glew_status = glewInit();
	if (glew_status != 0) 
  {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
      return NULL;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  glViewport(0, 0, w, h);
	
	return glContext;
}

GLuint BuildShaderProgram(const char *vsPath, const char *fsPath)
{
	GLuint vertexShader;
	GLuint fragmentShader;
	
	vertexShader = CreateShader(GL_VERTEX_SHADER, vsPath);
	fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fsPath);
	
	/* So we've compiled our shaders, now we need to link them in to the program
	that will be used for rendering. */
	
	/*This section could be broken out into a separate function, but we're doing it here 
	because I'm not using C++ STL features that would make this easier. Tutorial doing so is 
	here: http://www.arcsynthesis.org/gltut/Basics/Tut01%20Making%20Shaders.html */
	
	GLuint tempProgram;
	tempProgram = glCreateProgram();
	
	glAttachShader(tempProgram, vertexShader);
	glAttachShader(tempProgram, fragmentShader);
	
	glLinkProgram(tempProgram); //linking!
	
	//error checking
	GLint status;
	glGetProgramiv(tempProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
    glGetProgramiv(tempProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
	
		GLchar strInfoLog[4096];
		glGetProgramInfoLog(tempProgram, infoLogLength, NULL, strInfoLog);
		printf("Shader linker failure: %s\n", strInfoLog);
		return -1;
	}
	//else
		//puts("Shader linked sucessfully!");
	
	glDetachShader(tempProgram, vertexShader);
	glDetachShader(tempProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	return tempProgram;
}

GLuint CreateShader(GLenum eShaderType, const char *strShaderFile)
{
	char shaderSource[4096];
	char inChar;
	FILE *shaderFile;
	int i = 0;
	
	shaderFile = fopen(strShaderFile, "r");
	while(fscanf(shaderFile,"%c",&inChar) > 0)
	{
		shaderSource[i++] = inChar; //loading the file's chars into array
	}
	shaderSource[i - 1] = '\0';
	fclose(shaderFile);
	//puts(shaderSource); //print to make sure the string is loaded
	
	GLuint shader = glCreateShader(eShaderType);
	const char *ss = shaderSource;
	glShaderSource(shader, 1, &ss, NULL);
	
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
  	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
  
  	GLchar strInfoLog[4096];
  	glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        
		char strShaderType[16];
		switch(eShaderType)
		{
			case GL_VERTEX_SHADER: sprintf(strShaderType, "vertex"); break;
			case GL_GEOMETRY_SHADER: sprintf(strShaderType, "geometry"); break;
			case GL_FRAGMENT_SHADER: sprintf(strShaderType, "fragment"); break;
		}

		//printf("Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		return -1;
	}
	//else
		//puts("Shader compiled sucessfully!");

	return shader;
}

QuadDrawData_t *NewQuadDataBuffer(const int numQuads)
{
	QuadDrawData_t *qDrawData = malloc(sizeof(QuadDrawData_t));
	qDrawData->vertexArray = AllocateQuadArray(numQuads);
	qDrawData->indexArray = AllocateIndexArray(numQuads);
	qDrawData->vertexArraySize = (sizeof(float) * 4 * 4) * numQuads; //hardcoded
	qDrawData->indexArraySize = (sizeof(GLuint) * 3) * numQuads; //hardcoded
	qDrawData->numVerts = 0;
	qDrawData->numIndices = 0;
	qDrawData->shader = -1;
	
	//set up OpenGL stuff
	glGenVertexArrays(1, &qDrawData->vao);
	glGenBuffers(1, &qDrawData->vbo); //create the buffer
	
	return qDrawData;
}

char SetQuadShader(QuadDrawData_t *qDrawData, GLuint shader)
{
	if (shader == (unsigned int)-1)
		return 0;

	qDrawData->shader = shader;
	return 1;
}

void DestroyQuadDrawData(QuadDrawData_t *qDrawData)
{
	glDeleteProgram(qDrawData->shader);
	glDeleteBuffers(1, &qDrawData->vbo);
	glDeleteVertexArrays(1, &qDrawData->vao);

	free(qDrawData->vertexArray);
	free(qDrawData->indexArray);
	free(qDrawData);
}

float *AllocateQuadArray(const int numQuads)
{
	return malloc((sizeof(float) * 4 * 4) * numQuads); //verts are four-component vec
}

GLuint *AllocateIndexArray(const int numQuads)
{
	return malloc((sizeof(GLuint) * 6) * numQuads);
}

void ClearQuadDrawData(QuadDrawData_t *qDrawData)
{
	qDrawData->numVerts = 0;
	qDrawData->numIndices = 0;
}

void DrawRect(QuadDrawData_t *QuadBuffer, 
	float x, float y, float z, float w, float h)
{
	int *numVerts = &QuadBuffer->numVerts;
	int *numIndices = &QuadBuffer->numIndices;	
	
	//insert 3 vertices for the uleft Quad.
	WriteVec4(QuadBuffer->vertexArray+(*numVerts*4), 
		x, y, z, 1.0f);
		
	WriteVec4(QuadBuffer->vertexArray+(*numVerts*4+4),
		x + w, y, z, 1.0f);
		
	WriteVec4(QuadBuffer->vertexArray+(*numVerts*4+8),
		x, y + h, z, 1.0f);
		
	*numVerts += 3;
	
	//insert 3 elements for drawing direction
	QuadBuffer->indexArray[*numIndices] = *numVerts - 3;
	QuadBuffer->indexArray[*numIndices + 1] = *numVerts - 2;
	QuadBuffer->indexArray[*numIndices + 2] = *numVerts - 1;
	
	*numIndices += 3;
	
	//insert 1 vertex for the lright corner of Quad.
	WriteVec4(QuadBuffer->vertexArray+(*numVerts*4), 
		x + w, y + h, z, 1.0f);
		
	*numVerts += 1;
	
	//insert 3 elements for drawing direction, containing 2 prev. existing verts
	QuadBuffer->indexArray[*numIndices] = *numVerts - 3;
	QuadBuffer->indexArray[*numIndices + 1] = *numVerts - 1;
	QuadBuffer->indexArray[*numIndices + 2] = *numVerts - 2;
	
	*numIndices += 3;
}

void WriteVec4(float *floatArr, float x, float y, float z, float w)
{
	*(floatArr) = x;
	*(floatArr+1) = y;
	*(floatArr+2) = z;
	*(floatArr+3) = w; 
}
	
void DrawQuadData(QuadDrawData_t *QuadBuffer, GLuint shader)
{
	/* drawing code in here! */
	glBindVertexArray(QuadBuffer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, QuadBuffer->vbo);
	
	/* I GET IT NOW!
	When we "buffer data" in the allocation function, we send a bunch of 
	empty data to the GPU. Once we update it, we need to re-bind the buffer 
	use glBufferSubData to send the changed portion of memory to the GPU.
	That's why drawing didn't work until we moved buffering to here. */
	glBufferData(GL_ARRAY_BUFFER, 
		QuadBuffer->vertexArraySize, 
		QuadBuffer->vertexArray, 
		GL_STREAM_DRAW); //formatting the data for the buffer, sending it to the buffer
	 
	glEnableVertexAttribArray(0); //0 is our index, refer to "location = 0" in the vertex shader
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0); //tell gl (shader!) how to interpret our
	
	glUseProgram(shader);
	glDrawElements(GL_TRIANGLES, QuadBuffer->numIndices, GL_UNSIGNED_INT, QuadBuffer->indexArray); 
	glUseProgram(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind any buffers. Doesn't affect VAO!
	glBindVertexArray(0); //unbind the VAO for now. We re-bind it to restore above state.
	/* drawing code above here! */
}

//Doesn't generate negatives correctly?
/*float RandFloat(float min, float max)
{
	return min + (float)rand()/((float)RAND_MAX/max);
}*/

void ClearScreen(float r, float g, float b)
{	
	glClearColor(r, g, b, 0.0);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}