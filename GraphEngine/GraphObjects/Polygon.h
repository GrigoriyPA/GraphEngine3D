#pragma once

#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>


class Material {
public:
	bool light = false;
	double shininess = 1, alpha = 1;
	eng::Vect3 ambient = eng::Vect3(0, 0, 0), diffuse = eng::Vect3(0, 0, 0), specular = eng::Vect3(0, 0, 0), emission = eng::Vect3(0, 0, 0);

	Texture diffuse_map, specular_map, emission_map;

	void set_uniforms(Shader* shader_program) {
		try {
			shader_program->use();
			glUniform1i(glGetUniformLocation(shader_program->program, "use_diffuse_map"), diffuse_map.texture_id);
			glUniform1i(glGetUniformLocation(shader_program->program, "use_specular_map"), specular_map.texture_id);
			glUniform1i(glGetUniformLocation(shader_program->program, "use_emission_map"), emission_map.texture_id);

			if (!diffuse_map.texture_id) {
				glUniform3f(glGetUniformLocation(shader_program->program, "object_material.ambient"), ambient.x, ambient.y, ambient.z);
				glUniform3f(glGetUniformLocation(shader_program->program, "object_material.diffuse"), diffuse.x, diffuse.y, diffuse.z);
				glUniform1f(glGetUniformLocation(shader_program->program, "object_material.alpha"), alpha);
			}
			if (!specular_map.texture_id)
				glUniform3f(glGetUniformLocation(shader_program->program, "object_material.specular"), specular.x, specular.y, specular.z);
			if (!emission_map.texture_id)
				glUniform3f(glGetUniformLocation(shader_program->program, "object_material.emission"), emission.x, emission.y, emission.z);
			glUniform1f(glGetUniformLocation(shader_program->program, "object_material.shininess"), shininess);
			glUniform1i(glGetUniformLocation(shader_program->program, "object_material.light"), light);

			diffuse_map.active(0);
			specular_map.active(1);
			emission_map.active(2);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::TEX_MATERIAL::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}

	void delete_uniforms(Shader* shader_program) {
		try {
			shader_program->use();
			emission_map.deactive(2);
			specular_map.deactive(1);
			diffuse_map.deactive(0);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::TEX_MATERIAL::DELETE_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}
};


class Polygon {
	double eps = 0.00001;
	unsigned int matrix_buffer = 0;
	eng::Matrix polygon_trans = eng::one_matrix(4);

	int count_points;
	unsigned int vertex_array = 0, vertex_buffer = 0, index_buffer = 0;
	std::vector < eng::Vect2 > tex_coords;
	std::vector < eng::Vect3 > positions, normals;
	std::vector < unsigned int > indices;
	eng::Vect3 center;

	void set_uniforms(Shader* shader_program) {
		if (shader_program == nullptr)
			return;

		try {
			shader_program->use();
			material.set_uniforms(shader_program);

			glLineWidth(border_width);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::POLYGON::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}

	void create_vertex_array() {
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * count_points, NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float) * 3 * count_points));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) * 6 * count_points));
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		if (count_points > 0 && indices.empty()) {
			indices.resize((count_points - 2) * 3);
			for (int i = 0; i < count_points - 2; i++) {
				indices[3 * i] = 0;
				indices[3 * i + 1] = i + 1;
				indices[3 * i + 2] = i + 2;
			}
		}
		if (!indices.empty())
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* indices.size(), &indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void delete_uniforms(Shader* shader_program) {
		if (shader_program == nullptr)
			return;

		try {
			shader_program->use();
			material.delete_uniforms(shader_program);

			glLineWidth(1);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::POLYGON::DELETE_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}

	void delete_buffers() {
		glDeleteVertexArrays(1, &vertex_array);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteBuffers(1, &index_buffer);
	}

public:
	bool frame = false;
	double border_width = 1;

	Material material;

	Polygon(const Polygon& object) {
		*this = object;
	}

	Polygon(int count_points = 0) {
		if (count_points < 0) {
			std::cout << "ERROR::POLYGON::BUILDER\n" << "Negative number of points.\n";
			assert(0);
		}

		this->count_points = count_points;
		positions.resize(count_points);
		normals.resize(count_points);
		tex_coords.resize(count_points);
		
		create_vertex_array();
	}

	Polygon& operator=(const Polygon& other) {
		delete_buffers();

		eps = other.eps;
		polygon_trans = other.polygon_trans;
		positions = other.positions;
		normals = other.normals;
		indices = other.indices;
		count_points = other.count_points;
		center = other.center;
		frame = other.frame;
		border_width = other.border_width;
		material = other.material;
		tex_coords = other.tex_coords;

		create_vertex_array();
		set_matrix_buffer(other.matrix_buffer);

		glBindVertexArray(vertex_array);
		glBindBuffer(GL_COPY_READ_BUFFER, other.vertex_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, vertex_buffer);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float) * 8 * count_points);

		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindVertexArray(0);

		return *this;
	}

	void set_positions(std::vector < eng::Vect3 > positions, bool update_normals = true) {
		if (positions.size() != count_points) {
			std::cout << "ERROR::POLYGON::SET_POSITIONS\n" << "Wrong number of points.\n";
			assert(0);
		}

		this->positions = positions;

		center = eng::Vect3(0, 0, 0);
		std::vector < float > converted_positions(count_points * 3);
		for (int i = 0; i < count_points; i++) {
			eng::Vect3 position = polygon_trans * positions[i];
			
			center += positions[i];
			for (int j = 0; j < 3; j++)
				converted_positions[3 * i + j] = position[j];
		}
		center /= count_points;

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * count_points, &converted_positions[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (update_normals) {
			eng::Vect3 normal = (positions[2] - positions[0]) ^ (positions[1] - positions[0]);

			std::vector < eng::Vect3 > normals(count_points, normal);
			set_normals(normals);
		}
	}

	void set_normals(std::vector < eng::Vect3 > normals) {
		if (normals.size() != count_points) {
			std::cout << "ERROR::POLYGON::SET_NORMALS\n" << "Wrong number of points.\n";
			assert(0);
		}

		this->normals = normals;

		eng::Matrix nomals_trans = polygon_trans.inverse().transpose();
		std::vector < float > converted_normals(count_points * 3);
		for (int i = 0; i < count_points; i++) {
			eng::Vect3 normal = nomals_trans * normals[i];

			for (int j = 0; j < 3; j++)
				converted_normals[3 * i + j] = normal[j];
		}

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 3 * count_points, sizeof(float) * 3 * count_points, &converted_normals[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void set_tex_coords(std::vector < eng::Vect2 > tex_coords) {
		if (tex_coords.size() != count_points) {
			std::cout << "ERROR::POLYGON::SET_TEX_COORDS\n" << "Wrong number of points.\n";
			assert(0);
		}

		this->tex_coords = tex_coords;

		std::vector < float > converted_tex_coords(count_points * 2);
		for (int i = 0; i < count_points; i++) {
			for (int j = 0; j < 2; j++)
				converted_tex_coords[2 * i + j] = tex_coords[i][j];
		}

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6 * count_points, sizeof(float) * 2 * count_points, &converted_tex_coords[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void set_indices(std::vector < unsigned int > indices) {
		this->indices = indices;

		glBindVertexArray(vertex_array);

		glDeleteBuffers(1, &index_buffer);
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void set_matrix_buffer(unsigned int matrix_buffer) {
		if (matrix_buffer == 0)
			return;

		this->matrix_buffer = matrix_buffer;

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBindVertexArray(vertex_array);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)0);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 4));
		glEnableVertexAttribArray(4);

		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 8));
		glEnableVertexAttribArray(5);

		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 12));
		glEnableVertexAttribArray(6);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	int get_count_points() {
		return count_points;
	}

	eng::Vect3 get_center() {
		return polygon_trans * center;
	}

	int get_vao() {
		return vertex_array;
	}

	std::vector < eng::Vect3 > get_positions() {
		std::vector < eng::Vect3 > converted_positions;
		for (eng::Vect3 position : positions)
			converted_positions.push_back(polygon_trans * position);

		return converted_positions;
	}

	void change_matrix(eng::Matrix trans) {
		polygon_trans = trans * polygon_trans;
		set_positions(positions, false);
		set_normals(normals);
	}

	void invert_points_order(bool update_normals = true) {
		std::reverse(positions.begin(), positions.end());
		set_positions(positions, update_normals);
	}

	void draw(int count, Shader* shader_program) {
		if (count == 0)
			return;

		if (count < 0) {
			std::cout << "ERROR::POLYGON::DRAW\n" << "Drawing a negative number of polygons.\n";
			assert(0);
		}

		set_uniforms(shader_program);

		glBindVertexArray(vertex_array);
		if (!frame)
			glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, count);
		else
			glDrawElementsInstanced(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0, count);
		glBindVertexArray(0);

		delete_uniforms(shader_program);
	}

	~Polygon() {
		delete_buffers();
	}
};
