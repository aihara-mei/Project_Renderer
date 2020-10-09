#include <iostream>
#include <limits>
#include "matrix.h"
#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 800;
const int height = 800;

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

Vec3 normalize(Vec3 v) {
	float norm = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	Vec3 t = Vec3(v.x / norm, v.y / norm, v.z / norm);
	return t;
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

void __triangle(Vec3 v1, Vec3 v2, Vec3 v3, TGAImage& image, TGAColor color) {
	//eliminate degenerated triangles
	if (v1.y == v2.y && v2.y == v3.y) return;

	if (v1.y > v2.y) std::swap(v1, v2);
	if (v1.y > v3.y) std::swap(v1, v3);
	if (v2.y > v3.y) std::swap(v2, v3);

	for (int y = v1.y; y <= v2.y; y++) {
		float t0 = float(y - v1.y) / (v3.y - v1.y + 1);
		int x0 = (1 - t0) * v1.x + t0 * v3.x;
		float t1 = float(y - v1.y) / (v2.y - v1.y + 1);
		int x1 = (1 - t1) * v1.x + t1 * v2.x;
		//if (t0<=1 && t0>=0 && t1<=1 && t1>=0)
		line(x0, y, x1, y, image, color);
	}

	for (int y = v2.y; y <= v3.y; y++) {
		float t0 = float(y - v2.y) / (v3.y - v2.y + 1);
		int x0 = (1 - t0) * v2.x + t0 * v3.x;
		float t1 = float(y - v1.y) / (v3.y - v1.y + 1);
		int x1 = (1 - t1) * v1.x + t1 * v3.x;
		//if (t0 <= 1 && t0 >= 0 && t1 <= 1 && t1 >= 0)
		line(x0, y, x1, y, image, color);
	}
}

void triangle(Vec3 v1, Vec3 v2, Vec3 v3, float* zbuffer, TGAImage& image, TGAColor color) {
	if (v1.y > v2.y) std::swap(v1, v2);
	if (v1.y > v3.y) std::swap(v1, v3);
	if (v2.y > v3.y) std::swap(v2, v3);
	// v1 at bottom; v3 is at right or at least up of v2; so that v1 -> v3 -> v2 is counterclockwise. 
	if (v2.x > v3.x) std::swap(v2, v3);

	//bounding box
	int x_max = std::max(v1.x, std::max(v2.x, v3.x));
	int x_min = std::min(v1.x, std::min(v2.x, v3.x));
	int y_max = std::max(v1.y, std::max(v2.y, v3.y));
	int y_min = std::min(v1.y, std::min(v2.y, v3.y));

	Vec3 AC = to(v1, v3);
	Vec3 CB = to(v3, v2);
	Vec3 BA = to(v2, v1);
	Vec3 S = cross(AC, CB);
	float SS = dot(S, S);

	for (int i = x_min; i <= x_max; i++) {
		for (int j = y_min; j <= y_max; j++) {
			Vec3 P(i, j, 0);
			Vec3 PA = to(P, v1);
			Vec3 PC = to(P, v3);
			Vec3 PB = to(P, v2);
			float S1 = dot(cross(PC, CB), S);
			float S2 = dot(cross(PA, AC), S);
			float S3 = dot(cross(PB, BA), S);
			float alpha = S1 / SS, beta = S2 / SS, gamma = S3 / SS;
			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				int z = alpha * v1.z + beta * v2.z + gamma * v3.z;
				if (z < zbuffer[i * width + j]) {
					zbuffer[i * width + j] = z;
					image.set(i, j, color);
				}
			}
		}
	}

}

int main(int argc, char** argv) {
	
	Model model("./obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	Vec3 light_dir(0, 0, -1);

	float* zbuffer = new float[width * height];
	for (int i = 0; i < width * height; i++) {
		zbuffer[i] = std::numeric_limits<float>::max();
	}

	for (int i = 0; i < model.n_faces(); i++) {
		std::vector<int> face = model.getFace(i);
		Vec3 screen_coords[3];
		Vec3 world_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3 v = model.getVert(face[j * 3]);
			screen_coords[j] = Vec3((v.x + 1.) * width / 2., (v.y + 1.) * height / 2., v.z);
			world_coords[j] = v;
		}
		Vec3 n = cross(to(world_coords[2], world_coords[0]), to(world_coords[1], world_coords[0]));
		Vec3 nm = normalize(n);
		float intensity = dot(nm, light_dir);

		if (intensity > 0) {
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255));
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");

	delete zbuffer;
	return 0;
}