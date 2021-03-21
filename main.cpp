#include "tgaimage.h"
#include "graphic.h"
#include "model.h"
#include "matrix.h"
#include "sample.h"

Model* model = nullptr;
float* shadowbuffer = nullptr;

const int width = 800;
const int height = 800;

const Vec3 light_dir(0, 0, 1); // reverse direction in z
const Vec3 light_pos(-4, 4, 4);
const Vec3 light_intensity(2, 2, 2);

const Vec3 direction(0.25, 0, 1);
const Vec3 eye_pos(1.1, -0.1, 4);
const Vec3 up(0, 1, 0);

// camera frustum
static const View_frustum frust{ -1, -30, -0.3, -0.3, 0.3, 0.3 }; //view in -z
// light frustum
static const View_frustum light_frust{ -1, -30, -8, -8, 8, 8 }; //view in -z

static float float_aces(float value)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	value = (value * (a * value + b)) / (value * (c * value + d) + e);
	return float_clamp(value, 0, 1);
}

static float GGX_distribution(float n_dot_h, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;

	float n_dot_h_2 = n_dot_h * n_dot_h;
	float factor = n_dot_h_2 * (alpha2 - 1) + 1;
	return alpha2 / (PI * factor * factor);
}

static float SchlickGGX_geometry(float n_dot_v, float roughness)
{
	float r = (1 + roughness);
	float k = r * r / 8.0;

	return n_dot_v / (n_dot_v * (1 - k) + k);
}

static float SchlickGGX_geometry_ibl(float n_dot_v, float roughness)
{
	float k = roughness * roughness / 2.0;

	return n_dot_v / (n_dot_v * (1 - k) + k);
}

static float geometry_Smith(float n_dot_v, float n_dot_l, float roughness)
{
	float g1 = SchlickGGX_geometry(n_dot_v, roughness);
	float g2 = SchlickGGX_geometry(n_dot_l, roughness);

	return g1 * g2;
}

static Vec3 fresenlschlick(float h_dot_v, Vec3& f0)
{
	return f0 + (Vec3(1.0, 1.0, 1.0) - f0) * pow(1 - h_dot_v, 5.0);
}

Vec3 fresenlschlick_roughness(float h_dot_v, Vec3& f0, float roughness)
{
	float r1 = 1.0f - roughness;
	if (r1 < f0[0])
		r1 = f0[0];
	return f0 + (Vec3(r1, r1, r1) - f0) * pow(1 - h_dot_v, 5.0f);
}

static Vec3 Reinhard_mapping(Vec3& color)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		color[i] = float_aces(color[i]);
		//color[i] = color[i] / (color[i] + 0.5);
		color[i] = pow(color[i], 1.0 / 2.2);
	}
	return color;
}

static Vec3 cal_normal(Vec3& normal, Vec3* world_coords, const Vec2* uvs, const Vec2& uv, TGAImage* normal_map)
{
	//calculate the difference in UV coordinate
	float x1 = uvs[1][0] - uvs[0][0];
	float y1 = uvs[1][1] - uvs[0][1];
	float x2 = uvs[2][0] - uvs[0][0];
	float y2 = uvs[2][1] - uvs[0][1];
	float det = (x1 * y2 - x2 * y1);

	//calculate the difference in world pos
	Vec3 e1 = world_coords[1] - world_coords[0];
	Vec3 e2 = world_coords[2] - world_coords[0];

	Vec3 t = e1 * y2 + e2 * (-y1);
	Vec3 b = e1 * (-x2) + e2 * x1;
	t = t / det;
	b = b / det;

	//Schmidt orthogonalization
	normal = normalize(normal);
	t = normalize(t - normal * dot(t, normal));
	b = normalize(b - normal * dot(b, normal) - t * dot(b, t));

	Vec3 sample = texture_sample(uv, normal_map);
	//modify the range 0 ~ 1 to -1 ~ +1
	sample = Vec3(sample[0] * 2 - 1, sample[1] * 2 - 1, sample[2] * 2 - 1);

	Vec3 normal_new = t * sample[0] + b * sample[1] + normal * sample[2];
	return normal_new;
}

struct Shader : public IShader {
	Matrix MVP_Shadow;

	virtual Vec4 vertex(int iface, int nthvert)
	{
		payload.in_world[nthvert] = model->getVert(iface, nthvert);
		payload.in_normal[nthvert] = model->getVert(iface, nthvert);
		payload.in_clip[nthvert] = mtov4(MVP * vtom(payload.in_world[nthvert]));
		payload.in_uv[nthvert] = model->getUV(iface, nthvert);
		return payload.in_clip[nthvert];
	}

	bool direct_fragment(Vec3 bar, Vec2 _uv, TGAColor& color)
	{
		Vec3 CookTorrance_brdf;
		Vec3 light_pos = Vec3(2, 1.5, 5);
		Vec3 radiance = Vec3(3, 3, 3);

		//for reading easily
		Vec4* clip = payload.clip;
		Vec3* world = payload.world;
		Vec3* normals = payload.normal;
		Vec2* uvs = payload.uv;

		float alpha = bar.x, beta = bar.y, gamma = bar.z;

		//interpolate attribute
		float Z = 1.0 / (alpha / clip[0].w + beta / clip[1].w + gamma / clip[2].w);
		Vec3 normal = (alpha * normals[0] / clip[0].w + beta * normals[1] / clip[1].w +
			gamma * normals[2] / clip[2].w) * Z;
		Vec2 uv = (alpha * uvs[0] / clip[0].w + beta * uvs[1] / clip[1].w +
			gamma * uvs[2] / clip[2].w) * Z;
		Vec3 worldpos = (alpha * world[0] / clip[0].w + beta * world[1] / clip[1].w +
			gamma * world[2] / clip[2].w) * Z;

		Vec3 l = normalize(light_dir * -1);
		Vec3 n = normalize(normal);
		Vec3 v = normalize(eye_pos - worldpos);
		Vec3 h = normalize(l + v);

		float n_dot_l = float_max(dot(n, l), 0);

		Vec3 c(0, 0, 0);
		if (n_dot_l > 0)
		{
			float n_dot_v = float_max(dot(n, v), 0);
			float n_dot_h = float_max(dot(n, h), 0);
			float h_dot_v = float_max(dot(h, v), 0);

			float roughness = model->roughness(uv);
			float metalness = model->metalness(uv);

			//roughness = 0.2;
			//metalness = 0.8;
			float NDF = GGX_distribution(n_dot_h, roughness);
			float G = geometry_Smith(n_dot_v, n_dot_l, roughness);

			//get albedo
			Vec3 albedo = model->diffuse(uv);
			Vec3 temp = Vec3(0.04, 0.04, 0.04);
			Vec3 f0 = vec3_lerp(temp, albedo, metalness);

			Vec3 F = fresenlschlick(h_dot_v, f0);
			Vec3 kD = (Vec3(1.0, 1.0, 1.0) - F) * (1 - metalness);

			CookTorrance_brdf = NDF * G * F / (4.0 * n_dot_l * n_dot_v + 0.0001);

			Vec3 Lo = (kD * albedo / PI + CookTorrance_brdf) * radiance * n_dot_l;
			Vec3 ambient = 0.05 * albedo;
			c = Lo + ambient;
			Reinhard_mapping(c);
		}

		c = c * 255;
		color = TGAColor(c.x, c.y, c.z);
		return false;
	}

	virtual bool fragment(Vec3 bar, Vec2 _uv, TGAColor& color) {
		Vec3 CookTorrance_brdf;
		Vec3 radiance = Vec3(3, 3, 3);

		//for reading easily
		Vec4* clip = payload.clip;
		Vec3* world = payload.world;
		Vec3* normals = payload.normal;
		Vec2* uvs = payload.uv;

		float alpha = bar.x, beta = bar.y, gamma = bar.z;

		//interpolate attribute
		float Z = 1.0 / (alpha / clip[0].w + beta / clip[1].w + gamma / clip[2].w);
		Vec3 normal = (alpha * normals[0] / clip[0].w + beta * normals[1] / clip[1].w +
			gamma * normals[2] / clip[2].w) * Z;
		Vec2 uv = (alpha * uvs[0] / clip[0].w + beta * uvs[1] / clip[1].w +
			gamma * uvs[2] / clip[2].w) * Z;
		Vec3 worldpos = (alpha * world[0] / clip[0].w + beta * world[1] / clip[1].w +
			gamma * world[2] / clip[2].w) * Z;


		if (model->normalmap_)
		{
			normal = cal_normal(normal, world, uvs, uv, model->normalmap_);
		}

		Vec3 n = normalize(normal);
		Vec3 v = normalize(eye_pos - worldpos);
		float n_dot_v = float_max(dot(n, v), 0.1);

		Vec3 c(0.0f, 0.0f, 0.0f);
		if (n_dot_v > 0)
		{
			float roughness = model->roughness(uv);
			float metalness = model->metalness(uv);
			float occlusion = model->occlusion(uv);
			Vec3 emission = model->emission(uv);

			//get albedo
			Vec3 albedo = model->diffuse(uv);
			Vec3 temp = Vec3(0.04, 0.04, 0.04);
			Vec3 temp2 = Vec3(1.0f, 1.0f, 1.0f);
			Vec3 f0 = vec3_lerp(temp, albedo, metalness);

			Vec3 F = fresenlschlick_roughness(n_dot_v, f0, roughness);
			Vec3 kD = (Vec3(1.0, 1.0, 1.0) - F) * (1 - metalness);

			//diffuse color
			cubemap_t* irradiance_map = payload.iblmap->irradiance_map;
			Vec3 irradiance = cubemap_sampling(n, irradiance_map);
			for (int i = 0; i < 3; i++)
				irradiance[i] = pow(irradiance[i], 2.0f);
			Vec3 diffuse = irradiance * kD * albedo;

			//specular color
			Vec3 r = normalize(2.0 * dot(v, n) * n - v);
			Vec2 lut_uv = Vec2(n_dot_v, roughness);
			Vec3 lut_sample = texture_sample(lut_uv, payload.iblmap->brdf_lut);
			float specular_scale = lut_sample.x;
			float specular_bias = lut_sample.y;
			Vec3 specular = f0 * specular_scale + Vec3(specular_bias, specular_bias, specular_bias);
			float max_mip_level = (float)(payload.iblmap->mip_levels - 1);
			int specular_miplevel = (int)(roughness * max_mip_level + 0.5f);
			Vec3 prefilter_color = cubemap_sampling(r, payload.iblmap->prefilter_maps[specular_miplevel]);
			for (int i = 0; i < 3; i++)
				prefilter_color[i] = pow(prefilter_color[i], 2.0f);
			specular = cwise_product(prefilter_color, specular);

			c = (diffuse + specular) + emission;
		}

		Reinhard_mapping(c);
		c = c * 255;
		color = TGAColor(c.x, c.y, c.z);

		return false;
	}
};

struct BlinPhongShader : public IShader {
	Matrix MVP_Shadow;

	virtual Vec4 vertex(int iface, int nthvert)
	{
		payload.in_world[nthvert] = model->getVert(iface, nthvert);
		payload.in_normal[nthvert] = model->getVert(iface, nthvert);
		payload.in_clip[nthvert] = mtov4(MVP * vtom(payload.in_world[nthvert]));
		payload.in_uv[nthvert] = model->getUV(iface, nthvert);
		return payload.in_clip[nthvert];
	}

	virtual bool fragment(Vec3 bar, Vec2 _uv, TGAColor& color)
	{
		//set light information
		float p = 5;
		Vec3 amb_light_intensity(0.5, 0.5, 0.5);
		light light1{ light_pos, light_intensity };
		//payload information
		Vec4* clip_coords = payload.clip;
		Vec3* world_coords = payload.world;
		Vec3* normals = payload.normal;
		Vec2* uvs = payload.uv;

		float alpha = bar.x, beta = bar.y, gamma = bar.z;
		//interpolate attribute
		float Z = 1.0 / (alpha / clip_coords[0].w + beta / clip_coords[1].w + gamma / clip_coords[2].w);
		Vec3 normal = (alpha * normals[0] / clip_coords[0].w + beta * normals[1] / clip_coords[1].w +
			gamma * normals[2] / clip_coords[2].w) * Z;
		Vec2 uv = (alpha * uvs[0] / clip_coords[0].w + beta * uvs[1] / clip_coords[1].w +
			gamma * uvs[2] / clip_coords[2].w) * Z;
		Vec3 worldpos = (alpha * world_coords[0] / clip_coords[0].w + beta * world_coords[1] / clip_coords[1].w +
			gamma * world_coords[2] / clip_coords[2].w) * Z;

		if (model->normalmap_)
		{
			normal = cal_normal(normal, world_coords, uvs, uv, model->normalmap_);
		}

		Vec3 ka(1, 1, 1);
		Vec3 ks(5, 5, 5);
		Vec3 kd = model->diffuse(uv);

		//calculate shading color
		Vec3 result_color(0, 0, 0);
		Vec3 ambient, diffuse, specular;
		normal = normalize(normal);
		Vec3 l = normalize(light_dir);
		Vec3 v = normalize(eye_pos - worldpos);
		Vec3 h = normalize(l + v);
		/*float r = (light1.pos - worldpos).norm_squared();*/

		// different ambient color just for better visual effect
		ambient = cwise_product(ka, cwise_product(amb_light_intensity, kd));
		diffuse = cwise_product(kd, light1.intensity) * float_max(0, dot(l, normal));
		specular = cwise_product(ks, light1.intensity) * float_max(0, pow(dot(normal, h), p));

		Vec3 light_space_pos = mtov3(MVP_Shadow * vtom(bary_inter(payload.world, bar)));
		float light_space_depth = shadowbuffer[int(light_space_pos.x) * width + int(light_space_pos.y)];
		float shadow = .3 + .7 * (light_space_depth < light_space_pos.z + .015 * (1 - dot(normal, l)));


		result_color = (ambient + diffuse + specular) * 255.f;
		clamp_v3(result_color, 255.f);

		color = TGAColor(result_color.x * shadow, result_color.y * shadow, result_color.z * shadow);
		return false;
	}
};

struct ShadowShader : public IShader {
	Matrix MVP_Shadow;

	virtual Vec4 vertex(int iface, int nthvert)
	{
		payload.in_world[nthvert] = model->getVert(iface, nthvert);
		payload.in_clip[nthvert] = mtov4(MVP * vtom(payload.in_world[nthvert]));
		payload.in_uv[nthvert] = model->getUV(iface, nthvert);
		return payload.in_clip[nthvert];
	}

	virtual bool fragment(Vec3 bar, Vec2 _uv, TGAColor& color)
	{
		Vec3 light_space_pos = mtov3(MVP_Shadow * vtom(bary_inter(payload.world, bar)));
		float light_space_depth = shadowbuffer[int(light_space_pos.x) * width + int(light_space_pos.y)];
		float shadow = .3 + .7 * (light_space_depth < light_space_pos.z + .01);
		Vec3 c = model->diffuse(_uv);
		color = TGAColor(c.x * shadow, c.y * shadow, c.z * shadow);
		return false;
	}
};

struct DepthShader : public IShader {

	virtual Vec4 vertex(int iface, int nthvert) {
		payload.in_world[nthvert] = model->getVert(iface, nthvert);
		payload.in_clip[nthvert] = mtov4(MVP * vtom(payload.in_world[nthvert]));
		return payload.in_clip[nthvert];
	}

	virtual bool fragment(Vec3 bar, Vec2 _uv, TGAColor& color) {
		Vec3 p = bary_inter(payload.world, bar);
		int c = (1 + p.z) / 2 * 255;
		color = TGAColor(c, c, c);
		return false;
	}
};

int main(int argc, char** argv) {

	model = new Model("./obj/helmet/helmet.obj");
	shadowbuffer = new float[(width + 1) * (height + 1)];
	float* zbuffer = new float[(width + 1) * (height + 1)];
	for (int i = 0; i < width * height; i++) {
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	Matrix MV;
	{	// render shadow buffer
		TGAImage depth(width, height, TGAImage::RGB);
		lookat(light_dir, light_pos, up);
		projection_orth(light_frust);
		viewport(width, height);

		DepthShader depthshader;
		depthshader.MVP = Projection * ModelView;
		MV = Viewport * depthshader.MVP;
		depthshader.Viewport = Viewport;

		for (int i = 0; i < model->n_faces(); i++) {
			draw_triangles(depth, shadowbuffer, depthshader, i);
		}
		depth.flip_vertically();
		depth.write_tga_file("depth.tga");
	}

	{	// render image
		TGAImage image(width, height, TGAImage::RGB);
		lookat(direction, eye_pos, up);
		projection(frust);
		viewport(width, height);

		Shader shader;
		load_ibl_map(shader.payload, "./obj/common2");

		shader.MVP = Projection * ModelView;
		shader.Viewport = Viewport;
		shader.MVP_Shadow = MV;

		for (int i = 0; i < model->n_faces(); i++) {
			draw_triangles(image, zbuffer, shader, i);
		}

		image.flip_vertically();
		image.write_tga_file("output.tga");
	}

	delete[] shadowbuffer;
	delete[] zbuffer;
	delete model;
	return 0;
}