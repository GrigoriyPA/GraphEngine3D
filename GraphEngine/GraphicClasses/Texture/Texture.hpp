#pragma once

#include "../../Common/common.hpp"


// Proxy class for openGL textures
namespace gre {
	class Texture {
		inline static GLint max_texture_image_units_ = 0;

		size_t width_ = 0;
		size_t height_ = 0;
		size_t* count_links_ = nullptr;
		GLuint texture_id_ = 0;

	public:
		Texture();

		explicit Texture(const sf::Image& image, bool gamma = true);

		Texture(const Texture& other) noexcept;

		Texture(Texture&& other) noexcept;

		Texture& operator=(const Texture& other)& noexcept;

		Texture& operator=(Texture&& other)& noexcept;

		bool operator==(const Texture& other) const noexcept;

		bool operator!=(const Texture& other) const noexcept;

		void set_image(const sf::Image& image, bool gamma = true);

		void set_wrapping(GLint wrapping) const;

		GLuint get_id() const noexcept;

		size_t get_width() const noexcept;

		size_t get_height() const noexcept;

		void activate(GLenum unit_id) const;

		void deactive(GLenum unit_id) const;

		void load_from_file(const std::string& texture_path, bool gamma = true);

		void load_from_memory(const void* data, size_t size, bool gamma = true);

		void swap(Texture& other) noexcept;

		void clear();

		~Texture();
	};
}  // namespace gre
