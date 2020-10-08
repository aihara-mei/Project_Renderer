#include "matrix.h"
#include <cassert>

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

Matrix::Matrix(const Matrix& n) {
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
	assert(data && i>=0 && i<row);
	return data[i];
}

Matrix Matrix::operator+(const Matrix& n) {
	assert(column == n.column && row == n.row);
	Matrix t(column, row);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[i][j] = data[i][j] + n[i][j];
		}
	}
	return t;
}

Matrix Matrix::operator-(const Matrix& n) {
	assert(column == n.column && row == n.row);
	Matrix t(column, row);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[i][j] = data[i][j] - n[i][j];
		}
	}
	return t;
}

Matrix Matrix::operator*(const Matrix& n) {
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

Matrix Matrix::inverse() {
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
		for (int j = i+1; j < row; j++) {
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
	Matrix t(row, column);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			t[i][j] = data[j][i];
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