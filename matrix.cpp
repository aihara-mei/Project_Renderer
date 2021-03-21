#include "matrix.h"
#include <cassert>

Matrix::Matrix() {
	row = 0, column = 0;
	data = nullptr;
}

Matrix::Matrix(int row_n, int column_n) {
	if (row_n <= 0 || column_n <= 0) {
		row = column = 0;
		data = nullptr;
		return;
	}

	row = row_n;
	column = column_n;
	data = new float* [row_n];
	for (int i = 0; i < row_n; i++) {
		data[i] = new float[column_n];

		for (int j = 0; j < column_n; j++) {
			data[i][j] = 0.;
		}
	}
}

Matrix::Matrix(const Matrix& n) :row(0), column(0), data(nullptr) {
	if (n.row <= 0 || n.column <= 0) {
		row = column = 0;
		data = nullptr;
		return;
	}

	row = n.row;
	column = n.column;
	data = new float* [row];
	for (int i = 0; i < row; i++) {
		data[i] = new float[column];

		for (int j = 0; j < column; j++) {
			data[i][j] = n[i][j];
		}
	}
}

Matrix::Matrix(Matrix&& n) :row(0), column(0), data(nullptr) {
	*this = std::move(n);
}

Matrix::~Matrix() {
	if (data) {
		for (int i = 0; i < row; i++) {
			delete[] data[i];
		}
		delete[] data;
	}
	data = nullptr;
}

float* Matrix::operator[](const int i) const {
	assert(data && i >= 0 && i < row);
	return data[i];
}

Matrix Matrix::operator+(const Matrix& n) const {
	assert(column == n.column && row == n.row);
	Matrix t(row, column);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[i][j] = data[i][j] + n[i][j];
		}
	}
	return t;
}

Matrix Matrix::operator-(const Matrix& n) const {
	assert(column == n.column && row == n.row);
	Matrix t(row, column);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[i][j] = data[i][j] - n[i][j];
		}
	}
	return t;
}

Matrix Matrix::operator*(const Matrix& n) const {
	assert(column == n.row);
	Matrix t(row, n.column);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < n.column; j++) {
			t[i][j] = 0;
			for (int k = 0; k < column; k++) {
				t[i][j] += data[i][k] * n[k][j];
			}
		}
	}
	return t;
}

Matrix Matrix::operator*(const float t) const {
	Matrix res(row, column);

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			res[i][j] = data[i][j] * t;
		}
	}
	return res;
}

float Matrix::operator^(const Matrix& n) const {
	assert(row == n.row);
	assert(column == n.column);

	float res = 0;
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			res += data[i][j] * n[i][j];
		}
	}
	return res;
}

Matrix& Matrix::operator=(const Matrix& n) {
	if (this == &n) {
		return *this;
	}

	if (this->data) {
		for (int i = 0; i < row; i++) {
			delete[] data[i];
		}
		delete[] data;
	}

	row = n.row;
	column = n.column;
	data = new float* [row];
	for (int i = 0; i < row; i++) {
		data[i] = new float[column];
	}
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			data[i][j] = n[i][j];
		}
	}
	return *this;
}

Matrix& Matrix::operator=(Matrix&& n) {
	if (this == &n) {
		return *this;
	}

	if (this->data) {
		for (int i = 0; i < row; i++) {
			delete[] data[i];
		}
		delete[] data;
	}

	row = n.row;
	column = n.column;
	data = n.data;

	n.data = nullptr;
	n.column = 0;
	n.row = 0;

	return *this;
}

Matrix Matrix::inverse() {
	assert(row == column);
	//Gauss-Jordan
	Matrix result(row, column * 2);
	for (int i = 0; i < row; i++) {
		result[i][i + column] = 1.;
		for (int j = 0; j < column; j++) {
			result[i][j] = data[i][j];
		}
	}

	for (int i = 0; i < row; i++) {
		// choosing partial pivot
		int maxRow = i;
		for (int j = i + 1; j < row; j++) {
			if (abs(result[j][i]) > abs(result[maxRow][i])) {
				maxRow = j;
			}
		}
		float coeff = result[maxRow][i];
		assert(abs(coeff) > 0.00001);
		std::swap(result.data[i], result.data[maxRow]);

		//dividing row i
		for (int j = 0; j < row * 2; j++) {
			result[i][j] = result[i][j] / coeff;
		}

		//eliminating row
		for (int j = 0; j < row; j++) {
			if (i != j) {
				float di = result[j][i];
				for (int k = 0; k < column * 2; k++) {
					result[j][k] -= result[i][k] * di;
				}
			}
		}
	}

	Matrix trunc(row, column);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			trunc[i][j] = result[i][j + column];
		}
	}
	return trunc;
}

Matrix Matrix::transpose() {
	Matrix t(column, row);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[j][i] = data[i][j];
		}
	}
	return t;
}

Matrix Matrix::eye(int row_n) {
	Matrix t(row_n, row_n);
	for (int i = 0; i < row_n; i++) {
		t[i][i] = 1.;
	}
	return t;
}

std::ostream& operator<<(std::ostream& cout, const Matrix& n) {
	for (int i = 0; i < n.row; i++) {
		for (int j = 0; j < n.column; j++) {
			cout << n[i][j] << " ,";
		}
		cout << std::endl;
	}
	return cout;
}

Matrix operator*(const float t, const Matrix& n) {
	Matrix res(n.row, n.column);
	for (int i = 0; i < n.row; i++) {
		for (int j = 0; j < n.column; j++) {
			res[i][j] = t * n[i][j];
		}
	}
	return res;
}

// vec2
Vec2 operator*(double t, const Vec2& v) {
	return Vec2(v.x * t, v.y * t);
}

Vec2 operator/(const Vec2& v, double t) {
	return Vec2(v.x / t, v.y / t);
}

// vec3
Vec3 normalize(const Vec3& v) {
	float norm = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	Vec3 t = Vec3(v.x / norm, v.y / norm, v.z / norm);
	return t;
}

Matrix vtom(const Vec3& v) {
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1;
	return m;
}

Vec3 mtov3(const Matrix& m) {
	Vec3 v;
	v.x = m[0][0] / m[3][0];
	v.y = m[1][0] / m[3][0];
	v.z = m[2][0] / m[3][0];
	return v;
}

void clamp_v3(Vec3& v, float c) {
	v.x = v.x > c ? c : v.x;
	v.y = v.y > c ? c : v.y;
	v.z = v.z > c ? c : v.z;
}

Vec3 operator*(double t, const Vec3& v) {
	return Vec3(v.x * t, v.y * t, v.z * t);
}

Vec3 operator-(const Vec3& u, const Vec3& v)
{
	return Vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

Vec3 operator*(const Vec3& u, const Vec3& v)
{
	return Vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

// vec4
Matrix vtom(const Vec4& v) {
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = v.w;
	return m;
}

Vec4 mtov4(const Matrix& m) {
	Vec4 v;
	v.x = m[0][0];
	v.y = m[1][0];
	v.z = m[2][0];
	v.w = m[3][0];
	return v;
}

Vec2 vec2_lerp(Vec2& start, Vec2& end, float alpha)
{
	return start + (end - start) * alpha;
}

Vec3 vec3_lerp(Vec3& start, Vec3& end, float alpha)
{
	return start + (end - start) * alpha;
}

Vec4 vec4_lerp(Vec4& start, Vec4& end, float alpha)
{
	return start + (end - start) * alpha;
}
