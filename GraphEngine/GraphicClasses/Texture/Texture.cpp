#include "Texture.hpp"


// Texture
namespace gre {
	Texture::Texture() {
		GRE_ENSURE(glew_is_ok(), GreRuntimeError, "failed to initialize GLEW");

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units_);
	}

	Texture::Texture(const sf::Image& image, bool gamma) {
		GRE_ENSURE(glew_is_ok(), GreRuntimeError, "failed to initialize GLEW");

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units_);

		set_image(image, gamma);
	}

	Texture::Texture(const Texture& other) noexcept {
		width_ = other.width_;
		height_ = other.height_;
		texture_id_ = other.texture_id_;
		count_links_ = other.count_links_;
		if (count_links_ != nullptr) {
			++(*count_links_);
		}
	}

	Texture::Texture(Texture&& other) noexcept {
		swap(other);
	}

	Texture& Texture::operator=(const Texture& other)& noexcept {
		Texture object(other);
		swap(object);
		return *this;
	}

	Texture& Texture::operator=(Texture&& other)& noexcept {
		clear();
		swap(other);
		return *this;
	}

	bool Texture::operator==(const Texture& other) const noexcept {
		return texture_id_ == other.texture_id_;
	}

	bool Texture::operator!=(const Texture& other) const noexcept {
		return texture_id_ != other.texture_id_;
	}

	void Texture::set_image(const sf::Image& image, bool gamma) {
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

		GRE_CHECK_GL_ERRORS;
	}

	void Texture::set_wrapping(GLint wrapping) const {
		GRE_ENSURE(wrapping == GL_REPEAT || wrapping == GL_MIRRORED_REPEAT || wrapping == GL_CLAMP_TO_EDGE || wrapping == GL_CLAMP_TO_BORDER, GreInvalidArgument, "invalid wrapping type");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
		glBindTexture(GL_TEXTURE_2D, 0);

		GRE_CHECK_GL_ERRORS;
	}

	GLuint Texture::get_id() const noexcept {
		return texture_id_;
	}

	size_t Texture::get_width() const noexcept {
		return width_;
	}

	size_t Texture::get_height() const noexcept {
		return height_;
	}

	void Texture::activate(GLenum unit_id) const {
		GRE_ENSURE(static_cast<GLint>(unit_id) < max_texture_image_units_, GreInvalidArgument, "invalid texture unit id");

		if (texture_id_ == 0) {
			return;
		}

		glActiveTexture(GL_TEXTURE0 + unit_id);
		glBindTexture(GL_TEXTURE_2D, texture_id_);
		glActiveTexture(GL_TEXTURE0);

		GRE_CHECK_GL_ERRORS;
	}

	void Texture::deactive(GLenum unit_id) const {
		GRE_ENSURE(static_cast<GLint>(unit_id) < max_texture_image_units_, GreInvalidArgument, "invalid texture unit id");

		if (texture_id_ == 0) {
			return;
		}

		glActiveTexture(GL_TEXTURE0 + unit_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);

		GRE_CHECK_GL_ERRORS;
	}

	void Texture::load_from_file(const std::string& texture_path, bool gamma) {
		sf::Image image;
		GRE_ENSURE(image.loadFromFile(texture_path), GreRuntimeError, "texture file loading failed");

		set_image(image, gamma);
	}

	void Texture::load_from_memory(const void* data, size_t size, bool gamma) {
		sf::Image image;
		GRE_ENSURE(image.loadFromMemory(data, size), GreRuntimeError, "texture loading from memory failed");

		set_image(image, gamma);
	}

	void Texture::swap(Texture& other) noexcept {
		std::swap(width_, other.width_);
		std::swap(height_, other.height_);
		std::swap(count_links_, other.count_links_);
		std::swap(texture_id_, other.texture_id_);
	}

	void Texture::clear() {
		if (count_links_ != nullptr) {
			--(*count_links_);
			if (*count_links_ == 0) {
				delete count_links_;

				glDeleteTextures(1, &texture_id_);
				GRE_CHECK_GL_ERRORS;
			}
		}
		count_links_ = nullptr;
		texture_id_ = 0;
	}

	Texture::~Texture() {
		clear();
	}
}  // namespace gre
