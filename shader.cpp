#include "shader.h"

void Shader::shade(TGAImage& image, Model* model) {

	// light direction
	const Vec3 light_dir(-1, -1, -1);

	// camera position
	const Vec3 direction(1, 1, 1);
	const Vec3 eye_pos(2, 2, 2);
	const Vec3 up(0, 1, 0);
	// camera frustum
	const View_frustum frust{ -1, -30, -0.4, -0.4, 0.4, 0.4 }; //view in -z

	// viewport size
	const int width = image.get_width();
	const int height = image.get_height();

	// MVP matrix
	Matrix view = Transform::view(direction, eye_pos, up);
	Matrix project = Transform::projection(frust);
	Matrix viewport = Transform::viewport(width, height);

	std::vector<float> zbuffer(width * height);
	for (int i = 0; i < width * height; i++) {
		zbuffer[i] = -std::numeric_limits<float>::max();
	}

	for (int i = 0; i < model->n_faces(); i++) {
		std::vector<int> face = model->getFace(i);
		Vec3 screen_coords[3];
		Vec3 world_coords[3];
		Vec3 uvs[3];
		float zs[3];
		for (int j = 0; j < 3; j++) {
			Vec3 v = model->getVert(face[j * 3]);
			Matrix m = project * view * vtom(v);
			zs[j] = m[3][0];
			Vec3 v_ = mtov(viewport * m);

			screen_coords[j] = v_;
			world_coords[j] = v;
			uvs[j] = model->getUV(face[j * 3 + 1]);
		}
		Vec3 n = cross(to(world_coords[2], world_coords[0]), to(world_coords[1], world_coords[0]));
		Vec3 nm = normalize(n);
		float intensity = dot(nm, normalize(light_dir));

		intensity = std::abs(intensity);
		if (intensity > 0) {
			Fragment::triangle(screen_coords, uvs, zs, zbuffer, image, intensity, model);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
}