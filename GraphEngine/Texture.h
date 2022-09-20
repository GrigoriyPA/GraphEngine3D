#pragma once

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>


class Texture {
	bool gamma;
	sf::Image image;

public:
	unsigned int texture_id;

	Texture() {
		texture_id = 0;
	}

	Texture(std::string texture_path, bool gamma = true) {
		this->gamma = gamma;
		if (!image.loadFromFile(texture_path)) {
			std::cout << "ERROR::TEXTURE::BUILDER\n" << "Texture file loading failed.\n";
			assert(0);
		}

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		if (gamma)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture set_wrapping(int wrapping) {
		if (wrapping != GL_REPEAT && wrapping != GL_MIRRORED_REPEAT && wrapping != GL_CLAMP_TO_EDGE && wrapping != GL_CLAMP_TO_BORDER) {
			std::cout << "ERROR::TEXTURE::SET_WRAPPING\n" << "Invalid wrapping type.\n";
			assert(0);
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
		glBindTexture(GL_TEXTURE_2D, 0);

		return *this;
	}

	int get_min_alpha() {
		int res = 255;
		for (int i = 0; i < image.getSize().x; i++) {
			for (int j = 0; j < image.getSize().y; j++)
				res = std::min((int)image.getPixel(i, j).a, res);
		}

		return res;
	}

	void active(int id) {
		if (!texture_id)
			return;

		if (id < 0 || 32 <= id) {
			std::cout << "ERROR::TEXTURE::ACTIVE\n" << "Invalid texture block id.\n";
			assert(0);
		}

		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glActiveTexture(GL_TEXTURE0);
	}

	void deactive(int id) {
		if (!texture_id)
			return;

		if (id < 0 || 32 <= id) {
			std::cout << "ERROR::TEXTURE::DEACTIVE\n" << "Invalid texture block id.\n";
			assert(0);
		}

		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
};
