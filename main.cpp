#include <iostream>
#include "matrix.h"

int main(int argc, char** argv) {
	Matrix a(3, 3), b(3, 3), c(3, 3);
	a[0][0] = 0;
	a[0][1] = 1;
	a[0][2] = 2;
	a[1][0] = 1;
	a[1][1] = 1;
	a[1][2] = 4;
	a[2][0] = 2;
	a[2][1] = -1;
	a[2][2] = 1;
	b = a;
	c = a.inverse() + b;

	Matrix d = Matrix::eye(5);

	std::cout << d;

	int name;
	std::cin >> name;
}