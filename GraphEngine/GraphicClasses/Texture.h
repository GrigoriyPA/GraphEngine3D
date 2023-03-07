#pragma once

#include "GraphicFunctions.h"


namespace gre {
	class Texture {
		inline static GLint max_texture_image_units_ = 0;

		size_t width_ = 0;
		size_t height_ = 0;
		size_t* count_links_ = nullptr;
		GLuint texture_id_ = 0;

	public:
		Texture() {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Texture, failed to initialize GLEW.\n\n");
			}

			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units_);
		}

		explicit Texture(const sf::Image& image, bool gamma = true) {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Texture, failed to initialize GLEW.\n\n");
			}

			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units_);

			set_image(image, gamma);
		}

		Texture(const Texture& other) noexcept {
			width_ = other.width_;
			height_ = other.height_;
			texture_id_ = other.texture_id_;
			count_links_ = other.count_links_;
			if (count_links_ != nullptr) {
				++(*count_links_);
			}
		}

		Texture(Texture&& other) noexcept {
			swap(other);
		}

		Texture& operator=(const Texture& other)& noexcept {
			Texture object(other);
			swap(object);
			return *this;
		}

		Texture& operator=(Texture&& other)& noexcept {
			clear();
			swap(other);
			return *this;
		}

		bool operator==(const Texture& other) const noexcept {
			return texture_id_ == other.texture_id_;
		}

		bool operator!=(const Texture& other) const noexcept {
			return texture_id_ != other.texture_id_;
		}

		void set_image(const sf::Image& image, bool gamma = true) {
			clear();

			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units_);
			width_ = image.getSize().x;
			height_ = image.getSize().y;
			count_links_ = new size_t(1);

			glGenTextures(1, &texture_id_);
			glBindTexture(GL_TEXTURE_2D, texture_id_);

			if (gamma) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
			}

			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_wrapping(GLint wrapping) const {
#ifdef _DEBUG
			if (wrapping != GL_REPEAT && wrapping != GL_MIRRORED_REPEAT && wrapping != GL_CLAMP_TO_EDGE && wrapping != GL_CLAMP_TO_BORDER) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_wrapping, invalid wrapping type.\n\n");
			}
#endif // _DEBUG

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
			glBindTexture(GL_TEXTURE_2D, 0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		GLuint get_id() const noexcept {
			return texture_id_;
		}

		size_t get_width() const noexcept {
			return width_;
		}

		size_t get_height() const noexcept {
			return height_;
		}

		void activate(GLenum unit_id) const {
#ifdef _DEBUG
			if (max_texture_image_units_ <= static_cast<GLint>(unit_id)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "activate, invalid texture unit id.\n\n");
			}
#endif // _DEBUG

			if (texture_id_ == 0) {
				return;
			}

			glActiveTexture(GL_TEXTURE0 + unit_id);
			glBindTexture(GL_TEXTURE_2D, texture_id_);
			glActiveTexture(GL_TEXTURE0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void deactive(GLenum unit_id) const {
#ifdef _DEBUG
			if (max_texture_image_units_ <= static_cast<GLint>(unit_id)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "deactive, invalid texture unit id.\n\n");
			}
#endif // _DEBUG

			if (texture_id_ == 0) {
				return;
			}

			glActiveTexture(GL_TEXTURE0 + unit_id);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void load_from_file(const std::string& texture_path, bool gamma = true) {
			sf::Image image;
			if (!image.loadFromFile(texture_path)) {
				throw GreRuntimeError(__FILE__, __LINE__, "Texture, texture file loading failed.\n\n");
			}

			set_image(image, gamma);
		}

		void swap(Texture& other) noexcept {
			std::swap(width_, other.width_);
			std::swap(height_, other.height_);
			std::swap(count_links_, other.count_links_);
			std::swap(texture_id_, other.texture_id_);
		}

		void clear() {
			if (count_links_ != nullptr) {
				--(*count_links_);
				if (*count_links_ == 0) {
					delete count_links_;

					glDeleteTextures(1, &texture_id_);
#ifdef _DEBUG
					check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
				}
			}
			count_links_ = nullptr;
			texture_id_ = 0;
		}

		~Texture() {
			clear();
		}
	};
}
