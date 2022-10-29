#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <unordered_map>
#include <unordered_set>
#include "Mesh.h"


namespace eng {
	class GraphObject {
		// ...
		struct Model {
			bool border;
			int used_memory;
			Matrix matrix;

			Model(Matrix matrix = Matrix::one_matrix(4), bool border = false, int used_memory = -1) : matrix(matrix) {
				this->matrix = matrix;
				this->border = border;
				this->used_memory = used_memory;
			}
		};

		inline static double eps_ = 1e-5;

		size_t count_borders_ = 0;
		GLuint matrix_buffer_ = 0;

		size_t max_count_models_;
		std::vector<int> used_memory_;
		std::unordered_map<size_t, Model> models_;
		std::unordered_map<size_t, Mesh> meshes_;

		void set_uniforms(GLint object_id, const Shader<size_t>& shader) const {
			if (shader.description != eng::ShaderType::MAIN) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_uniforms, invalid shader type.\n\n");
			}

			shader.set_uniform_i("object_id", object_id);
		}

		void create_matrix_buffer() {
			glGenBuffers(1, &matrix_buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);

			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * max_count_models_, NULL, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			for (auto& [id, mesh] : meshes_) {
				mesh.set_matrix_buffer(matrix_buffer_);
			}
		}

		void draw_meshes(size_t model_id, const Shader<size_t>& shader) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}

			const Model& model = models_.at(model_id);
			shader.set_uniform_i("model_id", model.used_memory);
			shader.set_uniform_matrix("not_instance_model", model.matrix);

			for (const auto& [id, mesh] : meshes_) {
				mesh.draw(1, shader);
			}
		}

		void draw_meshes(const Shader<size_t>& shader) const {
			shader.set_uniform_i("model_id", -1);

			for (const auto& [id, mesh] : meshes_) {
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
				meshes_[meshes_.size()] = processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory, transform);
				meshes_[meshes_.size() - 1].set_matrix_buffer(matrix_buffer_);
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
			std::swap(border_mask, other.border_mask);
			std::swap(count_borders_, other.count_borders_);
			std::swap(matrix_buffer_, other.matrix_buffer_);
			std::swap(max_count_models_, other.max_count_models_);
			std::swap(used_memory_, other.used_memory_);
			std::swap(models_, other.models_);
			std::swap(meshes_, other.meshes_);
			std::swap(transparent, other.transparent);
		}

	public:
		bool transparent = false;
		uint8_t border_mask = 1;

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
			used_memory_.resize(max_count_models, -1);

			create_matrix_buffer();
		}

		GraphObject(const GraphObject& other) {
			border_mask = other.border_mask;
			count_borders_ = other.count_borders_;
			max_count_models_ = other.max_count_models_;
			used_memory_ = other.used_memory_;
			models_ = other.models_;
			meshes_ = other.meshes_;
			transparent = other.transparent;

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

		Mesh& operator[](size_t mesh_id) {
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid mesh id.\n\n");
			}

			return meshes_[mesh_id];
		}

		const Mesh& operator[](size_t mesh_id) const {
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid mesh id.\n\n");
			}

			return meshes_.at(mesh_id);
		}

		GraphObject& set_matrix(const Matrix& matrix, size_t model_id)& {
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

		GraphObject& set_border(bool border, size_t model_id)& {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "set_border, invalid model id.\n\n");
			}

			count_borders_ += static_cast<int32_t>(border) - static_cast<int32_t>(models_[model_id].border);
			models_[model_id].border = border;
			return *this;
		}

		Matrix get_matrix(size_t model_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_matrix, invalid model id.\n\n");
			}

			return models_.at(model_id).matrix;
		}

		size_t get_model_id_by_memory_id(size_t memory_id) const {
			if (models_.size() <= memory_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_model_id_by_memory_id, invalid memory id.\n\n");
			}

			return used_memory_[memory_id];
		}

		std::vector<Vect3> get_mesh_positions(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid model id.\n\n");
			}
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid mesh id.\n\n");
			}

			Matrix transform = models_.at(model_id).matrix;
			std::vector<Vect3> positions;
			for (Vect3 position : meshes_.at(mesh_id).get_positions()) {
				positions.push_back(transform * position);
			}
			return positions;
		}

		std::vector<Vect3> get_mesh_normals(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid model id.\n\n");
			}
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid mesh id.\n\n");
			}

			Matrix transform = Matrix::normal_transform(models_.at(model_id).matrix);
			std::vector<Vect3> normals;
			for (Vect3 normal : meshes_.at(mesh_id).get_normals()) {
				normals.push_back(transform * normal);
			}
			return normals;
		}

		Vect3 get_mesh_center(size_t model_id, size_t mesh_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid model id.\n\n");
			}
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid mesh id.\n\n");
			}

			return models_.at(model_id).matrix * meshes_.at(mesh_id).get_center();
		}

		Vect3 get_center(size_t model_id) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_center, invalid model id.\n\n");
			}
			if (meshes_.size() == 0) {
				throw EngDomainError(__FILE__, __LINE__, "get_center, object does not contain vertices.\n\n");
			}

			Vect3 center(0, 0, 0);
			std::unordered_set<Vect3> used_positions;
			for (const auto& [id, mesh] : meshes_) {
				for (const Vect3& position : mesh.get_positions()) {
					if (used_positions.count(position) == 1) {
						continue;
					}

					used_positions.insert(position);
					center += position;
				}
			}
			return models_.at(model_id).matrix * (center / used_positions.size());
		}

		// ...
		std::vector < std::pair < int, int > > getModels() {
			std::vector < std::pair < int, int > > models_description;
			for (std::unordered_map < size_t, Model >::iterator model = models_.begin(); model != models_.end(); model++) {
				for (std::unordered_map < size_t, Mesh >::iterator polygon = meshes_.begin(); polygon != meshes_.end(); polygon++)
					models_description.push_back({ model->first, polygon->first });
			}

			return models_description;
		}

		size_t count_models() const noexcept {
			return models_.size();
		}

		size_t count_meshes() const noexcept {
			return meshes_.size();
		}

		bool contains_model(size_t model_id) const noexcept {
			return static_cast<bool>(models_.count(model_id));
		}

		bool contains_mesh(size_t mesh_id) const noexcept {
			return static_cast<bool>(meshes_.count(mesh_id));
		}

		GraphObject& apply_func_meshes(std::function<void(Mesh&)> func)& {
			for (auto& [id, mesh] : meshes_) {
				func(mesh);
				mesh.set_matrix_buffer(matrix_buffer_);
			}
			return *this;
		}

		size_t add_model(const Matrix& matrix = Matrix::one_matrix(4))& {
			if (models_.size() == max_count_models_) {
				throw EngRuntimeError(__FILE__, __LINE__, "add_model, too many instances created.\n\n");
			}

			size_t free_model_id = 0;
			for (; models_.count(free_model_id) == 1; ++free_model_id) {}

			used_memory_[models_.size()] = free_model_id;
			models_[free_model_id] = Model(matrix, false, models_.size());

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * models_[free_model_id].used_memory, sizeof(GLfloat) * 16, &std::vector<GLfloat>(matrix)[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);
			return free_model_id;
		}

		size_t add_mesh(const Mesh& mesh)& {
			size_t free_mesh_id = 0;
			for (; meshes_.count(free_mesh_id) == 1; ++free_mesh_id) {}

			meshes_[free_mesh_id] = mesh;
			meshes_[free_mesh_id].set_matrix_buffer(matrix_buffer_);
			return free_mesh_id;
		}

		GraphObject& delete_model(size_t model_id)& {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "delete_model, invalid model id.\n\n");
			}

			glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer_);
			for (size_t i = models_[model_id].used_memory + 1; i < models_.size(); ++i) {
				used_memory_[i - 1] = used_memory_[i];
				models_[used_memory_[i - 1]].used_memory = i - 1;

				glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * (i - 1), sizeof(GLfloat) * 16, &std::vector<GLfloat>(models_[used_memory_[i - 1]].matrix)[0]);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			check_gl_errors(__FILE__, __LINE__, __func__);

			count_borders_ -= static_cast<int32_t>(models_[model_id].border);
			used_memory_[models_.size() - 1] = -1;
			models_.erase(model_id);
			return *this;
		}

		GraphObject& delete_mesh(size_t mesh_id)& {
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "delete_mesh, invalid mesh id.\n\n");
			}

			meshes_[mesh_id].set_matrix_buffer(0);
			meshes_.erase(mesh_id);
			return *this;
		}

		GraphObject& change_matrix_left(const Matrix& transform, size_t model_id)& {
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

		GraphObject& change_matrix_right(const Matrix& transform, size_t model_id)& {
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
			for (const auto& [id, mesh] : meshes_) {
				if (!mesh.material.shadow) {
					continue;
				}

				mesh.draw(models_.size(), Shader<size_t>());
			}
		}

		void draw(size_t object_id, size_t model_id, size_t mesh_id, const Shader<size_t>& shader) const {
			if (shader.description != eng::ShaderType::MAIN) {
				throw EngInvalidArgument(__FILE__, __LINE__, "draw, invalid shader type.\n\n");
			}
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}
			if (meshes_.count(mesh_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid mesh id.\n\n");
			}

			const Model& model = models_.at(model_id);
			shader.set_uniform_i("model_id", model.used_memory);
			shader.set_uniform_matrix("not_instance_model", model.matrix);
			set_uniforms(object_id, shader);

			if (model.border) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			meshes_.at(mesh_id).draw(1, shader);

			if (model.border) {
				glStencilMask(0x00);
			}
		}

		void draw(size_t object_id, size_t model_id, const Shader<size_t>& shader) const {
			if (models_.count(model_id) == 0) {
				throw EngOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}

			set_uniforms(object_id, shader);

			const Model& model = models_.at(model_id);
			if (model.border) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			draw_meshes(model_id, shader);

			if (model.border) {
				glStencilMask(0x00);
			}
		}

		void draw(size_t object_id, const Shader<size_t>& shader_program) const {
			set_uniforms(object_id, shader_program);

			if (count_borders_ > 0) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);

				for (const auto& [id, model] : models_) {
					if (!model.border) {
						continue;
					}

					draw_meshes(id, shader_program);
				}

				glStencilMask(0x00);
			}

			draw_meshes(shader_program);
		}

		// ...
		void importFromFile(std::string path) {
			meshes_.clear();

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

		static void set_epsilon(double eps) {
			if (eps <= 0) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_epsilon, not positive epsilon value.\n\n");
			}

			eps_ = eps;
		}
	};
}

#include "Cube.h"
#include "Cylinder.h"
#include "Sphere.h"
