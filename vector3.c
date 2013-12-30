#include "vector3.h"

void Vector3Normalize(Vector3_t *out, Vector3_t * const in)
{
	
}

void Vector3Add(Vector3_t *out, Vector3_t * const term1, Vector3_t * const term2)
{
	out->x = term1->x + term2->x;
	out->y = term1->y + term2->y;
	out->z = term1->z + term2->z; 
}

float Vector3Dot(Vector3_t * const term1, Vector3_t * const term2)
{
	return (term1->x * term2->x) + (term1->y * term2->y) + (term1->z * term2->z);
}
