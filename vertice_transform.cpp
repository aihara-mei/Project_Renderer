#include "vertice_transform.h"
#include "cassert"
#include <queue>;

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

Matrix Transform::lerp_matrix(const Matrix& v1, const Matrix& v2, double t) {
	return v1 * (1 - t) + v2 * t;
}

Vec3 Transform::lerp_vec3(const Vec3& v1, const Vec3& v2, float t) {
	return v1 * (1 - t) + v2 * t;
}

double Transform::get_intersect_ratio(float d1, float d2) {
	return std::fabs(d1) / (std::fabs(d1) + std::fabs(d2));
}

void Transform::clipByPlane_(std::vector<Matrix>& verts, const Plane& plane) {
	std::vector<Matrix> clip;
	int vert_num = verts.size();

	for (int i = 0; i < vert_num; i++) {
		Matrix cur_v = verts[i];
		float d1 = cur_v ^ plane.m;
		if (d1 < 0)
			clip.push_back(cur_v);
	}

	if (clip.size() < 3)
		clip.clear();

	verts.swap(clip);
}

void Transform::clipByPlane(std::vector<Matrix>& verts, const Plane& plane, std::vector<Vec3>& uvs) {
	std::vector<Matrix> clip;
	std::vector<Vec3> clip_uvs;

	int i;
	int vert_num = verts.size();
	int post, cur;

	for (i = 0; i < vert_num; i++) {
		cur = i;
		post = (i + 1) % vert_num;
		Matrix cur_v = verts[cur];
		Matrix post_v = verts[post];

		Vec3 cur_uv = uvs[cur];
		Vec3 post_uv = uvs[post];

		float d1 = cur_v ^ plane.m;
		float d2 = post_v ^ plane.m;

		if (d1 * d2 < 0) {
			double t = get_intersect_ratio(d1, d2);
			Matrix intsec = lerp_matrix(cur_v, post_v, t);
			Vec3 intsec_uv = lerp_vec3(cur_uv, post_uv, t);
			
			clip.push_back(intsec);
			clip_uvs.push_back(intsec_uv);
		}
		
		if (d2 < 0) {
			clip.push_back(post_v);
			clip_uvs.push_back(post_uv);
		}
	}
	clip.swap(verts);
	clip_uvs.swap(uvs);
}

std::vector<Matrix> Transform::vertices_process(std::vector<Vec3>& verts, std::vector<Vec3>& uvs, std::vector<float>& zs) {
	std::vector<Matrix> WRD_coords; WRD_coords.reserve(3);
	std::vector<Matrix> NDC_coords; NDC_coords.reserve(3);

	bool culling = false;

	for (int j = 0; j < 3; j++) {
		Matrix m_view = view_matrix * vtom(verts[j]);
		WRD_coords.emplace_back(m_view);
	}

	clipByPlane(WRD_coords, top, uvs);
	clipByPlane(WRD_coords, bottom, uvs);
	clipByPlane(WRD_coords, left, uvs);
	clipByPlane(WRD_coords, right, uvs);
	clipByPlane(WRD_coords, near, uvs);
	clipByPlane(WRD_coords, far, uvs);

	for (auto &WRD_coord: WRD_coords) {
		Matrix m_proj = project_matrix * WRD_coord;
		NDC_coords.emplace_back(m_proj);
		zs.push_back(m_proj[3][0]);
	}

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

void Transform::pack_triangles(int nums, std::vector<std::tuple<int, int, int>>& triangles_idx) {
	std::queue<int> q;
	for (int i=0; i<nums; i++)
		q.push(i);
	while (q.size() > 2) {
		auto first = q.front();
		q.pop();
		auto second = q.front();
		q.pop();
		q.push(first);
		auto third = q.front();
		triangles_idx.push_back({ first, second, third });
	}
}