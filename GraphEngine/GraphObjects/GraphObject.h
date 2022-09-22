#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Polygon.h"


struct Model {
	bool border;
	int used_memory;
	Matrix matrix;

	Model(Matrix matrix = one_matrix(4), bool border = false, int used_memory = -1) {
		this->matrix = matrix;
		this->border = border;
		this->used_memory = used_memory;
	}
};


class GraphObject {
	int count_borders = 0, border_bit = 1;
	double eps = 0.00001;
	Vect3 center = Vect3(0, 0, 0);

	int max_count_models;
	unsigned int matrix_buffer;
	std::vector < int > used_memory;
	std::unordered_map < int, Model > models;
	std::unordered_map < int, Polygon > polygons;

	void set_uniforms(Shader* shader_program, int object_id) {
		try {
			shader_program->use();
			glUniform1i(glGetUniformLocation(shader_program->program, "object_id"), object_id);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::GRAPH_OBJECT::SET_UNIFORMS\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}

	void create_matrix_buffer() {
		glGenBuffers(1, &matrix_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * max_count_models, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++)
			polygon->second.set_matrix_buffer(matrix_buffer);
	}

	void delete_buffers() {
		glDeleteBuffers(1, &matrix_buffer);
	}

	void draw_polygons(Shader* shader_program, int model_id = -1) {
		if (model_id != -1 && !models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::DRAW_POLYGONS\n" << "Invalid model id.\n";
			assert(0);
		}

		int cnt = models.size();
		if (model_id != -1) {
			glUniformMatrix4fv(glGetUniformLocation(shader_program->program, "not_instance_model"), 1, GL_FALSE, &models[model_id].matrix.value_vector()[0]);
			cnt = 1;
		}

		int cur_id = -1;
		if (model_id != -1)
			cur_id = models[model_id].used_memory;
		glUniform1i(glGetUniformLocation(shader_program->program, "model_id"), cur_id);

		for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++)
			polygon->second.draw(cnt, shader_program);
	}

public:
	bool transparent = false;

	GraphObject(const GraphObject& object) {
		*this = object;
	}

	GraphObject(int max_count_models = 0) {
		if (max_count_models < 0) {
			std::cout << "ERROR::GRAPH_OBJECT::BUILDER\n" << "Negative number of models.\n";
			assert(0);
		}

		this->max_count_models = max_count_models;
		used_memory.resize(max_count_models, -1);
		matrix_buffer = 0;

		create_matrix_buffer();
	}
	
	GraphObject& operator=(const GraphObject& other) {
		delete_buffers();

		count_borders = other.count_borders; 
		border_bit = other.border_bit;
		eps = other.eps;
		center = other.center;
		max_count_models = other.max_count_models;
		used_memory = other.used_memory;
		models = other.models;
		polygons = other.polygons;
		transparent = other.transparent;

		create_matrix_buffer();
		set_center();

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBindBuffer(GL_COPY_READ_BUFFER, other.matrix_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, matrix_buffer);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float) * 16 * max_count_models);

		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return *this;
	}

	Polygon& operator[](int polygon_id) {
		if (!polygons.count(polygon_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::OPERATOR[]\n" << "Invalid polygon id.\n";
			assert(0);
		}

		return polygons[polygon_id];
	}

	void set_center() {
		center = Vect3(0, 0, 0);
		std::vector < Vect3 > used_positions;
		for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++) {
			for (Vect3 position : polygon->second.get_positions()) {
				if (std::count(used_positions.begin(), used_positions.end(), position))
					continue;

				used_positions.push_back(position);
				center += position;
			}
		}
		if (used_positions.size() > 0)
			center /= used_positions.size();
	}

	void set_material(Material material) {
		for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++)
			polygon->second.material = material;
	}

	void set_matrix(Matrix trans, int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::SET_MATRIX\n" << "Invalid model id.\n";
			assert(0);
		}

		models[model_id].matrix = trans;

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 16 * models[model_id].used_memory, sizeof(float) * 16, &models[model_id].matrix.value_vector()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void set_border(bool border, int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::SET_BORDER\n" << "Invalid model id.\n";
			assert(0);
		}

		count_borders += int(border) - int(models[model_id].border);
		models[model_id].border = border;
	}

	void set_border_bit(int bit_id) {
		if (bit_id < 0 || 8 <= bit_id) {
			std::cout << "ERROR::GRAPH_OBJECT::SET_BORDER_BIT\n" << "Invalid bit id.\n";
			assert(0);
		}

		border_bit = 1 << bit_id;
	}

	std::vector < std::pair < int, int > > get_models() {
		std::vector < std::pair < int, int > > models_description;
		for (std::unordered_map < int, Model >::iterator model = models.begin(); model != models.end(); model++) {
			for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++)
				models_description.push_back({ model->first, polygon->first });
		}

		return models_description;
	}

	Vect3 get_center(int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_CENTER\n" << "Invalid model id.\n";
			assert(0);
		}

		return models[model_id].matrix * center;
	}

	Vect3 get_polygon_center(int model_id, int polygon_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_POLYGON_CENTER\n" << "Invalid model id.\n";
			assert(0);
		}

		if (!polygons.count(polygon_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_POLYGON_CENTER\n" << "Invalid polygon id.\n";
			assert(0);
		}

		return models[model_id].matrix * polygons[polygon_id].get_center();
	}

	std::vector < Vect3 > get_polygon_positions(int model_id, int polygon_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_POLYGON_CENTER\n" << "Invalid model id.\n";
			assert(0);
		}

		if (!polygons.count(polygon_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_POLYGON_CENTER\n" << "Invalid polygon id.\n";
			assert(0);
		}

		std::vector < Vect3 > positions;
		for (Vect3 position : polygons[polygon_id].get_positions())
			positions.push_back(models[model_id].matrix * position);

		return positions;
	}

	Matrix get_matrix(int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_MATRIX\n" << "Invalid model id.\n";
			assert(0);
		}

		return models[model_id].matrix;
	}

	int get_count_models() {
		return models.size();
	}

	int get_model_id(int memory_id) {
		if (memory_id == -1)
			return -1;

		if (memory_id < 0 || models.size() <= memory_id) {
			std::cout << "ERROR::GRAPH_OBJECT::GET_MODEL_ID\n" << "Invalid memory id.\n";
			assert(0);
		}

		return used_memory[memory_id];
	}

	int add_polygon(Polygon polygon) {
		int free_polygon_id = 0;
		for (; polygons.count(free_polygon_id); free_polygon_id++) {}

		polygons[free_polygon_id] = polygon;
		polygons[free_polygon_id].set_matrix_buffer(matrix_buffer);

		set_center();

		return free_polygon_id;
	}

	int add_model(Matrix matrix = one_matrix(4)) {
		if (models.size() == max_count_models) {
			std::cout << "ERROR::GRAPH_OBJECT::ADD_MATRYX\n" << "Too many instances created.\n";
			assert(0);
		}

		int free_model_id = 0;
		for (; models.count(free_model_id); free_model_id++) {}

		used_memory[models.size()] = free_model_id;
		models[free_model_id] = Model(matrix, false, models.size());

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 16 * models[free_model_id].used_memory, sizeof(float) * 16, &models[free_model_id].matrix.value_vector()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return free_model_id;
	}

	void delete_model(int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::DELETE_MODEL\n" << "Invalid model id.\n";
			assert(0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		for (int i = models[model_id].used_memory + 1; i < models.size(); i++) {
			used_memory[i - 1] = used_memory[i];
			models[used_memory[i - 1]].used_memory = i - 1;

			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 16 * (i - 1), sizeof(float) * 16, &models[used_memory[i - 1]].matrix.value_vector()[0]);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		count_borders -= int(models[model_id].border);
		used_memory[models.size() - 1] = -1;
		models.erase(model_id);
	}

	void change_matrix(Matrix trans, int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::CHANGE_MATRIX\n" << "Invalid model id.\n";
			assert(0);
		}

		models[model_id].matrix = trans * models[model_id].matrix;

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 16 * models[model_id].used_memory, sizeof(float) * 16, &models[model_id].matrix.value_vector()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	bool is_model(int model_id) {
		return models.count(model_id);
	}

	void draw_depth_map() {
		for (std::unordered_map < int, Polygon >::iterator polygon = polygons.begin(); polygon != polygons.end(); polygon++) {
			if (polygon->second.material.light)
				continue;

			polygon->second.draw(models.size(), nullptr);
		}
	}

	void draw_polygon(Shader* shader_program, int object_id, int model_id, int polygon_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::DRAW_POLYGON\n" << "Invalid model id.\n";
			assert(0);
		}

		if (!polygons.count(polygon_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::DRAW_POLYGON\n" << "Invalid polygon id.\n";
			assert(0);
		}

		set_uniforms(shader_program, object_id);
		glUniformMatrix4fv(glGetUniformLocation(shader_program->program, "not_instance_model"), 1, GL_FALSE, &models[model_id].matrix.value_vector()[0]);
		glUniform1i(glGetUniformLocation(shader_program->program, "model_id"), models[model_id].used_memory);

		if (models[model_id].border) {
			glStencilFunc(GL_ALWAYS, border_bit, 0xFF);
			glStencilMask(border_bit);
		}

		polygons[polygon_id].draw(1, shader_program);

		if (models[model_id].border)
			glStencilMask(0x00);
	}

	void draw(Vect3 view_pos, Shader* shader_program, int object_id, int model_id = -1) {
		if (model_id != -1 && !models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::DRAW\n" << "Invalid model id.\n";
			assert(0);
		}

		set_uniforms(shader_program, object_id);

		if (model_id != -1) {
			if (models[model_id].border) {
				glStencilFunc(GL_ALWAYS, border_bit, 0xFF);
				glStencilMask(border_bit);
			}

			draw_polygons(shader_program, model_id);

			if (models[model_id].border)
				glStencilMask(0x00);

			return;
		}

		if (count_borders) {
			glStencilFunc(GL_ALWAYS, border_bit, 0xFF);
			glStencilMask(border_bit);

			for (std::unordered_map < int, Model >::iterator model = models.begin(); model != models.end(); model++) {
				if (!model->second.border)
					continue;

				draw_polygons(shader_program, model->first);
			}

			glStencilMask(0x00);
		}

		draw_polygons(shader_program);
	}

	void central_scaling(Vect3 scale, int model_id) {
		if (!models.count(model_id)) {
			std::cout << "ERROR::GRAPH_OBJECT::CENTRAL_SCALING\n" << "Invalid model id.\n";
			assert(0);
		}

		Matrix model_matrix = get_matrix(model_id);
		change_matrix(model_matrix * scale_matrix(scale) * model_matrix.inverse(), model_id);
	}

	~GraphObject() {
		delete_buffers();
	}
};

#include "Cube.h"
#include "Cylinder.h"
#include "Sphere.h"
