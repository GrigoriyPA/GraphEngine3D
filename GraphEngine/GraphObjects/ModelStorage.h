#pragma once

#include "../CommonClasses/Matrix.h"
#include "../GraphicClasses/GraphicFunctions.h"


namespace eng {
	class ModelStorage {
		friend class GraphObject;

		GLuint matrix_buffer_ = 0;

		size_t max_count_models_;
		std::vector<size_t> models_index_;
		std::vector<size_t> free_model_id_;
		std::vector<std::pair<size_t, Matrix>> models_;

		GLuint create_matrix_buffer(size_t max_count_models) {
			max_count_models_ = max_count_models;

			glGenBuffers(1, &matrix_buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * max_count_models, NULL, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return matrix_buffer_;
		}

		void update_matrix(size_t memory_id) const {
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * memory_id, sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[memory_id].second)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void deallocate() {
			glDeleteBuffers(1, &matrix_buffer_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			matrix_buffer_ = 0;
		}

		void swap(ModelStorage& other) noexcept {
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(max_count_models_, other.max_count_models_);
			std::swap(models_index_, other.models_index_);
			std::swap(free_model_id_, other.free_model_id_);
			std::swap(models_, other.models_);
		}

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

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		ModelStorage(ModelStorage&& other) noexcept {
			swap(other);
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Matrix>>::const_iterator;

		ModelStorage& operator=(const ModelStorage& other)& {
			ModelStorage object(other);
			swap(object);
			return *this;
		}

		ModelStorage& operator=(ModelStorage&& other)& {
			deallocate();
			swap(other);
			return *this;
		}

		const Matrix& operator[](size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid model id.\n\n");
			}

			return models_[models_index_[id]].second;
		}

		ModelStorage& set(size_t id, const Matrix& matrix) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "set, invalid model id.\n\n");
			}

			models_[models_index_[id]].second = matrix;

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_index_[id], sizeof(GLfloat) * 16, &std::vector<GLfloat>(matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		Matrix get(size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get, invalid model id.\n\n");
			}

			return models_[models_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid model id.\n\n");
			}

			return models_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			if (models_.size() <= memory_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}

			return models_[memory_id].first;
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

		Iterator begin() const noexcept {
			return models_.begin();
		}

		Iterator end() const noexcept {
			return models_.end();
		}

		ModelStorage& erase(size_t id) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "erase, invalid model id.\n\n");
			}

			free_model_id_.push_back(id);

			models_index_[models_.back().first] = models_index_[id];
			models_[models_index_[id]] = models_.back();

			update_matrix(models_index_[id]);

			models_.pop_back();
			models_index_[id] = std::numeric_limits<size_t>::max();
			return *this;
		}

		ModelStorage& clear() noexcept {
			models_index_.clear();
			free_model_id_.clear();
			models_.clear();
			return *this;
		}

		size_t insert(const Matrix& matrix) {
			if (models_.size() == max_count_models_) {
				throw EngRuntimeError(__FILE__, __LINE__, "insert, too many instances created.\n\n");
			}

			size_t free_model_id = models_index_.size();
			if (free_model_id_.empty()) {
				models_index_.push_back(models_.size());
			} else {
				free_model_id = free_model_id_.back();
				free_model_id_.pop_back();
				models_index_[free_model_id] = models_.size();
			}

			models_.push_back({ free_model_id, matrix });
			update_matrix(models_.size() - 1);
			return free_model_id;
		}

		ModelStorage& change_left(size_t id, const Matrix& matrix) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "change_left, invalid model id.\n\n");
			}

			models_[models_index_[id]].second = matrix * models_[models_index_[id]].second;
			update_matrix(models_index_[id]);
			return *this;
		}

		ModelStorage& change_right(size_t id, const Matrix& matrix) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "change_left, invalid model id.\n\n");
			}

			models_[models_index_[id]].second *= matrix;
			update_matrix(models_index_[id]);
			return *this;
		}

		~ModelStorage() {
			deallocate();
		}
	};
}
