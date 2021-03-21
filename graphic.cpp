#include "graphic.h"
#include "cassert"
#include <queue>;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

static int is_back_facing(Vec3 ndc_pos[3])
{
	Vec3 a = ndc_pos[0];
	Vec3 b = ndc_pos[1];
	Vec3 c = ndc_pos[2];
	float signed_area = a.x * b.y - a.y * b.x +
		b.x * c.y - b.y * c.x +
		c.x * a.y - c.y * a.x;   //|AB AC|
	return signed_area <= 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	// sort the x, y so that dx < 1
	bool changed = false;
	if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
		changed = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	// sort the points so that x0 <= x1
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dy = y1 - y0;
	int step = dy > 0 ? 1 : -1;
	dy = std::abs(dy);

	int y = y0;
	int d = 0;
	for (int i = x0; i <= x1; i++) {
		if (changed) {
			image.set(y, i, color);
		}
		else {
			image.set(i, y, color);
		}
		d += dy;
		if (2 * d > x1 - x0) {
			d -= (x1 - x0);
			y += step;
		}
	}
}

void triangle(Vec4* verts, IShader &shader, float* zbuffer, TGAImage& image) {

	int width = image.get_width();

	Vec3 v1 = mtov3(shader.Viewport * vtom(verts[0]));
	Vec3 v2 = mtov3(shader.Viewport * vtom(verts[1]));
	Vec3 v3 = mtov3(shader.Viewport * vtom(verts[2]));

	Vec3 v[3] = { v1, v2, v3 };
	//backface clip
	if (is_back_facing(v))
		return;

	/*Vec3 v1 = verts[0], v2 = verts[1], v3 = verts[2];*/
	float z1 = v1.z, z2 = v2.z, z3 = v3.z;

	//bounding box
	int x_max = std::max(v1.x, std::max(v2.x, v3.x));
	int x_min = std::min(v1.x, std::min(v2.x, v3.x));
	int y_max = std::max(v1.y, std::max(v2.y, v3.y));
	int y_min = std::min(v1.y, std::min(v2.y, v3.y));

	Vec3 AC = to(v1, v3);
	Vec3 CB = to(v3, v2);
	Vec3 S = cross(AC, CB);
	float SS = dot(S, S);

	Vec2 uv;

	for (int i = x_min; i <= x_max; i++) {
		for (int j = y_min; j <= y_max; j++) {
			Vec3 P(i, j, 0);
			Vec3 PA = to(P, v1);
			Vec3 PC = to(P, v3);
			float S1 = dot(cross(PC, CB), S);
			float S2 = dot(cross(PA, AC), S);
			float alpha = S1 / SS, beta = S2 / SS, gamma = 1.f - alpha - beta;

			Vec3 bar(alpha, beta, gamma);
			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				//perspective-correct interpolation
				float z = 1 / (alpha / z1 + beta / z2 + gamma / z3);
				 uv.x = z * (alpha * shader.payload.uv[0].x / z1 + beta * shader.payload.uv[1].x / z2 + gamma * shader.payload.uv[2].x / z3);
				 uv.y = z * (alpha * shader.payload.uv[0].y / z1 + beta * shader.payload.uv[1].y / z2 + gamma * shader.payload.uv[2].y / z3);

				TGAColor color;
				shader.fragment(bar, uv, color);
				int idx = i * width + j;

				if (z > zbuffer[idx]) {
					zbuffer[idx] = z;
					image.set(i, j, TGAColor(color.r , color.g, color.b));
				}
			}
		}
	}

}

void lookat(Vec3 look_direction, Vec3 eye_pos, Vec3 up) {
	Vec3 z = normalize(look_direction);
	Vec3 x = normalize(cross(up, z));
	Vec3 y = normalize(cross(z, x));
	Matrix r = Matrix::eye(4);
	Matrix t = Matrix::eye(4);
	for (int i = 0; i < 3; i++) {
		r[0][i] = x[i];
		r[1][i] = y[i];
		r[2][i] = z[i];
		t[i][3] = -eye_pos[i];
	}
	ModelView = r * t;
}

void viewport(int w, int h) {
	Matrix m = Matrix::eye(4);
	m[0][3] = w / 2.f;
	m[1][3] = h / 2.f;
	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	Viewport = m;
}

// origin orthographic matrix
void projection_orth(float l, float r, float t, float b, float n, float f) {
	Matrix m = Matrix(4, 4);
	m[0][0] = 2. / (r - l);
	m[0][3] = (r + l) / (l - r);

	m[1][1] = 2. / (t - b);
	m[1][3] = (t + b) / (b - t);

	m[2][2] = 2. / (n - f);
	m[2][3] = (n + f) / (f - n);

	m[3][3] = 1.;
	Projection = m;
}

// orthographic matrix with w = -1, to use homogeneous clipping
void projection_orth(const View_frustum& frt) {
	float n, f, r, l, b, t;
	n = frt.near;
	f = frt.far;
	r = frt.r;
	l = frt.l;
	b = frt.b;
	t = frt.t;

	Matrix m = Matrix(4, 4);
	m[0][0] = 2. / (l - r);
	m[0][3] = (r + l) / (r - l);

	m[1][1] = 2. / (b - t);
	m[1][3] = (t + b) / (t - b);

	m[2][2] = 2. / (f - n);
	m[2][3] = (n + f) / (n - f);

	m[3][3] = -1.;
	Projection = m;
}

void projection(float l, float r, float t, float b, float n, float f) {
	Matrix m = Matrix(4, 4);
	m[0][0] = 2. * n / (r - l);
	m[0][2] = (r + l) / (l - r);

	m[1][1] = 2. * n / (t - b);
	m[1][2] = (t + b) / (b - t);

	m[2][2] = (f + n) / (n - f);
	m[2][3] = 2. * f * n / (f - n);

	m[3][2] = 1.;
	Projection = m;
}

void projection(const View_frustum& frt) {
	float n, f, r, l, b, t;
	n = frt.near;
	f = frt.far;
	r = frt.r;
	l = frt.l;
	b = frt.b;
	t = frt.t;

	Matrix m = Matrix(4, 4);
	m[0][0] = 2. * n / (r - l);
	m[0][2] = (r + l) / (l - r);

	m[1][1] = 2. * n / (t - b);
	m[1][2] = (t + b) / (b - t);

	m[2][2] = (f + n) / (n - f);
	m[2][3] = 2. * f * n / (f - n);

	m[3][2] = 1.;
	Projection = m;
}

// obsolete
bool cull(const Matrix& m) {
	// w < 0
	float x = m[0][0];
	float y = m[1][0];
	float z = m[2][0];
	float w = m[3][0];

	if (x > -w || x < w) return true;
	if (y > -w || y < w) return true;
	if (z > -w || z < w) return true;
	return false;
}

TGAImage* texture_from_file(const char* file_name)
{
	TGAImage* texture = new TGAImage();
	texture->read_tga_file(file_name);
	texture->flip_vertically();
	return texture;
}

cubemap_t* cubemap_from_files(const char* positive_x, const char* negative_x,
	const char* positive_y, const char* negative_y,
	const char* positive_z, const char* negative_z)
{
	cubemap_t* cubemap = new cubemap_t();
	cubemap->faces[0] = texture_from_file(positive_x);
	cubemap->faces[1] = texture_from_file(negative_x);
	cubemap->faces[2] = texture_from_file(positive_y);
	cubemap->faces[3] = texture_from_file(negative_y);
	cubemap->faces[4] = texture_from_file(positive_z);
	cubemap->faces[5] = texture_from_file(negative_z);
	return cubemap;
}

void load_ibl_map(payload_t& p, const char* env_path)
{
	int i, j;
	iblmap_t* iblmap = new iblmap_t();
	const char* faces[6] = { "px", "nx", "py", "ny", "pz", "nz" };
	char paths[6][256];

	iblmap->mip_levels = 10;

	/* diffuse environment map */
	for (j = 0; j < 6; j++) {
		sprintf_s(paths[j], "%s/i_%s.tga", env_path, faces[j]);
	}
	iblmap->irradiance_map = cubemap_from_files(paths[0], paths[1], paths[2],
		paths[3], paths[4], paths[5]);

	/* specular environment maps */
	for (i = 0; i < iblmap->mip_levels; i++) {
		for (j = 0; j < 6; j++) {
			sprintf_s(paths[j], "%s/m%d_%s.tga", env_path, i, faces[j]);
		}
		iblmap->prefilter_maps[i] = cubemap_from_files(paths[0], paths[1],
			paths[2], paths[3], paths[4], paths[5]);
	}

	/* brdf lookup texture */
	iblmap->brdf_lut = texture_from_file("./obj/common/BRDF_LUT.tga");

	p.iblmap = iblmap;
}

//clipping
typedef enum {
	W_PLANE,
	X_RIGHT,
	X_LEFT,
	Y_TOP,
	Y_BOTTOM,
	Z_NEAR,
	Z_FAR
} clip_plane;

static int is_inside_plane(clip_plane c_plane, Vec4 vertex)
{
	switch (c_plane)
	{
	case W_PLANE:
		return vertex.w < -EPSILON;
	case X_RIGHT:
		return vertex.x > vertex.w;
	case X_LEFT:
		return vertex.x < -vertex.w;
	case Y_TOP:
		return vertex.y > vertex.w;
	case Y_BOTTOM:
		return vertex.y < -vertex.w;
	case Z_NEAR:
		return vertex.z > vertex.w;
	case Z_FAR:
		return vertex.z < -vertex.w;
	default:
		return 0;
	}
}

static float get_intersect_ratio(Vec4 prev, Vec4 curv, clip_plane c_plane)
{
	switch (c_plane)
	{
	case W_PLANE:
		return (prev.w + EPSILON) / (prev.w - curv.w);
	case X_RIGHT:
		return (prev.w - prev.x) / ((prev.w - prev.x) - (curv.w - curv.x));
	case X_LEFT:
		return (prev.w + prev.x) / ((prev.w + prev.x) - (curv.w + curv.x));
	case Y_TOP:
		return (prev.w - prev.y) / ((prev.w - prev.y) - (curv.w - curv.y));
	case Y_BOTTOM:
		return (prev.w + prev.y) / ((prev.w + prev.y) - (curv.w + curv.y));
	case Z_NEAR:
		return (prev.w - prev.z) / ((prev.w - prev.z) - (curv.w - curv.z));
	case Z_FAR:
		return (prev.w + prev.z) / ((prev.w + prev.z) - (curv.w + curv.z));
	default:
		return 0;
	}
}

static int clip_with_plane(clip_plane c_plane, int num_vert, payload_t& payload)
{
	int i;
	int out_vert_num = 0;
	int previous_index, current_index;
	int is_odd = (c_plane + 1) % 2;

	Vec4* in_clipcoord = is_odd ? payload.in_clip : payload.out_clip;
	Vec3* in_worldcoord = is_odd ? payload.in_world : payload.out_world;
	Vec3* in_normal = is_odd ? payload.in_normal : payload.out_normal;
	Vec2* in_uv = is_odd ? payload.in_uv : payload.out_uv;
	Vec4* out_clipcoord = is_odd ? payload.out_clip : payload.in_clip;
	Vec3* out_worldcoord = is_odd ? payload.out_world : payload.in_world;
	Vec3* out_normal = is_odd ? payload.out_normal : payload.in_normal;
	Vec2* out_uv = is_odd ? payload.out_uv : payload.in_uv;

	for (i = 0; i < num_vert; i++)
	{
		//从最后一个点开始，遍历所有边
		current_index = i;
		previous_index = (i - 1 + num_vert) % num_vert;
		Vec4 cur_vertex = in_clipcoord[current_index];
		Vec4 pre_vertex = in_clipcoord[previous_index];

		int is_cur_inside = is_inside_plane(c_plane, cur_vertex);
		int is_pre_inside = is_inside_plane(c_plane, pre_vertex);

		if (is_cur_inside != is_pre_inside)
		{
			float ratio = get_intersect_ratio(pre_vertex, cur_vertex, c_plane);

			out_clipcoord[out_vert_num] = vec4_lerp(pre_vertex, cur_vertex, ratio);
			out_worldcoord[out_vert_num] = vec3_lerp(in_worldcoord[previous_index], in_worldcoord[current_index], ratio);
			out_normal[out_vert_num] = vec3_lerp(in_normal[previous_index], in_normal[current_index], ratio);
			out_uv[out_vert_num] = vec2_lerp(in_uv[previous_index], in_uv[current_index], ratio);

			out_vert_num++;
		}

		if (is_cur_inside)
		{
			out_clipcoord[out_vert_num] = cur_vertex;
			out_worldcoord[out_vert_num] = in_worldcoord[current_index];
			out_normal[out_vert_num] = in_normal[current_index];
			out_uv[out_vert_num] = in_uv[current_index];

			out_vert_num++;
		}
	}

	return out_vert_num;
}

static int homo_clipping(payload_t& payload)
{
	int num_vertex = 3;
	num_vertex = clip_with_plane(W_PLANE, num_vertex, payload);
	num_vertex = clip_with_plane(X_RIGHT, num_vertex, payload);
	num_vertex = clip_with_plane(X_LEFT, num_vertex, payload);
	num_vertex = clip_with_plane(Y_TOP, num_vertex, payload);
	num_vertex = clip_with_plane(Y_BOTTOM, num_vertex, payload);
	num_vertex = clip_with_plane(Z_NEAR, num_vertex, payload);
	num_vertex = clip_with_plane(Z_FAR, num_vertex, payload);
	return num_vertex;
}

static void transform_attri(payload_t& payload, int index0, int index1, int index2)
{
	payload.clip[0] = payload.out_clip[index0];
	payload.clip[1] = payload.out_clip[index1];
	payload.clip[2] = payload.out_clip[index2];
	payload.world[0] = payload.out_world[index0];
	payload.world[1] = payload.out_world[index1];
	payload.world[2] = payload.out_world[index2];
	payload.normal[0] = payload.out_normal[index0];
	payload.normal[1] = payload.out_normal[index1];
	payload.normal[2] = payload.out_normal[index2];
	payload.uv[0] = payload.out_uv[index0];
	payload.uv[1] = payload.out_uv[index1];
	payload.uv[2] = payload.out_uv[index2];
}


void draw_triangles(TGAImage& image, float* zbuffer, IShader& shader, int nface)
{
	int i;
	//vertex shader
	for (i = 0; i < 3; i++)
	{
		shader.vertex(nface, i);
	}

	//homogeneous clipping
	int num_vertex = homo_clipping(shader.payload);

	//triangle assembly
	for (i = 0; i < num_vertex - 2; i++) {
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;
		//transform data to real vertex attri
		transform_attri(shader.payload, index0, index1, index2);

		triangle(shader.payload.clip, shader, zbuffer, image);
	}
}
