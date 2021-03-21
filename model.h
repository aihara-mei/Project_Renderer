#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "tgaimage.h"
#include "matrix.h"

class Model
{
private:
	std::vector<Vec3> verts;
	std::vector<Vec3> uvs;
	std::vector<Vec3> norms;
	std::vector<std::vector<Vec3>> faces;
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
	void load_texture(std::string filename, const char* suffix, TGAImage* img);
	void create_map(const char* filename);
public:
	TGAImage* diffusemap_;
	TGAImage* normalmap_;
	TGAImage* specularmap_;
	TGAImage* roughnessmap_;
	TGAImage* metalnessmap_;
	TGAImage* occlusion_map;
	TGAImage* emision_map;
	int n_faces();
	Vec3 getVert(int iface, int nthVert);
	Vec2 getUV(int iface, int nthVert);
	Vec3 getNorm(int iface, int nthVert);
	std::vector<int> getFace(int idx);
	Vec3 diffuse(Vec2 uv);
	Vec3 normal(Vec2 uv);
	float roughness(Vec2 uv);
	float metalness(Vec2 uv);
	Vec3 emission(Vec2 uv);
	float occlusion(Vec2 uv);
	float specular(Vec2 uv);
	Model(const char* filename);
	~Model();
};
