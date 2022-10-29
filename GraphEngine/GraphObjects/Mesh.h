#pragma once

#include "../CommonClasses/Vect2.h"
#include "../GraphicClasses/Shader.h"
#include "../GraphicClasses/Texture.h"


namespace eng {
	class Mesh {
		inline static const std::vector<GLint> MEMORY_CONFIGURATION = { 3, 3, 2, 3 };

		inline static double eps_ = 1e-5;

		GLuint vertex_array_ = 0;
		GLuint matrix_buffer_ = 0;
		GLuint vertex_buffer_ = 0;
		GLuint index_buffer_ = 0;
		GLfloat border_width_ = 1.0;

		GLsizei count_points_;
		GLsizei count_indices_;

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

			size_t memory_size = get_value<size_t>(MEMORY_CONFIGURATION.begin(), MEMORY_CONFIGURATION.end(), 0, [](auto element, auto* result) { *result += element; });
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
			matrix_buffer_ = 0;
			vertex_buffer_ = 0;
			index_buffer_ = 0;
		}

		void swap(Mesh& other) noexcept {
			std::swap(vertex_array_, other.vertex_array_);
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(vertex_buffer_, other.vertex_buffer_);
			std::swap(index_buffer_, other.index_buffer_);
			std::swap(border_width_, other.border_width_);
			std::swap(count_points_, other.count_points_);
			std::swap(count_indices_, other.count_indices_);
			std::swap(frame, other.frame);
			std::swap(material, other.material);
		}

	public:
		class Material {
			friend class Mesh;

			double shininess_ = 1.0;
			double alpha_ = 1.0;
			Vect3 ambient_ = Vect3(0.0, 0.0, 0.0);
			Vect3 diffuse_ = Vect3(0.0, 0.0, 0.0);
			Vect3 specular_ = Vect3(0.0, 0.0, 0.0);
			Vect3 emission_ = Vect3(0.0, 0.0, 0.0);

			void set_uniforms(const Shader<size_t>& shader) const {
				if (shader.description != eng::ShaderType::MAIN) {
					throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
				}

				shader.set_uniform_i("use_diffuse_map", diffuse_map.get_id() != 0);
				shader.set_uniform_i("use_specular_map", specular_map.get_id() != 0);
				shader.set_uniform_i("use_emission_map", emission_map.get_id() != 0);

				if (diffuse_map.get_id() == 0) {
					shader.set_uniform_f("object_material.ambient", static_cast<GLfloat>(ambient_.x), static_cast<GLfloat>(ambient_.y), static_cast<GLfloat>(ambient_.z));
					shader.set_uniform_f("object_material.diffuse", static_cast<GLfloat>(diffuse_.x), static_cast<GLfloat>(diffuse_.y), static_cast<GLfloat>(diffuse_.z));
					shader.set_uniform_f("object_material.alpha", static_cast<GLfloat>(alpha_));
				}

				if (specular_map.get_id() == 0) {
					shader.set_uniform_f("object_material.specular", static_cast<GLfloat>(specular_.x), static_cast<GLfloat>(specular_.y), static_cast<GLfloat>(specular_.z));
				}
				shader.set_uniform_f("object_material.shininess", static_cast<GLfloat>(shininess_));

				if (emission_map.get_id() == 0) {
					shader.set_uniform_f("object_material.emission", static_cast<GLfloat>(emission_.x), static_cast<GLfloat>(emission_.y), static_cast<GLfloat>(emission_.z));
				}

				shader.set_uniform_i("object_material.use_vertex_color", use_vertex_color);
				shader.set_uniform_i("object_material.shadow", shadow);

				diffuse_map.activate(0);
				specular_map.activate(1);
				emission_map.activate(2);
			}

			void delete_uniforms(const Shader<size_t>& shader) const {
				if (shader.description != eng::ShaderType::MAIN) {
					throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
				}

				diffuse_map.deactive(0);
				specular_map.deactive(1);
				emission_map.deactive(2);
			}

		public:
			bool shadow = true;
			bool use_vertex_color = false;

			Texture diffuse_map;
			Texture specular_map;
			Texture emission_map;

			void set_shininess(double shininess)& {
				if (shininess < 0.0) {
					throw EngInvalidArgument(__FILE__, __LINE__, "set_shininess, invalid shininess value.\n\n");
				}

				shininess_ = shininess;
			}

			void set_alpha(double alpha)& {
				if (alpha < 0.0 || 1.0 < alpha) {
					throw EngInvalidArgument(__FILE__, __LINE__, "set_alpha, invalid alpha value.\n\n");
				}

				alpha_ = alpha;
			}

			void set_ambient(const Vect3& ambient)& {
				check_color_value(__FILE__, __LINE__, __func__, ambient);
				ambient_ = ambient;
			}

			void set_diffuse(const Vect3& diffuse)& {
				check_color_value(__FILE__, __LINE__, __func__, diffuse);
				diffuse_ = diffuse;
			}

			void set_specular(const Vect3& specular)& {
				check_color_value(__FILE__, __LINE__, __func__, specular);
				specular_ = specular;
			}

			void set_emission(const Vect3& emission)& {
				check_color_value(__FILE__, __LINE__, __func__, emission);
				emission_ = emission;
			}
		};

		bool frame = false;

		Material material;

		Mesh() {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "Mesh, failed to initialize GLEW.\n\n");
			}

			count_points_ = 0;
			count_indices_ = 0;
		}

		// Default polygon shape
		explicit Mesh(GLsizei count_points) {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "Mesh, failed to initialize GLEW.\n\n");
			}

			if (count_points < 2) {
				throw EngInvalidArgument(__FILE__, __LINE__, "Mesh, invalid number of points.\n\n");
			}

			count_points_ = count_points;
			count_indices_ = (count_points - 2) * 3;

			create_vertex_array();

			std::vector<GLuint> indices(count_indices_);
			for (GLsizei i = 0; i < count_points - 2; ++i) {
				indices[3 * static_cast<size_t>(i)] = 0;
				indices[3 * static_cast<size_t>(i) + 1] = static_cast<GLuint>(i + 1);
				indices[3 * static_cast<size_t>(i) + 2] = static_cast<GLuint>(i + 2);
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
			set_matrix_buffer(other.matrix_buffer_);

			glBindVertexArray(vertex_array_);
			glBindBuffer(GL_COPY_READ_BUFFER, other.vertex_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, vertex_buffer_);

			size_t memory_size = get_value<size_t>(MEMORY_CONFIGURATION.begin(), MEMORY_CONFIGURATION.end(), 0, [](auto element, auto* result) { *result += element; });
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

		Mesh& operator=(Mesh&& other)& {
			deallocate();
			swap(other);
			return *this;
		}

		Mesh& set_border_width(GLfloat border_width)& {
			if (border_width < 0.0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_border_width, invalid border width value.\n\n");
			}

			border_width_ = border_width;
			return *this;
		}

		Mesh& set_positions(const std::vector<Vect3>& positions, bool update_normals = false)& {
			if (positions.size() != count_points_) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_positions, invalid number of points.\n\n");
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
					throw EngInvalidArgument(__FILE__, __LINE__, "set_positions, invalid number of points for automatic calculation of normals.\n\n");
				}

				set_normals(std::vector<Vect3>(count_points_, (positions[2] - positions[0]) ^ (positions[1] - positions[0])));
			}
			return *this;
		}

		Mesh& set_normals(const std::vector<Vect3>& normals)& {
			if (normals.size() != count_points_) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_normals, invalid number of points.\n\n");
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

		Mesh& set_tex_coords(const std::vector<Vect2>& tex_coords)& {
			if (tex_coords.size() != count_points_) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_tex_coords, invalid number of points.\n\n");
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

		Mesh& set_colors(const std::vector<Vect3>& colors)& {
			if (colors.size() != count_points_) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_colors, invalid number of points.\n\n");
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

		Mesh& set_indices(const std::vector<GLuint>& indices)& {
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

		Mesh& set_matrix_buffer(GLuint matrix_buffer)& {
			matrix_buffer_ = matrix_buffer;
			if (matrix_buffer == 0) {
				return *this;
			}

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
			glBindVertexArray(vertex_array_);

			GLuint attrib_offset = static_cast<GLuint>(MEMORY_CONFIGURATION.size());
			for (GLuint i = 0; i < 4; ++i) {
				glVertexAttribPointer(attrib_offset + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 4 * i));
				glEnableVertexAttribArray(attrib_offset + i);
				glVertexAttribDivisor(attrib_offset + i, 1);
			}

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

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

		std::vector<Vect3> get_positions() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vect3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<Vect3> get_normals() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * count_points_, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vect3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<Vect2> get_tex_coords() const {
			GLvoid* buffer = new GLfloat[2 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * count_points_, sizeof(GLfloat) * 2 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vect2::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<Vect3> get_colors() const {
			GLvoid* buffer = new GLfloat[3 * count_points_];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * count_points_, sizeof(GLfloat) * 3 * count_points_, buffer);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return Vect3::move_in(count_points_, reinterpret_cast<GLfloat*>(buffer));
		}

		std::vector<GLuint> get_indices() const {
			GLuint* buffer = new GLuint[count_indices_];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLuint) * count_indices_, reinterpret_cast<GLvoid*>(buffer));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			std::vector<GLuint> result(count_indices_);
			for (size_t i = 0; i < count_indices_; ++i) {
				result[i] = buffer[i];
			}
			delete[] buffer;

			return result;
		}

		Vect3 get_center() const {
			std::vector<Vect3> positions = get_positions();
			return get_value<Vect3>(positions.begin(), positions.end(), Vect3(0, 0, 0), [&](auto element, auto* result) { *result += element; }) / static_cast<double>(positions.size());
		}

		Mesh& apply_matrix(const Matrix& transform)& {
			Matrix normal_transform = Matrix::normal_transform(transform);
			std::vector<Vect3> positions = get_positions();
			std::vector<Vect3> normals = get_normals();
			for (size_t i = 0; i < count_points_; ++i) {
				positions[i] = transform * positions[i];
				normals[i] = normal_transform * normals[i];
			}
			set_positions(positions);
			set_normals(normals);
			return *this;
		}

		Mesh& invert_points_order(bool update_normals = false)& {
			std::vector<Vect3> positions = get_positions();
			std::reverse(positions.begin(), positions.end());
			set_positions(positions, update_normals);
			return *this;
		}

		void draw(GLsizei count, const Shader<size_t>& shader) const {
			if (count == 0) {
				return;
			}

			set_uniforms(shader);

			glBindVertexArray(vertex_array_);
			if (!frame) {
				glDrawElementsInstanced(GL_TRIANGLES, count_indices_, GL_UNSIGNED_INT, NULL, count);
			} else {
				glDrawElementsInstanced(GL_LINE_LOOP, count_indices_, GL_UNSIGNED_INT, NULL, count);
			}
			glBindVertexArray(0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			delete_uniforms(shader);
		}

		~Mesh() {
			deallocate();
		}

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}
