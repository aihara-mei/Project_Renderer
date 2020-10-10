#include "vertice_transform.h"

Matrix Transform::view(Vec3 look_direction, Vec3 eye_pos, Vec3 up) {
	Vec3 z = normalize(look_direction);
	Vec3 x = normalize(cross(up, z));
	Vec3 y = normalize(cross(z, x));
	Matrix t = Matrix::eye(4);
	for (int i = 0; i < 3; i++) {
		t[0][i] = x[i];
		t[1][i] = y[i];
		t[2][i] = z[i];
		t[i][3] = -eye_pos[i];
	}
	return t;
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