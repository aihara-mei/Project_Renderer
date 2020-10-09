#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "tgaimage.h"

struct Vec3 {
	Vec3(): x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float x, y, z;
};

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


