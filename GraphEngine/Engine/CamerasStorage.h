#pragma once

#include "Camera.h"


namespace gre {
	struct ObjectDesc {
		bool exist = false;
		size_t object_id = 0;
		size_t model_id = 0;
	};

	class CamerasStorage {
		friend class GraphEngine;

		GLuint shader_storage_buffer_ = 0;

		bool is_actual_ = false;
		GLint* intersect_id_ = nullptr;
		GLfloat* intersect_dist_ = nullptr;

		GLint* init_int_ = nullptr;
		GLfloat* init_float_ = nullptr;

		Shader<size_t>* shader_;
		size_t max_count_cameras_;
		std::vector<size_t> cameras_index_;
		std::vector<size_t> free_camera_id_;
		std::vector<std::pair<size_t, Camera>> cameras_;

		CamerasStorage() noexcept {
			shader_ = nullptr;
			max_count_cameras_ = 0;
		}

		CamerasStorage(const CamerasStorage& other) {
			cameras_index_ = other.cameras_index_;
			free_camera_id_ = other.free_camera_id_;
			cameras_ = other.cameras_;

			if (other.max_count_cameras_ > 0) {
				intersect_id_ = new GLint[2 * other.max_count_cameras_];
				intersect_dist_ = new GLfloat[other.max_count_cameras_];
				init_int_ = new GLint[2 * other.max_count_cameras_];
				init_float_ = new GLfloat[other.max_count_cameras_];
				for (size_t i = 0; i < other.max_count_cameras_; ++i) {
					init_int_[2 * i] = -1;
					init_int_[2 * i + 1] = -1;
					init_float_[i] = 1;
				}
			}

			create_shader_storage_buffer(other.max_count_cameras_, *other.shader_);

			glBindBuffer(GL_COPY_READ_BUFFER, other.shader_storage_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, shader_storage_buffer_);

			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (2 * sizeof(GLint) + sizeof(GLfloat)) * max_count_cameras_);

			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
			glBindBuffer(GL_COPY_READ_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		CamerasStorage(CamerasStorage&& other) noexcept {
			swap(other);
		}

		CamerasStorage& operator=(const CamerasStorage& other)& {
			CamerasStorage object(other);
			swap(object);
			return *this;
		}

		CamerasStorage& operator=(CamerasStorage&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		ObjectDesc get_check_object(size_t id, Vec3& intersect_point) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_check_object, invalid camera id.\n\n");
			}

			load_buffer_data();

			const Camera& camera = cameras_[cameras_index_[id]].second;
			intersect_point = camera.convert_point(intersect_dist_[cameras_index_[id]]);

			GLint object_id = intersect_id_[cameras_index_[id]];
			GLint model_id = intersect_id_[cameras_index_[id] + max_count_cameras_];
			return { .exist = true, .object_id = static_cast<size_t>(object_id), .model_id = static_cast<size_t>(model_id) };
		}

		ObjectDesc get_check_object(size_t id) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_check_object, invalid camera id.\n\n");
			}

			load_buffer_data();

			GLint object_id = intersect_id_[cameras_index_[id]];
			GLint model_id = intersect_id_[cameras_index_[id] + max_count_cameras_];
			return { .exist = true, .object_id = static_cast<size_t>(object_id), .model_id = static_cast<size_t>(model_id) };
		}

		void create_shader_storage_buffer(size_t max_count_cameras, Shader<size_t>& shader) {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "create_shader_storage_buffer, invalid shader type.\n\n");
			}

			shader_ = &shader;
			max_count_cameras_ = max_count_cameras;

			delete[] intersect_id_;
			delete[] intersect_dist_;
			delete[] init_int_;
			delete[] init_float_;
			intersect_id_ = new GLint[2 * max_count_cameras_];
			intersect_dist_ = new GLfloat[max_count_cameras_];
			init_int_ = new GLint[2 * max_count_cameras_];
			init_float_ = new GLfloat[max_count_cameras_];
			for (size_t i = 0; i < max_count_cameras_; ++i) {
				init_int_[2 * i] = -1;
				init_int_[2 * i + 1] = -1;
				init_float_[i] = 1;
			}

			shader.use();
			glGenBuffers(1, &shader_storage_buffer_);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			glBufferData(GL_SHADER_STORAGE_BUFFER, (2 * sizeof(GLint) + sizeof(GLfloat)) * max_count_cameras_, init_int_, GL_DYNAMIC_READ);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, init_float_);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void load_buffer_data() {
			if (is_actual_) {
				return;
			}
			is_actual_ = true;

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint) * max_count_cameras_, intersect_id_);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, intersect_dist_);
			
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void update_storage() {
			is_actual_ = false;

			shader_->use();
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint) * max_count_cameras_, init_int_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, init_float_);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void swap(CamerasStorage& other) {
			std::swap(shader_storage_buffer_, other.shader_storage_buffer_);
			std::swap(is_actual_, other.is_actual_);
			std::swap(intersect_id_, other.intersect_id_);
			std::swap(intersect_dist_, other.intersect_dist_);
			std::swap(init_int_, other.init_int_);
			std::swap(init_float_, other.init_float_);
			std::swap(shader_, other.shader_);
			std::swap(max_count_cameras_, other.max_count_cameras_);
			cameras_index_.swap(other.cameras_index_);
			free_camera_id_.swap(other.free_camera_id_);
			cameras_.swap(other.cameras_);
		}

		void deallocate() {
			glDeleteBuffers(1, &shader_storage_buffer_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			delete[] intersect_id_;
			delete[] intersect_dist_;
			delete[] init_int_;
			delete[] init_float_;

			shader_storage_buffer_ = 0;
			intersect_id_ = nullptr;
			intersect_dist_ = nullptr;
			init_int_ = nullptr;
			init_float_ = nullptr;
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Camera>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, Camera>>::const_iterator;

		Camera& operator[](size_t id) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid camera id.\n\n");
			}

			return cameras_[cameras_index_[id]].second;
		}

		const Camera& operator[](size_t id) const {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid camera id.\n\n");
			}

			return cameras_[cameras_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid camera id.\n\n");
			}

			return cameras_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			if (cameras_.size() <= memory_id) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}

			return cameras_[memory_id].first;
		}

		size_t get_max_count_cameras() const noexcept {
			return max_count_cameras_;
		}

		bool contains(size_t id) const noexcept {
			return id < cameras_index_.size() && cameras_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < cameras_.size();
		}

		size_t size() const noexcept {
			return cameras_.size();
		}

		bool empty() const noexcept {
			return cameras_.empty();
		}

		Iterator begin() noexcept {
			return cameras_.begin();
		}

		Iterator end() noexcept {
			return cameras_.end();
		}

		ConstIterator begin() const noexcept {
			return cameras_.begin();
		}

		ConstIterator end() const noexcept {
			return cameras_.end();
		}

		CamerasStorage& erase(size_t id) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid camera id.\n\n");
			}

			free_camera_id_.push_back(id);

			cameras_index_[cameras_.back().first] = cameras_index_[id];
			std::swap(cameras_[cameras_index_[id]], cameras_.back());

			cameras_.pop_back();
			cameras_index_[id] = std::numeric_limits<size_t>::max();
			return *this;
		}

		CamerasStorage& clear() noexcept {
			cameras_index_.clear();
			free_camera_id_.clear();
			cameras_.clear();
			return *this;
		}

		size_t insert(const Camera& camera) {
			size_t free_camera_id = cameras_index_.size();
			if (free_camera_id_.empty()) {
				cameras_index_.push_back(cameras_.size());
			} else {
				free_camera_id = free_camera_id_.back();
				free_camera_id_.pop_back();
				cameras_index_[free_camera_id] = cameras_.size();
			}

			cameras_.push_back({ free_camera_id, camera });
			return free_camera_id;
		}

		void switch_active() {
			for (auto& [id, camera] : cameras_) {
				camera.switch_active();
			}
		}

		void compute_event(const sf::Event& event) {
			for (auto& [id, camera] : cameras_) {
				camera.compute_event(event);
			}
		}

		void update() {
			for (auto& [id, camera] : cameras_) {
				camera.update();
			}
		}
	};
}

