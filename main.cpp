#include <iostream>
#include <limits>
#include "model.h"
#include "tgaimage.h"
#include "fragment.h"
#include "vertice_transform.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const Vec3 light_dir(0, 0, -1);

const Vec3 direction(0, 0, 1);
const Vec3 eye_pos(0, 0, 4);
const Vec3 up(0, 1, 0);
const View_frustum frust { -1, -30, -0.6, -0.6, 0.6, 0.6 };

const int width = 800;
const int height = 800;

int main(int argc, char** argv) {
	Model* model = new Model("./obj/cube.obj");
	TGAImage image(width, height, TGAImage::RGB);
	Fragment fragment(model);
	Transform transform;

	Matrix view = transform.view(direction, eye_pos, up);
	Matrix project = transform.projection_R(frust);
	Matrix viewport = transform.viewport(width, height);

	float* zbuffer = new float[width * height];
	for (int i = 0; i < width * height; i++) {
		zbuffer[i] = -std::numeric_limits<float>::max();
	}

	for (int i = 0; i < model->n_faces(); i++) {
		std::vector<int> face = model->getFace(i);
		Vec3 screen_coords[3];
		Vec3 world_coords[3];
		Vec3 uvs[3];
		for (int j = 0; j < 3; j++) {
			Vec3 v = model->getVert(face[j * 3]);
			Vec3 v_ = mtov(viewport * project * view * vtom(v));

			if (v_.z > 0) std::cout << "E";

			screen_coords[j] = v_;
			world_coords[j] = v;
			uvs[j] = model->getUV(face[j * 3 + 1]);
		}
		Vec3 n = cross(to(world_coords[2], world_coords[0]), to(world_coords[1], world_coords[0]));
		Vec3 nm = normalize(n);
		float intensity = dot(nm, normalize(light_dir));

		intensity = std::abs(intensity);
		if (intensity > 0) {
			fragment.triangle(screen_coords, uvs, zbuffer, image, intensity);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");

	delete zbuffer;
	delete model;
	return 0;
}