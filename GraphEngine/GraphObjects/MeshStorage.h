#pragma once

#include "Mesh.h"


namespace gre {
	class MeshStorage {
		friend class GraphObject;

		GLuint matrix_buffer_ = 0;

		std::vector<size_t> meshes_index_;
		std::vector<size_t> free_mesh_id_;
		std::vector<std::pair<size_t, Mesh>> meshes_;

		MeshStorage() noexcept {
		}

		MeshStorage(const MeshStorage& other) {
			meshes_index_ = other.meshes_index_;
			free_mesh_id_ = other.free_mesh_id_;
			meshes_ = other.meshes_;

			set_matrix_buffer(other.matrix_buffer_);
		}

		MeshStorage(MeshStorage&& other) noexcept {
			swap(other);
		}

		MeshStorage& operator=(const MeshStorage& other)& {
			MeshStorage object(other);
			swap(object);
			return *this;
		}

		MeshStorage& operator=(MeshStorage&& other)& noexcept {
			swap(other);
			return *this;
		}

		void set_mesh_matrix_buffer(Mesh& mesh) const {
			if (matrix_buffer_ == 0) {
				return;
			}

			glBindVertexArray(mesh.get_vertex_array());
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

			GLuint attrib_offset = static_cast<GLuint>(Mesh::get_count_params());
			for (GLuint i = 0; i < 4; ++i) {
				glVertexAttribPointer(attrib_offset + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 4 * i));
				glEnableVertexAttribArray(attrib_offset + i);
				glVertexAttribDivisor(attrib_offset + i, 1);
			}

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void set_matrix_buffer(GLuint matrix_buffer) {
			if (matrix_buffer_ == matrix_buffer) {
				return;
			}

			matrix_buffer_ = matrix_buffer;

			for (auto& [id, mesh] : meshes_) {
				set_mesh_matrix_buffer(mesh);
			}
		}

		void swap(MeshStorage& other) noexcept {
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(meshes_index_, other.meshes_index_);
			std::swap(free_mesh_id_, other.free_mesh_id_);
			std::swap(meshes_, other.meshes_);
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Mesh>>::const_iterator;

		const Mesh& operator[](size_t id) const {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid mesh id.\n\n");
			}

			return meshes_[meshes_index_[id]].second;
		}

		Mesh get(size_t id) const {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get, invalid mesh id.\n\n");
			}

			return meshes_[meshes_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid mesh id.\n\n");
			}

			return meshes_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			if (meshes_.size() <= memory_id) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}

			return meshes_[memory_id].first;
		}

		bool contains(size_t id) const noexcept {
			return id < meshes_index_.size() && meshes_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < meshes_.size();
		}

		size_t size() const noexcept {
			return meshes_.size();
		}

		bool empty() const noexcept {
			return meshes_.empty();
		}

		Iterator begin() const noexcept {
			return meshes_.begin();
		}

		Iterator end() const noexcept {
			return meshes_.end();
		}

		MeshStorage& erase(size_t id) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid mesh id.\n\n");
			}

			free_mesh_id_.push_back(id);

			meshes_index_[meshes_.back().first] = meshes_index_[id];
			meshes_[meshes_index_[id]].swap(meshes_.back());

			meshes_.pop_back();
			meshes_index_[id] = std::numeric_limits<size_t>::max();
			return *this;
		}

		MeshStorage& clear() noexcept {
			meshes_index_.clear();
			free_mesh_id_.clear();
			meshes_.clear();
			return *this;
		}

		size_t insert(const Mesh& mesh) {
			size_t free_mesh_id = meshes_index_.size();
			if (free_mesh_id_.empty()) {
				meshes_index_.push_back(meshes_.size());
			} else {
				free_mesh_id = free_mesh_id_.back();
				free_mesh_id_.pop_back();
				meshes_index_[free_mesh_id] = meshes_.size();
			}

			meshes_.push_back({ free_mesh_id, mesh });
			set_mesh_matrix_buffer(meshes_.back().second);
			return free_mesh_id;
		}

		MeshStorage& modify(size_t id, const Mesh& mesh) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "modify, invalid mesh id.\n\n");
			}

			set_mesh_matrix_buffer(meshes_[meshes_index_[id]].second = mesh);
			return *this;
		}

		MeshStorage& apply_func(size_t id, std::function<void(Mesh&)> func) {
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "apply_func, invalid mesh id.\n\n");
			}

			Mesh object(meshes_[meshes_index_[id]].second);
			func(object);
			set_mesh_matrix_buffer(meshes_[meshes_index_[id]].second = object);
			return *this;
		}

		MeshStorage& apply_func(std::function<void(Mesh&)> func) {
			for (auto& [id, mesh] : meshes_) {
				Mesh object(mesh);
				func(object);
				set_mesh_matrix_buffer(mesh = object);
			}
			return *this;
		}

		MeshStorage& compress() {
			std::vector<Mesh> new_meshes;
			while (!meshes_.empty()) {
				Mesh current_mesh = meshes_[0].second;
				std::vector<Vec3> positions;
				std::vector<Vec3> normals;
				std::vector<Vec2> tex_coords;
				std::vector<Vec3> colors;
				std::vector<GLuint> indices;
				for (size_t i = 0; i < meshes_.size(); ++i) {
					if (meshes_[i].second != current_mesh) {
						continue;
					}

					for (GLuint index : meshes_[i].second.get_indices()) {
						indices.push_back(static_cast<GLuint>(positions.size()) + index);
					}
					for (const Vec3& position : meshes_[i].second.get_positions()) {
						positions.push_back(position);
					}
					for (const Vec3& normal : meshes_[i].second.get_normals()) {
						normals.push_back(normal);
					}
					for (const Vec2& tex_coord : meshes_[i].second.get_tex_coords()) {
						tex_coords.push_back(tex_coord);
					}
					for (const Vec3& color : meshes_[i].second.get_colors()) {
						colors.push_back(color);
					}

					erase(meshes_[i].first);
					--i;
				}

				new_meshes.push_back(Mesh(positions.size()));
				new_meshes.back().set_positions(positions);
				new_meshes.back().set_normals(normals);
				new_meshes.back().set_tex_coords(tex_coords);
				new_meshes.back().set_colors(colors);
				new_meshes.back().set_indices(indices);
			}

			clear();
			for (const Mesh& mesh : new_meshes) {
				insert(mesh);
			}
			return *this;
		}
	};
}
