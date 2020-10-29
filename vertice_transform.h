#pragma once
#include "matrix.h"
#include "model.h"

struct View_frustum {
	float near, far;
	float l, b, r, t; // left, bottom, right, top
};

struct Plane {
	Vec3 n;
	float d;
	Matrix m;
	Plane(Vec3 n_, float d_) : n(n_), d(d_), m(4, 1) {
		float norm = std::sqrt(n_.x * n_.x + n_.y * n_.y + n_.z * n_.z);
		n = Vec3(n_.x / norm, n_.y / norm, n_.z / norm);

		m[0][0] = n.x;
		m[1][0] = n.y;
		m[2][0] = n.z;
		m[3][0] = d / norm;
	}
};

// camera position
static const Vec3 direction{ 1, 1, 1 };
static const Vec3 eye_pos{ 2, 2, 2 };
static const Vec3 up{ 0, 1, 0 };
// camera frustum
static const View_frustum frust{ -1, -30, -1, -1, 1, 1 }; //view in -z
// camera clipping plane
static const Plane top{ Vec3{ 0, -frust.near/frust.t, 1 }, 0 };
static const Plane bottom{ Vec3{ 0, -frust.near/frust.b, 1 }, 0 };
static const Plane near{ Vec3{ 0, 0, 1 }, -frust.near };
static const Plane far{ Vec3{ 0, 0, -1 }, frust.far };
static const Plane left{ Vec3{ -frust.near / frust.l, 0, 1 }, 0 };
static const Plane right{ Vec3{ -frust.near / frust.r, 0, 1 }, 0 };


static const Plane tt{ Vec3{ 0, 1, 0 }, -0.5 };

class Transform
{
public:
	static Matrix view(Vec3 look_direction, Vec3 eye_pos, Vec3 up);
	static Matrix viewport(int w, int h);
	static Matrix projection(float n, float f, float r, float l, float t, float b);
	static Matrix projection(const View_frustum& m);
	static bool cull(const Matrix& m);
	static Matrix lerp(const Matrix& v1, const Matrix& v2, float t);
	static void clipByPlane(std::vector<Matrix>& m, const Plane& plane);
	static std::vector<Matrix> vertices_process(std::vector<Vec3>& verts);
	static std::vector<Vec3> viewport_process(std::vector<Matrix>& verts, const Matrix& viewport);
};

// MVP matrix
static const Matrix view_matrix = Transform::view(direction, eye_pos, up);
static const Matrix project_matrix = Transform::projection(frust);