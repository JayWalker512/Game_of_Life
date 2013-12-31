#ifndef VECTOR3_H
#define VECTOR3_H

typedef struct Vector3_s {
	float x;
	float y;
	float z;
} Vector3_t;

Vector3_t *NewVector3(float x, float y, float z);
void Vector3Set(Vector3_t *out, float x, float y, float z);
void Vector3Normalize(Vector3_t *out, Vector3_t *const in);
void Vector3Add(Vector3_t *out, Vector3_t *const term1, Vector3_t *const term2);
float Vector3Dot(Vector3_t *const term1, Vector3_t *const term2);

#endif
