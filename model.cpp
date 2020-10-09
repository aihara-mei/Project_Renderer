#include "model.h"

Model::Model(const char* filename):verts(), uvs(), norms(), faces() {
	std::ifstream f;
	f.open(filename, std::ifstream::in);
	if (f.fail()) return;

	std::string line;
	while (!f.eof()) {
		std::getline(f, line);
		std::stringstream sstr(line);
		char waste;

		if (!line.compare(0, 2, "v ")) {
			sstr >> waste;
			Vec3 ve(0, 0, 0);
			sstr >> ve.x;
			sstr >> ve.y;
			sstr >> ve.z;
			verts.push_back(ve);
		}
		else if (!line.compare(0, 3, "vt ")) {
			sstr >> waste >> waste;
			Vec3 ve(0, 0, 0);
			sstr >> ve.x;
			sstr >> ve.y;
			uvs.push_back(ve);
		}
		else if (!line.compare(0, 3, "vn ")) {
			sstr >> waste >> waste;
			Vec3 ve(0, 0, 0);
			sstr >> ve.x;
			sstr >> ve.y;
			sstr >> ve.z;
			norms.push_back(ve);
		}
		else if (!line.compare(0, 2, "f ")) {
			sstr >> waste;
			std::vector<Vec3> onef;
			Vec3 ve(0, 0, 0);
			while (sstr >> ve.x >> waste >> ve.y >> waste >> ve.z) {
				ve.x--;
				ve.y--;
				ve.z--;
				onef.push_back(ve);
			}
			faces.push_back(onef);
		}
	}
	std::cout << "read Model:" << filename << "\n";
	load_texture(filename, "_texture.tga");
}


int Model::n_faces() {
	return faces.size();
}

Vec3 Model::getVert(int i) {
	return verts[i];
}

Vec3 Model::getUV(int i) {
	return uvs[i];
}

Vec3 Model::getNorm(int i) {
	return norms[i];
}

std::vector<int> Model::getFace(int i) {
	std::vector<int> t;
	for (Vec3 ve : faces[i]) {
		t.push_back(ve.x);
		t.push_back(ve.y);
		t.push_back(ve.z);
	}
	return t;
}

void Model::load_texture(std::string filename, const char* suffix) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (UVMap.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		UVMap.flip_vertically();
	}
}

TGAColor Model::UVColor(Vec3 uv) {
	return UVMap.get(uv.x, uv.y);
}