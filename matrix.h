#pragma once
#include <iostream>
#include <cmath>
#include <array>
#include <utility>

// Vector
struct Vec2 {
	Vec2() : x(0), y(0) {}
	Vec2(float _x, float _y) : x(_x), y(_y) {}
	float& operator[](const int i) { return i == 0 ? x : y; }
	float operator[](const int i) const { return i == 0 ? x : y; }
	Vec2 operator*(const float t) const { return Vec2(x * t, y * t); }
	Vec2 operator*(const Vec2& v2) const { return Vec2(x * v2.x, y * v2.y); }
	Vec2 operator+(const Vec2& v2) { return Vec2(x + v2.x, y + v2.y); }
	Vec2 operator-(const Vec2& v2) { return Vec2(x - v2.x, y - v2.y); }
	float x, y;
};

struct Vec3 {
	Vec3() : x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float& operator[](const int i) { return i == 0 ? x : (i == 1 ? y : z); }
	float operator[](const int i) const { return i == 0 ? x : (i == 1 ? y : z); }
	Vec3 operator*(const float t) const { return Vec3(x * t, y * t, z * t); }
	Vec3 operator+(const Vec3& v2) { return Vec3(x + v2.x, y + v2.y, z + v2.z); }
	Vec3 operator-(const Vec3& v2) { return Vec3(x - v2.x, y - v2.y, z - v2.z); }
	Vec3 operator/(const float t) { return Vec3(x / t, y / t, z / t); }
	float norm_squared() const { return x * x + y * y + z * z; }
	float x, y, z;
};

struct Vec4 {
	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	float& operator[](const int i) { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
	Vec4 operator*(const float t) const { return Vec4(x * t, y * t, z * t, w * t); }
	Vec4 operator+(const Vec4& v2) { return Vec4(x + v2.x, y + v2.y, z + v2.z, w + v2.w); }
	Vec4 operator-(const Vec4& v2) { return Vec4(x - v2.x, y - v2.y, z - v2.z, w - v2.w); }
	float x, y, z, w;
};

class Matrix
{
public:
	Matrix inverse();
	Matrix transpose();
	Matrix operator*(const Matrix& n) const;
	Matrix operator*(const float t) const;
	Matrix operator+(const Matrix& n) const;
	Matrix operator-(const Matrix& n) const;
	float operator^(const Matrix& n) const;
	Matrix& operator=(const Matrix& n);
	Matrix& operator=(Matrix&& n);
	float* operator[](const int i) const;

	Matrix();
	Matrix(int row_n, int column_n);
	Matrix(const Matrix& n);
	Matrix(Matrix&& n);
	~Matrix();

	static Matrix eye(int row_n);
	friend std::ostream& operator<<(std::ostream& cout, const Matrix& n);
	friend Matrix operator*(const float t, const Matrix& n);
private:
	int row;
	int column;
	float** data;
};

inline float float_max(float a, float b)
{
	return a > b ? a : b;
}

inline float float_clamp(float f, float min, float max) {
	return f < min ? min : (f > max ? max : f);
}

// vec2
Vec2 operator*(double t, const Vec2& v);
Vec2 operator/(const Vec2& v, double t);

// vec3
inline Vec3 to(Vec3 v1, Vec3 v2) {
	Vec3 t(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
	return t;
}

inline Vec3 cross(Vec3 a, Vec3 b) {
	Vec3 t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	return t;
}

inline float dot(Vec3 v1, Vec3 v2) {
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline Vec3 cwise_product(const Vec3& a, const Vec3& b)
{
	return Vec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

inline Vec3 bary_inter(Vec3* v, Vec3 bar) {
	return Vec3(
		v[0][0] * bar.x + v[1][0] * bar.y + v[2][0] * bar.z,
		v[0][1] * bar.x + v[1][1] * bar.y + v[2][1] * bar.z,
		v[0][2] * bar.x + v[1][2] * bar.y + v[2][2] * bar.z
	);
}

Vec3 normalize(const Vec3& v);

Matrix vtom(const Vec3& v);

Vec3 mtov3(const Matrix& m);

void clamp_v3(Vec3& v, float c);

Vec3 operator*(double t, const Vec3& v);

Vec3 operator-(const Vec3& u, const Vec3& v);

Vec3 operator*(const Vec3& u, const Vec3& v);

// vec4
Matrix vtom(const Vec4& v);

Vec4 mtov4(const Matrix& m);

inline Vec4 bary_inter4(Vec4* v, Vec3 bar) {
	return Vec4(
		v[0][0] * bar.x + v[1][0] * bar.y + v[2][0] * bar.z,
		v[0][1] * bar.x + v[1][1] * bar.y + v[2][1] * bar.z,
		v[0][2] * bar.x + v[1][2] * bar.y + v[2][2] * bar.z,
		v[0][2] * bar.x + v[1][2] * bar.y + v[2][2] * bar.z
	);
}

// lerp
Vec2 vec2_lerp(Vec2& start, Vec2& end, float alpha);

Vec3 vec3_lerp(Vec3& start, Vec3& end, float alpha);

Vec4 vec4_lerp(Vec4& start, Vec4& end, float alpha);