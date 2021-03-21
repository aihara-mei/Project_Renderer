#pragma once
#include "matrix.h"
#include "tgaimage.h"
#include "model.h"
#include <vector>

#define MAX_VERTEX 9
#define EPSILON 1e-5f
#define PI 3.1415926

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

struct light
{
	Vec3 pos;
	Vec3 intensity;
};

struct View_frustum {
	float near, far;
	float l, b, r, t; // left, bottom, right, top
};

typedef struct cubemap {
	TGAImage* faces[6];
}cubemap_t;

typedef struct iblmap {
	int mip_levels;
	cubemap_t* irradiance_map;
	cubemap_t* prefilter_maps[15];
	TGAImage* brdf_lut;
} iblmap_t;

struct payload_t
{

	// vertex attribute
	Vec3 normal[3];
	Vec2 uv[3];
	Vec3 world[3];
	Vec3 tri[3];
	Vec4 clip[3];
	//for homogeneous clipping
	Vec3 in_normal[MAX_VERTEX];
	Vec2 in_uv[MAX_VERTEX];
	Vec3 in_world[MAX_VERTEX];
	Vec4 in_clip[MAX_VERTEX];
	Vec3 out_normal[MAX_VERTEX];
	Vec2 out_uv[MAX_VERTEX];
	Vec3 out_world[MAX_VERTEX];
	Vec4 out_clip[MAX_VERTEX];
	// IBL
	iblmap_t* iblmap;
};


struct IShader {
	// transform matrix
	Matrix MVP;
	Matrix Viewport;
	
	// other attribute
	payload_t payload;

	virtual ~IShader();
	virtual Vec4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3 bar, Vec2 _uv, TGAColor& color) = 0;
};

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
void triangle(Vec4* vec, IShader& shader, float* zbuffer, TGAImage& image);

void lookat(Vec3 look_direction, Vec3 eye_pos, Vec3 up);
void viewport(int w, int h);
void projection_orth(float l, float r, float t, float b, float n, float f);
void projection_orth(const View_frustum& m);
void projection(float l, float r, float t, float b, float n, float f);
void projection(const View_frustum& m);
bool cull(const Matrix& m);

void load_ibl_map(payload_t& p, const char* env_path);

void draw_triangles(TGAImage& image, float* zbuffer, IShader& shader, int nface);
