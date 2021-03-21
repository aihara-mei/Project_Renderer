#include "sample.h"

Vec3 texture_sample(Vec2 uv, TGAImage* image)
{
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);
	int uv0 = uv[0] * image->get_width();
	int uv1 = uv[1] * image->get_height();
	TGAColor c = image->get(uv0, uv1);
	Vec3 res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f;
	return res;
}

static int cal_cubemap_uv(Vec3 direction, Vec2& uv)
{
	int face_index = -1;
	float ma = 0, sc = 0, tc = 0;
	float abs_x = fabs(direction[0]), abs_y = fabs(direction[1]), abs_z = fabs(direction[2]);

	if (abs_x > abs_y && abs_x > abs_z)			/* major axis -> x */
	{
		ma = abs_x;
		if (direction.x > 0)					/* positive x */
		{
			face_index = 0;
			sc = +direction.z;
			tc = +direction.y;
		}
		else									/* negative x */
		{
			face_index = 1;
			sc = -direction.z;
			tc = +direction.y;
		}
	}
	else if (abs_y > abs_z)						/* major axis -> y */
	{
		ma = abs_y;
		if (direction.y > 0)					/* positive y */
		{
			face_index = 2;
			sc = +direction.x;
			tc = +direction.z;
		}
		else									/* negative y */
		{
			face_index = 3;
			sc = +direction.x;
			tc = -direction.z;
		}
	}
	else										/* major axis -> z */
	{
		ma = abs_z;
		if (direction.z > 0)					/* positive z */
		{
			face_index = 4;
			sc = -direction.x;
			tc = +direction.y;
		}
		else									/* negative z */
		{
			face_index = 5;
			sc = +direction.x;
			tc = +direction.y;
		}
	}

	uv[0] = (sc / ma + 1.0f) / 2.0f;
	uv[1] = (tc / ma + 1.0f) / 2.0f;

	return face_index;
}

Vec3 cubemap_sampling(Vec3 direction, cubemap_t* cubemap)
{
	Vec2 uv;
	Vec3 color;
	int face_index = cal_cubemap_uv(direction, uv);
	color = texture_sample(uv, cubemap->faces[face_index]);

	return color;
}