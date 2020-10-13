#include "vertice_transform.h"

Matrix Transform::view(Vec3 look_direction, Vec3 eye_pos, Vec3 up) {
	Vec3 z = normalize(look_direction);
	Vec3 x = normalize(cross(up, z));
	Vec3 y = normalize(cross(z, x));
	Matrix r = Matrix::eye(4);
	Matrix t = Matrix::eye(4);
	for (int i = 0; i < 3; i++) {
		r[0][i] = x[i];
		r[1][i] = y[i];
		r[2][i] = z[i];
		t[i][3] = -eye_pos[i];
	}
	return r * t;
}

Matrix Transform::viewport(int w, int h) {
	Matrix m = Matrix::eye(4);
	m[0][3] = w / 2.f;
	m[1][3] = h / 2.f;
	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	return m;
}

Matrix Transform::projection(float c) {
	Matrix m = Matrix::eye(4);
	m[3][2] = (-1.) / c;
	return m;
}

Matrix Transform::projection_R(float n, float f, float r, float l, float t, float b) {
	Matrix m = Matrix(4, 4);
	m[0][0] = 2. * n / (r - l);
	m[0][2] = (r + l) / (l - r);

	m[1][1] = 2. * n / (t - b);
	m[1][2] = (t + b) / (b - t);

	m[2][2] = (f + n) / (n - f);
	m[2][3] = 2. * f * n / (f - n);

	m[3][2] = 1.;
	return m;
}

Matrix Transform::projection_R(const View_frustum& frt) {
	float n, f, r, l, b, t;
	n = frt.near;
	f = frt.far;
	r = frt.r;
	l = frt.l;
	b = frt.b;
	t = frt.t;

	Matrix m = Matrix(4, 4);
	m[0][0] = 2. * n / (r - l);
	m[0][2] = (r + l) / (l - r);

	m[1][1] = 2. * n / (t - b);
	m[1][2] = (t + b) / (b - t);

	m[2][2] = (f + n) / (n - f);
	m[2][3] = 2. * f * n / (f - n);

	m[3][2] = 1.;
	return m;
}