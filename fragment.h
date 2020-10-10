#pragma once
#include "tgaimage.h"
#include "model.h"

class Fragment
{
public:
	void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
	void triangle(Vec3* verts, Vec3* uvs, float* zbuffer, TGAImage& image, float intensity);
	Fragment(Model* m);
private:
	Model* model;
};
