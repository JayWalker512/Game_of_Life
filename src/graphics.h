#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "vector3.h"

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
	GLint scaleLoc;
	GLint translationLoc;
	GLint rgbLoc;
	Vector3_t scale;
	Vector3_t translation;
	Vector3_t rgb;
} QuadDrawData_t; //don't modify this struct by hand! Only in functions!

SDL_Window *InitSDL(int winWidth, int winHeight, const char *title, char bFull);
SDL_GLContext InitSDL_GL(SDL_Window *window);
void ClearScreen(float r, float g, float b); //clears color and depth buffers

QuadDrawData_t *NewQuadDataBuffer(const int numQuads); //allocates QuadDrawData
void DestroyQuadDrawData(QuadDrawData_t *qDrawData); //free memory
char SetQuadShader(QuadDrawData_t *qDrawData, GLuint shader); //returns 1 on success
void ClearQuadDrawData(QuadDrawData_t *qDrawData); //resets vertex and index counts to 0.

void SetQuadTranslation(QuadDrawData_t *dest, Vector3_t *const translation);
void SetQuadScale(QuadDrawData_t *dest, Vector3_t *const scale);
void SetQuadColor(QuadDrawData_t *dest, Vector3_t *const rgb);

void DrawRect(QuadDrawData_t *QuadBuffer, 
	float x, float y, float z, float w, float h); //inserts vertices and indices in buffer
void DrawQuadData(QuadDrawData_t *QuadBuffer); //does actual GL drawing

GLuint BuildShaderProgram(const char *vsPath, const char *fsPath);

#endif
