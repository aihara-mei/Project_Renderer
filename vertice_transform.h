#pragma once
#include "matrix.h"
#include "model.h"

class Transform
{
public:
	Matrix view(Vec3 look_direction, Vec3 eye_pos, Vec3 up);
	Matrix viewport(int w, int h);
	Matrix projection(float c);
};

