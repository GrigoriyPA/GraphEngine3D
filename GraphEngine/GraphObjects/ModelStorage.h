#pragma once

#include "Material.h"


namespace gre {
	class ModelStorage {
		friend class GraphObject;

		GLuint matrix_buffer_ = 0;

		size_t max_count_models_;
		std::vector<size_t> models_index_;
		std::vector<size_t> free_model_id_;
		std::vector<std::pair<size_t, Matrix4x4>> models_;

		ModelStorage() noexcept {
			max_count_models_ = 0;
		}

		ModelStorage(const ModelStorage& other) {
			max_count_models_ = other.max_count_models_;
			models_index_ = other.models_index_;
			free_model_id_ = other.free_model_id_;
			models_ = other.models_;

			create_matrix_buffer(max_count_models_);

			glBindBuffer(GL_COPY_READ_BUFFER, other.matrix_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, matrix_buffer_);

			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLfloat) * 16 * max_count_models_);

			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
			glBindBuffer(GL_COPY_READ_BUFFER, 0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		ModelStorage(ModelStorage&& other) noexcept {
			swap(other);
		}

		ModelStorage& operator=(const ModelStorage& other)& {
			ModelStorage object(other);
			swap(object);
			return *this;
		}

		ModelStorage& operator=(ModelStorage&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		GLuint create_matrix_buffer(size_t max_count_models) {
			max_count_models_ = max_count_models;

			glGenBuffers(1, &matrix_buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * max_count_models, NULL, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
			return matrix_buffer_;
		}

		void update_matrix(size_t memory_id) const {
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * memory_id, sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[memory_id].second)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
		}

		void deallocate() {
			glDeleteBuffers(1, &matrix_buffer_);
#ifdef _DEBUG
			check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG

			matrix_buffer_ = 0;
		}

		void swap(ModelStorage& other) noexcept {
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(max_count_models_, other.max_count_models_);
			models_index_.swap(other.models_index_);
			free_model_id_.swap(other.free_model_id_);
			models_.swap(other.models_);
		}

	public:
		using ConstIterator = std::vector<std::pair<size_t, Matrix4x4>>::const_iterator;

		const Matrix4x4& operator[](size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid model id.\n\n");
			}
#endif // _DEBUG

			return models_[models_index_[id]].second;
		}

		void set(size_t id, const Matrix4x4& matrix) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "set, invalid model id.\n\n");
			}
#endif // _DEBUG

			models_[models_index_[id]].second = matrix;
			update_matrix(models_index_[id]);
		}

		Matrix4x4 get(size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get, invalid model id.\n\n");
			}
#endif // _DEBUG

			return models_[models_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid model id.\n\n");
			}
#endif // _DEBUG

			return models_index_[id];
		}

		size_t get_id(size_t memory_id) const {
#ifdef _DEBUG
			if (models_.size() <= memory_id) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}
#endif // _DEBUG

			return models_[memory_id].first;
		}

		size_t get_max_count_models() const noexcept {
			return max_count_models_;
		}

		bool contains(size_t id) const noexcept {
			return id < models_index_.size() && models_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < models_.size();
		}

		size_t size() const noexcept {
			return models_.size();
		}

		bool empty() const noexcept {
			return models_.empty();
		}

		ConstIterator begin() const noexcept {
			return models_.begin();
		}

		ConstIterator end() const noexcept {
			return models_.end();
		}

		void erase(size_t id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid model id.\n\n");
			}
#endif // _DEBUG

			free_model_id_.push_back(id);

			models_index_[models_.back().first] = models_index_[id];
			models_[models_index_[id]] = models_.back();

			update_matrix(models_index_[id]);

			models_.pop_back();
			models_index_[id] = std::numeric_limits<size_t>::max();
		}

		void clear() noexcept {
			models_index_.clear();
			free_model_id_.clear();
			models_.clear();
		}

		size_t insert(const Matrix4x4& matrix) {
#ifdef _DEBUG
			if (models_.size() == max_count_models_) {
				throw GreRuntimeError(__FILE__, __LINE__, "insert, too many instances created.\n\n");
			}
#endif // _DEBUG

			size_t free_model_id = models_index_.size();
			if (free_model_id_.empty()) {
				models_index_.push_back(models_.size());
			}
			else {
				free_model_id = free_model_id_.back();
				free_model_id_.pop_back();
				models_index_[free_model_id] = models_.size();
			}

			models_.push_back({ free_model_id, matrix });
			update_matrix(models_.size() - 1);
			return free_model_id;
		}

		void change_left(size_t id, const Matrix4x4& matrix) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "change_left, invalid model id.\n\n");
			}
#endif // _DEBUG

			models_[models_index_[id]].second = matrix * models_[models_index_[id]].second;
			update_matrix(models_index_[id]);
		}

		void change_right(size_t id, const Matrix4x4& matrix) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "change_left, invalid model id.\n\n");
			}
#endif // _DEBUG

			models_[models_index_[id]].second *= matrix;
			update_matrix(models_index_[id]);
		}

		~ModelStorage() {
			deallocate();
		}
	};
}
