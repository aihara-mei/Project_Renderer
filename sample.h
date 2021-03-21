#pragma once
#include "tgaimage.h"
#include "matrix.h"
#include "graphic.h"

Vec3 texture_sample(Vec2 uv, TGAImage* image);

Vec3 cubemap_sampling(Vec3 direction, cubemap_t* cubemap);