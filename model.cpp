#include "model.h"
#include <io.h> 

Model::Model(const char* filename) :verts(), uvs(), norms(), faces() {

	diffusemap_ = NULL;
	normalmap_ = NULL;
	specularmap_ = NULL;
	roughnessmap_ = NULL;
	metalnessmap_ = NULL;
	occlusion_map = NULL;
	emision_map = NULL;

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
	create_map(filename);
}

Model::~Model() {
	delete diffusemap_;
	delete normalmap_;
	delete specularmap_;
	delete roughnessmap_;
	delete metalnessmap_;
	delete occlusion_map;
	delete emision_map;
}

int Model::n_faces() {
	return faces.size();
}

Vec3 Model::getVert(int iface, int nthVert) {
	int nth = faces[iface][nthVert][0];
	return verts[nth];
}

Vec3 Model::getNorm(int iface, int nthVert) {
	int nth = faces[iface][nthVert][2];
	return norms[nth];
}

Vec2 Model::getUV(int iface, int nthVert) {
	int nth = faces[iface][nthVert][1];
	uvs[nth].x = fmod(uvs[nth].x, 1);
	uvs[nth].y = fmod(uvs[nth].y, 1);
	return Vec2(uvs[nth].x, uvs[nth].y);
}

std::vector<int> Model::getFace(int i) {
	std::vector<int> t;
	for (Vec3& ve : faces[i]) {
		t.push_back(ve.x);
		t.push_back(ve.y);
		t.push_back(ve.z);
	}
	return t;
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage* img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img->read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		//img->read_tga_file(texfile.c_str());
		img->flip_vertically();
	}
}

void Model::create_map(const char* filename)
{
	diffusemap_ = NULL;
	normalmap_ = NULL;
	specularmap_ = NULL;
	roughnessmap_ = NULL;
	metalnessmap_ = NULL;
	occlusion_map = NULL;
	emision_map = NULL;


	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");

	texfile = texfile.substr(0, dot) + std::string("_diffuse.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		diffusemap_ = new TGAImage();
		load_texture(filename, "_diffuse.tga", diffusemap_);
	}

	texfile = texfile.substr(0, dot) + std::string("_normal.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		normalmap_ = new TGAImage();
		load_texture(filename, "_normal.tga", normalmap_);
	}

	texfile = texfile.substr(0, dot) + std::string("_spec.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		specularmap_ = new TGAImage();
		load_texture(filename, "_spec.tga", specularmap_);
	}

	texfile = texfile.substr(0, dot) + std::string("_roughness.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		roughnessmap_ = new TGAImage();
		load_texture(filename, "_roughness.tga", roughnessmap_);
	}

	texfile = texfile.substr(0, dot) + std::string("_metalness.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		metalnessmap_ = new TGAImage();
		load_texture(filename, "_metalness.tga", metalnessmap_);
	}

	texfile = texfile.substr(0, dot) + std::string("_emission.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		emision_map = new TGAImage();
		load_texture(filename, "_emission.tga", emision_map);
	}

	texfile = texfile.substr(0, dot) + std::string("_occlusion.tga");
	if (_access(texfile.data(), 0) != -1)
	{
		occlusion_map = new TGAImage();
		load_texture(filename, "_occlusion.tga", occlusion_map);
	}
	0;
}

Vec3 Model::diffuse(Vec2 uv)
{
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * diffusemap_->get_width();
	int uv1 = uv[1] * diffusemap_->get_height();
	TGAColor c = diffusemap_->get(uv0, uv1);
	Vec3 res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f;
	return res;
}

Vec3 Model::normal(Vec2 uv) {
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * normalmap_->get_width();
	int uv1 = uv[1] * normalmap_->get_height();
	TGAColor c = normalmap_->get(uv0, uv1);
	Vec3 res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f; //because the normap_map coordinate is -1 ~ +1
	return res;
}

float Model::roughness(Vec2 uv) {
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * roughnessmap_->get_width();
	int uv1 = uv[1] * roughnessmap_->get_height();
	return roughnessmap_->get(uv0, uv1)[0] / 255.f;
}

float Model::metalness(Vec2 uv) {
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * metalnessmap_->get_width();
	int uv1 = uv[1] * metalnessmap_->get_height();
	return metalnessmap_->get(uv0, uv1)[0] / 255.f;
}

float Model::specular(Vec2 uv) {
	int uv0 = uv[0] * specularmap_->get_width();
	int uv1 = uv[1] * specularmap_->get_height();
	return specularmap_->get(uv0, uv1)[0] / 1.f;
}

float Model::occlusion(Vec2 uv) {
	if (!occlusion_map)
		return 1;
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * occlusion_map->get_width();
	int uv1 = uv[1] * occlusion_map->get_height();
	return occlusion_map->get(uv0, uv1)[0] / 255.f;
}

Vec3 Model::emission(Vec2 uv)
{
	if (!occlusion_map)
		return Vec3(0.0f, 0.0f, 0.0f);
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * emision_map->get_width();
	int uv1 = uv[1] * emision_map->get_height();
	TGAColor c = emision_map->get(uv0, uv1);
	Vec3 res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f;
	return res;
}
