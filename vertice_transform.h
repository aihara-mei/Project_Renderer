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
	static Matrix view(Vec3 look_direction, Vec3 eye_pos, Vec3 up);
	static Matrix viewport(int w, int h);
	static Matrix projection(float n, float f, float r, float l, float t, float b);
	static Matrix projection(const View_frustum& m);
};
