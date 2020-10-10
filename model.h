#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "tgaimage.h"
#include "matrix.h"

struct Vec3 {
	Vec3(): x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float& operator[](const int i) { return i == 0 ? x : (i == 1 ? y : z); }
	float x, y, z;
};

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

Vec3 normalize(Vec3 v);

Matrix vtom(Vec3 v);

Vec3 mtov(Matrix m);

class Model
{
private:
	std::vector<Vec3> verts;
	std::vector<Vec3> uvs;
	std::vector<Vec3> norms;
	std::vector<std::vector<Vec3>> faces;
	TGAImage UVMap;
	void load_texture(std::string filename, const char* suffix);
public:
	int n_faces();
	Vec3 getVert(int i);
	Vec3 getUV(int i);
	Vec3 getNorm(int i);
	std::vector<int> getFace(int idx);
	TGAColor UVColor(Vec3 uv);
	Model(const char* filename);
};


