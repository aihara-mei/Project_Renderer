#pragma once
#include <iostream>
#include <cmath>
#include <array>
#include <utility>

class Matrix
{
public:
	Matrix inverse();
	Matrix transpose();
	Matrix operator*(const Matrix& n);
	Matrix operator+(const Matrix& n);
	Matrix operator-(const Matrix& n);
	Matrix& operator=(const Matrix& n);
	Matrix& operator=(Matrix&& n);
	float* operator[](const int i) const;
	Matrix(int row_n, int column_n);
	Matrix(const Matrix& n);
	Matrix(Matrix&& n);
	~Matrix();

	static Matrix eye(int row_n);
	friend std::ostream& operator<<(std::ostream& cout, const Matrix& n);
private:
	int row;
	int column;
	float** data;
};
