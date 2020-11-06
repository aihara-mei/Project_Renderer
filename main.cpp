#include <iostream>
#include <limits>
#include <memory>
#include "model.h"
#include "tgaimage.h"
#include "shader.h"

// viewport size
const int width = 800;
const int height = 800;

int main(int argc, char** argv) {
	std::shared_ptr<Model> model = std::make_shared<Model>("./obj/cube.obj");
	TGAImage image(width, height, TGAImage::RGB);

	Shader shader;
	shader.shade(image, model.get());
	return 0;
}