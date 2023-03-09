#pragma once

#include "../Light/Light.h"


namespace gre {
	class LightStorage {
		friend class GraphEngine;

		GLuint depth_map_frame_buffer_ = 0;
		GLuint depth_map_texture_id_ = 0;

		size_t shadow_width_ = 1024;
		size_t shadow_height_ = 1024;

		size_t max_count_lights_;
		std::vector<size_t> lights_index_;
		std::vector<size_t> free_light_id_;
		std::vector<std::pair<size_t, Light*>> lights_;

		LightStorage() noexcept {
			max_count_lights_ = 0;
		}

		LightStorage(const LightStorage& other) {
			shadow_width_ = other.shadow_width_;
			shadow_height_ = other.shadow_height_;
			max_count_lights_ = other.max_count_lights_;
			lights_index_ = other.lights_index_;
			free_light_id_ = other.free_light_id_;
			lights_ = other.lights_;

			create_depth_map_frame_buffer(max_count_lights_);
		}

		LightStorage(LightStorage&& other) noexcept {
			swap(other);
		}

		LightStorage& operator=(const LightStorage& other)& {
			LightStorage object(other);
			swap(object);
			return *this;
		}

		LightStorage& operator=(LightStorage&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		void set_uniforms(const Shader& shader) const {
			shader.set_uniform_i("number_lights", static_cast<GLint>(lights_.size()));
			for (const auto& [id, light] : lights_) {
				light->set_uniforms(lights_index_[id], shader);
			}
		}

		void set_framebuffer() const {
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer_);
			glViewport(0, 0, static_cast<GLsizei>(shadow_width_), static_cast<GLsizei>(shadow_height_));
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void set_depth_map_texture(size_t id) const {
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, static_cast<GLint>(depth_map_texture_id_), 0, static_cast<GLint>(lights_index_[id]));
			glClear(GL_DEPTH_BUFFER_BIT);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void create_depth_map_frame_buffer(size_t max_count_lights) {
			max_count_lights_ = max_count_lights;

			if (max_count_lights_ == 0) {
				return;
			}

			glGenTextures(1, &depth_map_texture_id_);
			glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_texture_id_);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(shadow_width_), static_cast<GLsizei>(shadow_height_), static_cast<GLsizei>(max_count_lights_), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat border_ñolor[] = { 1.0, 1.0, 1.0, 1.0 };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_ñolor);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			glGenFramebuffers(1, &depth_map_frame_buffer_);
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer_);

			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map_texture_id_, 0, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

#ifdef _DEBUG
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				throw GreRuntimeError(__FILE__, __LINE__, "create_depth_map_frame_buffer, framebuffer is not complete.\n\n");
			}
#endif // _DEBUG

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void swap(LightStorage& other) noexcept {
			std::swap(depth_map_frame_buffer_, other.depth_map_frame_buffer_);
			std::swap(depth_map_texture_id_, other.depth_map_texture_id_);
			std::swap(shadow_width_, other.shadow_width_);
			std::swap(shadow_height_, other.shadow_height_);
			std::swap(max_count_lights_, other.max_count_lights_);
			lights_index_.swap(other.lights_index_);
			free_light_id_.swap(other.free_light_id_);
			lights_.swap(other.lights_);
		}

		void deallocate() {
			glDeleteFramebuffers(1, &depth_map_frame_buffer_);
			glDeleteTextures(1, &depth_map_texture_id_);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			depth_map_frame_buffer_ = 0;
			depth_map_texture_id_ = 0;
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Light*>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, Light*>>::const_iterator;

		Light*& operator[](size_t id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid light id.\n\n");
			}
#endif // _DEBUG

			return lights_[lights_index_[id]].second;
		}

		const Light* const& operator[](size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid light id.\n\n");
			}
#endif // _DEBUG

			return lights_[lights_index_[id]].second;
		}

		void set_shadow_resolution(size_t width, size_t height) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_texture_id_);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(max_count_lights_), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			shadow_width_ = width;
			shadow_height_ = height;
		}

		size_t get_memory_id(size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid light id.\n\n");
			}
#endif // _DEBUG

			return lights_index_[id];
		}

		size_t get_id(size_t memory_id) const {
#ifdef _DEBUG
			if (lights_.size() <= memory_id) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}
#endif // _DEBUG

			return lights_[memory_id].first;
		}

		size_t get_max_count_lights() const noexcept {
			return max_count_lights_;
		}

		Vec2 get_shadow_resolution() const noexcept {
			return Vec2(static_cast<double>(shadow_width_), static_cast<double>(shadow_height_));
		}

		bool contains(size_t id) const noexcept {
			return id < lights_index_.size() && lights_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < lights_.size();
		}

		size_t size() const noexcept {
			return lights_.size();
		}

		bool empty() const noexcept {
			return lights_.empty();
		}

		Iterator begin() noexcept {
			return lights_.begin();
		}

		Iterator end() noexcept {
			return lights_.end();
		}

		ConstIterator begin() const noexcept {
			return lights_.begin();
		}

		ConstIterator end() const noexcept {
			return lights_.end();
		}

		void erase(size_t id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid light id.\n\n");
			}
#endif // _DEBUG

			free_light_id_.push_back(id);

			lights_index_[lights_.back().first] = lights_index_[id];
			std::swap(lights_[lights_index_[id]], lights_.back());

			lights_.pop_back();
			lights_index_[id] = std::numeric_limits<size_t>::max();
		}

		void clear() noexcept {
			lights_index_.clear();
			free_light_id_.clear();
			lights_.clear();
		}

		size_t insert(Light* light) {
#ifdef _DEBUG
			if (lights_.size() == max_count_lights_) {
				throw GreRuntimeError(__FILE__, __LINE__, "insert, too many lights created.\n\n");
			}
#endif // _DEBUG

			size_t free_light_id = lights_index_.size();
			if (free_light_id_.empty()) {
				lights_index_.push_back(lights_.size());
			}
			else {
				free_light_id = free_light_id_.back();
				free_light_id_.pop_back();
				lights_index_[free_light_id] = lights_.size();
			}

			lights_.push_back({ free_light_id, light });
			return free_light_id;
		}
	};
}
