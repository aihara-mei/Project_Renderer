#include "fragment.h"

void Fragment::line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
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

void Fragment::triangle(std::vector<Vec3>& verts, TGAImage& image) {
	Vec3 v1 = verts[0], v2 = verts[1], v3 = verts[2];
	line(v1.x, v1.y, v2.x, v2.y, image, TGAColor(255, 255, 255));
	line(v2.x, v2.y, v3.x, v3.y, image, TGAColor(255, 255, 255));
	line(v1.x, v1.y, v3.x, v3.y, image, TGAColor(255, 255, 255));
}

void Fragment::triangle(Vec3* verts, Vec3* uvs, float* zs, std::vector<float>& zbuffer, TGAImage& image, float intensity, Model* model) {
	Vec3 v1 = verts[0], v2 = verts[1], v3 = verts[2];
	Vec3 uv1 = uvs[0], uv2 = uvs[1], uv3 = uvs[2];
	float z1 = zs[0], z2 = zs[1], z3 = zs[2];

	//bounding box
	int x_max = std::max(v1.x, std::max(v2.x, v3.x));
	int x_min = std::min(v1.x, std::min(v2.x, v3.x));
	int y_max = std::max(v1.y, std::max(v2.y, v3.y));
	int y_min = std::min(v1.y, std::min(v2.y, v3.y));

	Vec3 AC = to(v1, v3);
	Vec3 CB = to(v3, v2);
	Vec3 S = cross(AC, CB);
	float SS = dot(S, S);

	for (int i = x_min; i <= x_max; i++) {
		for (int j = y_min; j <= y_max; j++) {
			Vec3 P(i, j, 0);
			Vec3 PA = to(P, v1);
			Vec3 PC = to(P, v3);
			float S1 = dot(cross(PC, CB), S);
			float S2 = dot(cross(PA, AC), S);
			float alpha = S1 / SS, beta = S2 / SS, gamma = 1.f - alpha - beta;
			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				float zt = 1 / (alpha / z1 + beta / z2 + gamma / z3);
				int u = zt * (alpha * uv1.x / z1 + beta * uv2.x / z2 + gamma * uv3.x / z3);
				int v = zt * (alpha * uv1.y / z1 + beta * uv2.y / z2 + gamma * uv3.y / z3);
				TGAColor color = model->UVColor(Vec3(u, v, 0));
				int idx = i * image.get_width() + j;

				if (zt > zbuffer[idx]) {
					zbuffer[idx] = zt;
					image.set(i, j, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity));
				}
			}
		}
	}
}