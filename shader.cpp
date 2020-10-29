#include "shader.h"

// light direction
static const Vec3 light_dir{ -1, -1, -1 };

void Shader::shade(TGAImage& image, Model* model) {

	// viewport size
	const int width = image.get_width();
	const int height = image.get_height();

	const Matrix viewport = Transform::viewport(width, height);

	// z-buffer
	std::vector<float> zbuffer(width * height);
	for (int i = 0; i < width * height; i++) {
		zbuffer[i] = -std::numeric_limits<float>::max();
	}

	for (int i = 0; i < model->n_faces(); i++) {
		std::vector<int> face = model->getFace(i);
		std::vector<Vec3> verts;
		std::vector<Vec3> uvs;
		std::vector<float> zs;

		for (int j = 0; j < 3; j++) {
			verts.push_back(model->getVert(face[j * 3]));
			uvs.push_back(model->getUV(face[j * 3 + 1]));
		}

		std::vector<Matrix> trans_verts = Transform::vertices_process(verts);

		std::vector<Vec3> triangle = Transform::viewport_process(trans_verts, viewport);

		if (!triangle.empty())
			Fragment::triangle(triangle, image);
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
}