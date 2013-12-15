#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdlib.h>

SDL_Window *InitSDL(int winWidth, int winHeight, const char *title, char bFull);
SDL_GLContext InitSDL_GL(SDL_Window *window);
GLuint BuildShaderProgram(const char *vsPath, const char *fsPath);
GLuint CreateShader(GLenum eShaderType, const char *strShaderFile);

typedef struct QuadDrawData_s {
	float *vertexArray;
	GLuint *indexArray;
	int vertexArraySize;
	int indexArraySize;
	int numVerts;
	int numIndices;
	GLuint vao;
	GLuint vbo;
	GLuint shader;
} QuadDrawData_t; //don't modify this struct by hand! Only in functions!
QuadDrawData_t *NewQuadDataBuffer(const int numQuads); //allocates QuadDrawData
char SetQuadShader(QuadDrawData_t *qDrawData, GLuint shader); //returns 1 on success
void DestroyQuadDrawData(QuadDrawData_t *qDrawData); //free memory
float *AllocateQuadArray(const int numQuads); //used in NewQuadDataBuffer
GLuint *AllocateIndexArray(const int numQuads); //used in NewQuadDataBuffer

//resets vertex and index counts to 0. Doesn't re-initialize data.
void ClearQuadDrawData(QuadDrawData_t *qDrawData); 
void DrawRect(QuadDrawData_t *QuadBuffer, 
	float x, float y, float z, float w, float h); //inserts vertices and indices in buffer
void WriteVec4(float *floatArr, float x, float y, float z, float w);
	
void DrawQuadData(QuadDrawData_t *QuadBuffer, GLuint shader); //does actual GL drawing

float RandFloat(float min, float max);

#endif