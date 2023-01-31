#pragma once

#include "Material.h"


namespace gre {
	class Mesh {
		inline static const std::vector<GLint> MEMORY_CONFIGURATION = { 3, 3, 2, 3 };

		GLuint vertex_array_ = 0;
		GLuint vertex_buffer_ = 0;
		GLuint index_buffer_ = 0;

		GLfloat border_width_ = 1.0;

		size_t count_points_;
		size_t count_indices_;

		void set_uniforms(const Shader<size_t>& shader) const {
			if (shader.description == ShaderType::MAIN) {
				material.set_uniforms(shader);
			}

			glLineWidth(border_width_);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void delete_uniforms(const Shader<size_t>& shader) const {
			if (shader.description == ShaderType::MAIN) {
				material.delete_uniforms(shader);
			}

			glLineWidth(1.0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_vertex_array() {
			glGenVertexArrays(1, &vertex_array_);
			glBindVertexArray(vertex_array_);

			glGenBuffers(1, &vertex_buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

			size_t memory_size = 0;
			for (size_t element : MEMORY_CONFIGURATION)
				memory_size += element;
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * memory_size * count_points_, NULL, GL_STATIC_DRAW);

			memory_size = 0;
			for (GLuint i = 0; i < MEMORY_CONFIGURATION.size(); memory_size += MEMORY_CONFIGURATION[i], ++i) {
				glVertexAttribPointer(i, MEMORY_CONFIGURATION[i], GL_FLOAT, GL_FALSE, sizeof(GLfloat) * MEMORY_CONFIGURATION[i], reinterpret_cast<GLvoid*>(sizeof(GLfloat) * memory_size * count_points_));
				glEnableVertexAttribArray(i);
			}

			glGenBuffers(1, &index_buffer_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void deallocate() {
			glDeleteVertexArrays(1, &vertex_array_);
			glDeleteBuffers(1, &vertex_buffer_);
			glDeleteBuffers(1, &index_buffer_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			vertex_array_ = 0;
			vertex_buffer_ = 0;
			index_buffer_ = 0;
		}

	public:
		bool frame = false;

		Material material;

		Mesh() {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Mesh, failed to initialize GLEW.\n\n");
			}

			count_points_ = 0;
			count_indices_ = 0;
		}

		// Default polygon shape
		explicit Mesh(size_t count_points) {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "Mesh, failed to initialize GLEW.\n\n");
			}
			if (count_points < 2) {
				throw GreInvalidArgument(__FILE__, __LINE__, "Mesh, invalid number of points.\n\n");
			}

			count_points_ = count_points;
			count_indices_ = (count_points - 2) * 3;

			create_vertex_array();

			std::vector<GLuint> indices(count_indices_);
			for (size_t i = 0; i < count_points - 2; ++i) {
				indices[3 * i] = 0;
				indices[3 * i + 1] = static_cast<GLuint>(i + 1);
				indices[3 * i + 2] = static_cast<GLuint>(i + 2);
			}
			set_indices(indices);
		}

		Mesh(const Mesh& other) {
			border_width_ = other.border_width_;
			count_points_ = other.count_points_;
			count_indices_ = other.count_indices_;
			frame = other.frame;
			material = other.material;

			create_vertex_array();

			glBindVertexArray(vertex_array_);
			glBindBuffer(GL_COPY_READ_BUFFER, other.vertex_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, vertex_buffer_);

			size_t memory_size = 0;
			for (size_t element : MEMORY_CONFIGURATION)
				memory_size += element; 
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLfloat) * memory_size * count_points_);

			glBindBuffer(GL_COPY_READ_BUFFER, other.index_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, index_buffer_);

			glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLuint) * count_indices_, NULL, GL_STATIC_DRAW);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLuint) * count_indices_);

			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
			glBindBuffer(GL_COPY_READ_BUFFER, 0);
			glBindVertexArray(0);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		Mesh(Mesh&& other) noexcept {
			swap(other);
		}

		Mesh& operator=(const Mesh& other)& {
			Mesh object(other);
			swap(object);
			return *this;
		}

		Mesh& operator=(Mesh&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		bool operator==(const Mesh& other) const noexcept {
			return frame == other.frame && material == other.material;
		}

		bool operator!=(const Mesh& other) const noexcept {
			return !(*this == other);
		}

		Mesh& set_border_width(GLfloat border_width) {
			if (border_width < 0.0) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_border_width, invalid border width value.\n\n");
			}

			border_width_ = border_width;
			return *this;
		}

		Mesh& set_positions(const std::vector<Vec3>& positions, bool update_normals = false) {
			if (positions.size() != count_points_) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_positions, invalid number of points.\n\n");
			}

			std::vector<GLfloat> converted_positions(count_points_ * 3);
			for (size_t i = 0; i < count_points_; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					converted_positions[3 * i + j] = static_cast<GLfloat>(positions[i][j]);
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * count_points_, reinterpret_cast<const GLvoid*>(&converted_positions[0]));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			if (update_normals) {
				if (positions.size() < 3) {
					throw GreInvalidArgument(__FILE__, __LINE__, "set_positions, invalid number of points for automatic calculation of normals.\n\n");
				}

				set_normals(std::vector<Vec3>(count_points_, (positions[2] - positions[0]) ^ (positions[1] - positions[0])));
			}
			return *this;
		}

		Mesh& set_normals(const std::vector<Vec3>& normals) {
			if (normals.size() != count_points_) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_normals, invalid number of points.\n\n");
			}

			std::vector<GLfloat> converted_normals(count_points_ * 3);
			for (size_t i = 0; i < count_points_; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					converted_normals[3 * i + j] = static_cast<GLfloat>(normals[i][j]);
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * count_points_, sizeof(GLfloat) * 3 * count_points_, reinterpret_cast<const GLvoid*>(&converted_normals[0]));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		Mesh& set_tex_coords(const std::vector<Vec2>& tex_coords) {
			if (tex_coords.size() != count_points_) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_tex_coords, invalid number of points.\n\n");
			}

			std::vector<GLfloat> converted_tex_coords(count_points_ * 2);
			for (size_t i = 0; i < count_points_; ++i) {
				for (size_t j = 0; j < 2; ++j) {
					converted_tex_coords[2 * i + j] = static_cast<GLfloat>(tex_coords[i][j]);
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * count_points_, sizeof(GLfloat) * 2 * count_points_, reinterpret_cast<const GLvoid*>(&converted_tex_coords[0]));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		Mesh& set_colors(const std::vector<Vec3>& colors) {
			if (colors.size() != count_points_) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_colors, invalid number of points.\n\n");
			}

			std::vector<GLfloat> converted_colors(count_points_ * 3);
			for (size_t i = 0; i < count_points_; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					converted_colors[3 * i + j] = static_cast<GLfloat>(colors[i][j]);
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * count_points_, sizeof(GLfloat) * 3 * count_points_, reinterpret_cast<const GLvoid*>(&converted_colors[0]));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		Mesh& set_indices(const std::vector<GLuint>& indices) {
			count_indices_ = indices.size();

			glBindVertexArray(vertex_array_);

			glDeleteBuffers(1, &index_buffer_);
			glGenBuffers(1, &index_buffer_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), reinterpret_cast<const GLvoid*>(&indices[0]), GL_STATIC_DRAW);

			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		GLuint get_vertex_array() const noexcept {
			return vertex_array_;
		}

		size_t get_count_points() const noexcept {
			return count_points_;
		}

		size_t get_count_indices() const noexcept {
			return count_indices_;
		}

		std::vector<Vec3> get_positions() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vec3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<Vec3> get_normals() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * count_points_, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vec3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<Vec2> get_tex_coords() const {
			GLfloat* buffer = new GLfloat[2 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * count_points_, sizeof(GLfloat) * 2 * count_points_, reinterpret_cast<GLvoid*>(buffer));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			std::vector<Vec2> result;
			result.reserve(count_points_);
			for (size_t i = 0; i < count_points_; ++i) {
				result.emplace_back(static_cast<double>(buffer[2 * i]), static_cast<double>(buffer[2 * i + 1]));
			}
			return result;
		}

		template <typename T>  // Casts required: double(T)
		static std::vector<Vec2> move_in(size_t size, T* data) {
			std::vector<Vec2> result(size);
			for (size_t i = 0; i < size; ++i) {
				result[i] = Vec2(static_cast<double>(data[2 * i]), static_cast<double>(data[2 * i + 1]));
			}

			delete[] data;
			return result;
		}

		std::vector<Vec3> get_colors() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * count_points_, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vec3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<GLuint> get_indices() const {
			GLuint* buffer = new GLuint[count_indices_];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
			glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * count_indices_, reinterpret_cast<GLvoid*>(buffer));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			std::vector<GLuint> result(count_indices_);
			for (size_t i = 0; i < count_indices_; ++i) {
				result[i] = buffer[i];
			}
			delete[] buffer;

			return result;
		}

		Vec3 get_center() const {
			std::vector<Vec3> positions = get_positions();
			Vec3 center(0.0);
			for (const Vec3& position : get_positions()) {
				center += position;
			}

			return center / static_cast<double>(positions.size());
		}

		void swap(Mesh& other) noexcept {
			std::swap(vertex_array_, other.vertex_array_);
			std::swap(vertex_buffer_, other.vertex_buffer_);
			std::swap(index_buffer_, other.index_buffer_);
			std::swap(border_width_, other.border_width_);
			std::swap(count_points_, other.count_points_);
			std::swap(count_indices_, other.count_indices_);
			std::swap(frame, other.frame);
			std::swap(material, other.material);
		}

		Mesh& apply_matrix(const Matrix& transform) {
			Matrix normal_transform = Matrix::normal_transform(transform);
			std::vector<Vec3> positions = get_positions();
			std::vector<Vec3> normals = get_normals();
			for (size_t i = 0; i < count_points_; ++i) {
				positions[i] = transform * positions[i];
				normals[i] = normal_transform * normals[i];
			}
			set_positions(positions);
			set_normals(normals);
			return *this;
		}

		Mesh& invert_points_order(bool update_normals = false) {
			std::vector<Vec3> positions = get_positions();
			std::reverse(positions.begin(), positions.end());
			set_positions(positions, update_normals);
			return *this;
		}

		void draw(size_t count, const Shader<size_t>& shader) const {
			if (count == 0) {
				return;
			}

			set_uniforms(shader);

			glBindVertexArray(vertex_array_);
			if (!frame) {
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(count_indices_), GL_UNSIGNED_INT, NULL, static_cast<GLsizei>(count));
			} else {
				glDrawElementsInstanced(GL_LINE_LOOP, static_cast<GLsizei>(count_indices_), GL_UNSIGNED_INT, NULL, static_cast<GLsizei>(count));
			}
			glBindVertexArray(0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			delete_uniforms(shader);
		}

		~Mesh() {
			deallocate();
		}

		static size_t get_count_params() noexcept {
			return MEMORY_CONFIGURATION.size();
		}
	};
}
