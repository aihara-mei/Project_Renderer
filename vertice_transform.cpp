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

Matrix Transform::projection(float n, float f, float r, float l, float t, float b) {
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

Matrix Transform::projection(const View_frustum& frt) {
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

bool Transform::cull(const Matrix& m) {
	// w < 0
	float x = m[0][0];
	float y = m[1][0];
	float z = m[2][0];
	float w = m[3][0];

	if (x > -w || x < w) return true;
	if (y > -w || y < w) return true;
	if (z > -w || z < w) return true;
	return false;
}

Matrix Transform::lerp(const Matrix& v1, const Matrix& v2, float t) {
	return Matrix(3, 3);
}

void Transform::clipByPlane(std::vector<Matrix>& verts, const Plane& plane) {
	std::vector<Matrix> clip;

	for (auto& vert : verts) {
		float d_ = vert ^ plane.m;
		if (d_ < 0)
			clip.push_back(vert);
	}

	clip.swap(verts);
}

std::vector<Matrix> Transform::vertices_process(std::vector<Vec3>& verts) {
	std::vector<Matrix> WRD_coords; WRD_coords.reserve(3);
	std::vector<Matrix> NDC_coords; NDC_coords.reserve(3);

	bool culling = false;

	for (int j = 0; j < 3; j++) {
		Matrix m_view = view_matrix * vtom(verts[j]);
		WRD_coords.emplace_back(m_view);
	}

	clipByPlane(WRD_coords, top);
	clipByPlane(WRD_coords, bottom);
	clipByPlane(WRD_coords, left);
	clipByPlane(WRD_coords, right);
	clipByPlane(WRD_coords, near);
	clipByPlane(WRD_coords, far);

	for (auto &WRD_coord: WRD_coords) {
		Matrix m_proj = project_matrix * WRD_coord;
		NDC_coords.emplace_back(m_proj);
	}

	if (NDC_coords.size() < 3)
		NDC_coords.clear();
	return NDC_coords;
}

std::vector<Vec3> Transform::viewport_process(std::vector<Matrix>& verts, const Matrix& viewport) {

	std::vector<Vec3> scr; scr.reserve(3);

	for (auto &NDC_coords: verts) {
		Vec3 v_ = mtov(viewport * NDC_coords);
		scr.push_back(v_);
	}

	return scr;
}