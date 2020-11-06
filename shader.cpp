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

		std::vector<Matrix> trans_verts = Transform::vertices_process(verts, uvs, zs);
		std::vector<Vec3> verts_list = Transform::viewport_process(trans_verts, viewport);
		std::vector<std::tuple<int, int, int>> triangles_idx;
		Transform::pack_triangles(verts_list.size(), triangles_idx);

		for (auto &idx: triangles_idx) {
			auto [a, b, c] = idx;
			Vec3 triangle[3] = { verts_list[a], verts_list[b], verts_list[c] };
			Vec3 uv[3] = { uvs[a], uvs[b], uvs[c] };
			float z[3] = { zs[a], zs[b], zs[c] };

			Fragment::triangle(triangle, uv, z, zbuffer, image, 1, model);
			//Fragment::triangle(triangle, image);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
}