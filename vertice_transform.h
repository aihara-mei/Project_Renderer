#pragma once
#include "matrix.h"
#include "model.h"

struct View_frustum {
	float near, far;
	float l, b, r, t; // left, bottom, right, top
};


class Transform
{
public:
	Matrix view(Vec3 look_direction, Vec3 eye_pos, Vec3 up);
	Matrix viewport(int w, int h);
	Matrix projection(float c);
	Matrix projection_(float n, float f);
	Matrix projection_R(float n, float f, float r, float l, float t, float b);
	Matrix projection_R(const View_frustum& m);
};
