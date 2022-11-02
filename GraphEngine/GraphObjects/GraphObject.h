#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <unordered_map>
#include <unordered_set>
#include "Mesh.h"


namespace eng {
	class GraphObject {
		class MeshStorage {
			friend class GraphObject;

			GLuint matrix_buffer_ = 0;

			std::vector<size_t> meshes_index_;
			std::vector<size_t> free_mesh_id_;
			std::vector<std::pair<size_t, Mesh>> meshes_;

			void set_mesh_matrix_buffer(Mesh& mesh) const {
				if (matrix_buffer_ == 0) {
					return;
				}

				glBindVertexArray(mesh.get_vertex_array());
				glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

				GLuint attrib_offset = Mesh::get_count_params();
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
				matrix_buffer_ = matrix_buffer;

				for (auto& [id, mesh] : meshes_) {
					set_mesh_matrix_buffer(mesh);
				}
			}

		public:
			using Iterator = std::vector<std::pair<size_t, Mesh>>::const_iterator;

			MeshStorage() {
				if (!glew_is_ok()) {
					throw EngRuntimeError(__FILE__, __LINE__, "MeshStorage, failed to initialize GLEW.\n\n");
				}
			}

			const Mesh& operator[](size_t id) const {
				if (!contains(id)) {
					throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid mesh id.\n\n");
				}

				return meshes_[meshes_index_[id]].second;
			}

			Mesh get(size_t id) const {
				if (!contains(id)) {
					throw EngOutOfRange(__FILE__, __LINE__, "get, invalid mesh id.\n\n");
				}

				return meshes_[meshes_index_[id]].second;
			}

			bool contains(size_t id) const noexcept {
				return id < meshes_index_.size() && meshes_index_[id] < std::numeric_limits<size_t>::max();
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
					throw EngOutOfRange(__FILE__, __LINE__, "erase, invalid mesh id.\n\n");
				}

				free_mesh_id_.push_back(id);

				meshes_index_[meshes_.back().first] = meshes_index_[id];
				std::swap(meshes_[meshes_index_[id]], meshes_.back());

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
					throw EngOutOfRange(__FILE__, __LINE__, "modify, invalid mesh id.\n\n");
				}

				set_mesh_matrix_buffer(meshes_[meshes_index_[id]].second = mesh);
				return *this;
			}

			MeshStorage& apply_func(size_t id, std::function<void(Mesh&)> func) {
				if (!contains(id)) {
					throw EngOutOfRange(__FILE__, __LINE__, "apply_func, invalid mesh id.\n\n");
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
		};

		// ...
		struct Model {
			size_t used_memory;
			Matrix matrix;

			Model() : matrix(Matrix::one_matrix(4)) {
				used_memory = std::numeric_limits<size_t>::max();
			}

			Model(size_t used_memory, Matrix matrix) : matrix(matrix) {
				this->used_memory = used_memory;
				this->matrix = matrix;
			}
		};

		GLuint matrix_buffer_ = 0;

		size_t max_count_models_;
		std::vector<size_t> used_memory_;
		std::unordered_map<size_t, Model> models_;

		void create_matrix_buffer() {
			glGenBuffers(1, &matrix_buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * max_count_models_, NULL, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			meshes.set_matrix_buffer(matrix_buffer_);
		}

		void draw_meshes(size_t model_id, const Shader<size_t>& shader) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}

			const Model& model = models_.at(model_id);
			shader.set_uniform_i("model_id", static_cast<GLint>(model.used_memory));
			shader.set_uniform_matrix("not_instance_model", model.matrix);

			for (const auto& [id, mesh] : meshes) {
				mesh.draw(1, shader);
			}
		}

		void draw_meshes(const Shader<size_t>& shader) const {
			shader.set_uniform_i("model_id", -1);

			for (const auto& [id, mesh] : meshes) {
				mesh.draw(models_.size(), shader);
			}
		}

		// ...
		std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene, std::string& directory) {
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
				aiString str;
				material->GetTexture(type, i, &str);

				std::string path(str.data);
				if (path[0] == '*') {
					path.erase(path.begin());
					path = "inline_texture" + path + "." + std::string(scene->mTextures[std::stoi(path)]->achFormatHint);
				}

				Texture texture(directory + "/" + path, true);
				textures.push_back(texture);
			}
			return textures;
		}

		// ...
		Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::string& directory, Matrix transform) {
			std::vector < Vect2 > tex_coords;
			std::vector < Vect3 > positions, normals;
			std::vector < Vect3 > colors;
			std::vector<unsigned int> indices;

			Matrix norm_transform = transform.inverse().transpose();
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				positions.push_back(transform * Vect3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
				if (mesh->mNormals)
					normals.push_back(norm_transform * Vect3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
				if (mesh->mTextureCoords[0])
					tex_coords.push_back(Vect2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
				else
					tex_coords.push_back(Vect2(0, 0));
				if (mesh->mColors[0])
					colors.push_back(Vect3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b));
				else
					colors.push_back(Vect3(0, 0, 0));
			}
			Mesh polygon_mesh(positions.size());
			polygon_mesh.set_positions(positions, normals.empty());
			if (!normals.empty())
				polygon_mesh.set_normals(normals);
			polygon_mesh.set_tex_coords(tex_coords);
			polygon_mesh.set_colors(colors);

			polygon_mesh.material.set_diffuse(Vect3(1, 1, 1));
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			polygon_mesh.set_indices(indices);

			if (mesh->mMaterialIndex >= 0) {
				aiColor3D color(0, 0, 0);
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				std::vector<Texture> diffuse_textures = loadMaterialTextures(material, aiTextureType_DIFFUSE, scene, directory);
				if (!diffuse_textures.empty())
					polygon_mesh.material.diffuse_map = diffuse_textures[0];
				//std::cout << material->GetTextureCount(aiTextureType_DIFFUSE) << "\n";

				std::vector<Texture> specular_textures = loadMaterialTextures(material, aiTextureType_SPECULAR, scene, directory);
				if (!specular_textures.empty())
					polygon_mesh.material.specular_map = specular_textures[0];
				//std::cout << material->GetTextureCount(aiTextureType_SPECULAR) << "\n";

				std::vector<Texture> emissive_textures = loadMaterialTextures(material, aiTextureType_EMISSIVE, scene, directory);
				if (!emissive_textures.empty())
					polygon_mesh.material.emission_map = emissive_textures[0];
				//std::cout << material->GetTextureCount(aiTextureType_EMISSIVE) << "\n";

				material->Get(AI_MATKEY_COLOR_AMBIENT, color);
				polygon_mesh.material.set_ambient(Vect3(color.r, color.g, color.b));
				//polygon_mesh.material.ambient.print();

				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				polygon_mesh.material.set_diffuse(Vect3(color.r, color.g, color.b));
				//polygon_mesh.material.diffuse.print();

				material->Get(AI_MATKEY_COLOR_SPECULAR, color);
				polygon_mesh.material.set_specular(Vect3(color.r, color.g, color.b));
				//polygon_mesh.material.specular.print();

				material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
				polygon_mesh.material.set_emission(Vect3(color.r, color.g, color.b));
				//polygon_mesh.material.emission.print();

				float opacity;
				material->Get(AI_MATKEY_OPACITY, opacity);
				polygon_mesh.material.set_alpha(opacity);
				//std::cout << polygon_mesh.material.alpha << "\n";

				float shininess;
				material->Get(AI_MATKEY_SHININESS, shininess);
				polygon_mesh.material.set_shininess(shininess);
				//std::cout << polygon_mesh.material.shininess << "\n";

				//if (polygon_mesh.material.shininess > 0)
				//	polygon_mesh.material.specular = Vect3(1, 1, 1);
			}

			return polygon_mesh;
		}

		// ...
		void processNode(aiNode* node, const aiScene* scene, std::string& directory, Matrix transform) {
			Matrix trans(4, 4, 0);
			aiMatrix4x4 cur_transform = node->mTransformation;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++)
					trans[i][j] = cur_transform[i][j];
			}
			transform = trans * transform;

			for (int i = 0; i < node->mNumMeshes; i++) {
				meshes.insert(processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory, transform));
				break;
			}

			for (unsigned int i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i], scene, directory, transform);
			}
		}

		void deallocate() {
			glDeleteBuffers(1, &matrix_buffer_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			matrix_buffer_ = 0;
		}

		void swap(GraphObject& other) noexcept {
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(max_count_models_, other.max_count_models_);
			std::swap(used_memory_, other.used_memory_);
			std::swap(models_, other.models_);
			std::swap(transparent, other.transparent);
			std::swap(border_mask, other.border_mask);
			std::swap(meshes, other.meshes);
		}

	public:
		bool transparent = false;
		uint8_t border_mask = 0;
		MeshStorage meshes;

		GraphObject() {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "GraphObject, failed to initialize GLEW.\n\n");
			}

			max_count_models_ = 0;
		}

		explicit GraphObject(size_t max_count_models) {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "GraphObject, failed to initialize GLEW.\n\n");
			}

			max_count_models_ = max_count_models;

			create_matrix_buffer();
		}

		GraphObject(const GraphObject& other) {
			max_count_models_ = other.max_count_models_;
			used_memory_ = other.used_memory_;
			models_ = other.models_;
			transparent = other.transparent;
			border_mask = other.border_mask;
			meshes = other.meshes;

			create_matrix_buffer();

			glBindBuffer(GL_COPY_READ_BUFFER, other.matrix_buffer_);
			glBindBuffer(GL_COPY_WRITE_BUFFER, matrix_buffer_);

			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLfloat) * 16 * max_count_models_);

			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
			glBindBuffer(GL_COPY_READ_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		GraphObject(GraphObject&& other) noexcept {
			swap(other);
		}

		GraphObject& operator=(const GraphObject& other)& {
			GraphObject object(other);
			swap(object);
			return *this;
		}

		GraphObject& operator=(GraphObject&& other)& {
			deallocate();
			swap(other);
			return *this;
		}

		GraphObject& set_matrix(const Matrix& matrix, size_t model_id) {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "set_matrix, invalid model id.\n\n");
			}

			models_[model_id].matrix = matrix;

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_[model_id].used_memory, sizeof(GLfloat) * 16, &std::vector<GLfloat>(matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		Matrix get_matrix(size_t model_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_matrix, invalid model id.\n\n");
			}

			return models_.at(model_id).matrix;
		}

		size_t get_model_id_by_memory_id(size_t memory_id) const {
			if (used_memory_.size() <= memory_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_model_id_by_memory_id, invalid memory id.\n\n");
			}

			return used_memory_[memory_id];
		}

		std::vector<Vect3> get_mesh_positions(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid mesh id.\n\n");
			}

			Matrix transform = models_.at(model_id).matrix;
			std::vector<Vect3> positions;
			for (Vect3 position : meshes[mesh_id].get_positions()) {
				positions.push_back(transform * position);
			}
			return positions;
		}

		std::vector<Vect3> get_mesh_normals(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid mesh id.\n\n");
			}

			Matrix transform = Matrix::normal_transform(models_.at(model_id).matrix);
			std::vector<Vect3> normals;
			for (Vect3 normal : meshes[mesh_id].get_normals()) {
				normals.push_back(transform * normal);
			}
			return normals;
		}

		Vect3 get_mesh_center(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid mesh id.\n\n");
			}

			return models_.at(model_id).matrix * meshes[mesh_id]
				.get_center();
		}

		Vect3 get_center(size_t model_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_center, invalid model id.\n\n");
			}
			if (meshes.size() == 0) {
				throw EngDomainError(__FILE__, __LINE__, "get_center, object does not contain vertices.\n\n");
			}

			Vect3 center(0, 0, 0);
			std::unordered_set<Vect3> used_positions;
			for (const auto& [id, mesh] : meshes) {
				for (const Vect3& position : mesh.get_positions()) {
					if (used_positions.count(position) == 1) {
						continue;
					}

					used_positions.insert(position);
					center += position;
				}
			}
			return models_.at(model_id).matrix * (center / static_cast<double>(used_positions.size()));
		}

		// ...
		std::vector < std::pair < int, int > > getModels() {
			std::vector < std::pair < int, int > > models_description;
			for (std::unordered_map < size_t, Model >::iterator model = models_.begin(); model != models_.end(); model++) {
				for (auto polygon = meshes.begin(); polygon != meshes.end(); polygon++)
					models_description.push_back({ model->first, polygon->first });
			}

			return models_description;
		}

		size_t count_models() const noexcept {
			return models_.size();
		}

		bool contains_model(size_t model_id) const noexcept {
			return models_.count(model_id) == 1;
		}

		size_t add_model(const Matrix& matrix = Matrix::one_matrix(4)) {
			if (models_.size() == max_count_models_) {
				throw EngRuntimeError(__FILE__, __LINE__, "add_model, too many instances created.\n\n");
			}

			size_t free_model_id = 0;
			for (; models_.count(free_model_id) == 1; ++free_model_id) {}

			used_memory_.push_back(free_model_id);
			models_[free_model_id] = Model(models_.size(), matrix);

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_[free_model_id].used_memory, sizeof(GLfloat) * 16, &std::vector<GLfloat>(matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return free_model_id;
		}

		GraphObject& delete_model(size_t model_id) {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "delete_model, invalid model id.\n\n");
			}

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			for (size_t i = models_[model_id].used_memory + 1; i < used_memory_.size(); ++i) {
				used_memory_[i - 1] = used_memory_[i];
				models_[used_memory_[i - 1]].used_memory = i - 1;

				glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * (i - 1), sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[used_memory_[i - 1]].matrix)[0]);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			used_memory_.pop_back();
			models_.erase(model_id);
			return *this;
		}

		GraphObject& change_matrix_left(const Matrix& transform, size_t model_id) {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "change_matrix_left, invalid model id.\n\n");
			}

			models_[model_id].matrix = transform * models_[model_id].matrix;

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_[model_id].used_memory, sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[model_id].matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		GraphObject& change_matrix_right(const Matrix& transform, size_t model_id) {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "change_matrix_right, invalid model id.\n\n");
			}

			models_[model_id].matrix = models_[model_id].matrix * transform;

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_[model_id].used_memory, sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[model_id].matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return *this;
		}

		void draw_depth_map() const {
			for (const auto& [id, mesh] : meshes) {
				if (!mesh.material.shadow) {
					continue;
				}

				mesh.draw(models_.size(), Shader<size_t>());
			}
		}

		void draw(size_t model_id, size_t mesh_id, const Shader<size_t>& shader) const {
			if (shader.description != eng::ShaderType::MAIN) {
				throw EngInvalidArgument(__FILE__, __LINE__, "draw, invalid shader type.\n\n");
			}
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid mesh id.\n\n");
			}

			const Model& model = models_.at(model_id);
			shader.set_uniform_i("model_id", static_cast<GLint>(model.used_memory));
			shader.set_uniform_matrix("not_instance_model", model.matrix);

			if (border_mask > 0) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			meshes[mesh_id].draw(1, shader);

			if (border_mask > 0) {
				glStencilMask(0x00);
			}
		}

		void draw(size_t model_id, const Shader<size_t>& shader) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}

			if (border_mask > 0) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			draw_meshes(model_id, shader);

			if (border_mask > 0) {
				glStencilMask(0x00);
			}
		}

		void draw(const Shader<size_t>& shader_program) const {
			if (border_mask > 0) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			draw_meshes(shader_program);

			if (border_mask > 0) {
				glStencilMask(0x00);
			}
		}

		// ...
		void importFromFile(std::string path) {
			meshes.clear();

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

			if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == NULL) {
				//std::cout << "ERROR::GRAPH_OBJECT::IMPORT\n" << importer.GetErrorString() << "\n";
				//assert(0);
			}
			std::string directory = path.substr(0, path.find_last_of('/'));

			for (int i = 0; i < scene->mNumTextures; i++) {
				aiTexture* tex = scene->mTextures[i];

				if (tex->mHeight == 0) {
					std::ofstream fout(directory + "/inline_texture" + std::to_string(i) + "." + std::string(tex->achFormatHint), std::ios::binary);
					fout.write((const char*)tex->pcData, tex->mWidth);
				}
			}

			processNode(scene->mRootNode, scene, directory, Matrix::one_matrix(4));
		}

		~GraphObject() {
			deallocate();
		}
	};
}

#include "Cube.h"
#include "Cylinder.h"
#include "Sphere.h"
