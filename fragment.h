#pragma once
#include "tgaimage.h"
#include "model.h"

class Fragment
{
public:
	static void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
	static void triangle(Vec3* verts, Vec3* uvs, std::vector<float>& zbuffer, TGAImage& image, float intensity, Model* model);
};
