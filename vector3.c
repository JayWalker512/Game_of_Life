#include "vector3.h"
#include <stdlib.h>
#include <math.h>

Vector3_t *NewVector3(float x, float y, float z)
{
	Vector3_t *vec;
	vec = malloc(sizeof(Vector3_t));
	if (vec == NULL)
		return NULL;

	vec->x = x;
	vec->y = y;
	vec->z = z;
	return vec;
}

void Vector3Set(Vector3_t *out, float x, float y, float z)
{
	out->x = x;
	out->y = y;
	out->z = z;
}

void Vector3Normalize(Vector3_t *out, Vector3_t *const in)
{
	float magnitude = sqrt((in->x * in->x) + (in->y * in->y) + (in->z * in->z));
	out->x = in->x / magnitude;
	out->y = in->y / magnitude;
	out->z = in->z / magnitude;
}

void Vector3Add(Vector3_t *out, Vector3_t *const term1, Vector3_t *const term2)
{
	out->x = term1->x + term2->x;
	out->y = term1->y + term2->y;
	out->z = term1->z + term2->z; 
}

float Vector3Dot(Vector3_t *const term1, Vector3_t *const term2)
{
	return (term1->x * term2->x) + (term1->y * term2->y) + (term1->z * term2->z);
}
