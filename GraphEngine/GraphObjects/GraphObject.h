#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <unordered_set>
#include "MeshStorage.h"
#include "ModelStorage.h"


namespace gre {
	class GraphObject {
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

				Texture texture;
				texture.load_from_file(directory + "/" + path, true);
				textures.push_back(texture);
			}
			return textures;
		}

		// ...
		Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::string& directory, Matrix4x4 transform) {
			std::vector < Vec2 > tex_coords;
			std::vector < Vec3 > positions, normals;
			std::vector < Vec3 > colors;
			std::vector<unsigned int> indices;

			Matrix4x4 norm_transform = transform.inverse().transpose();
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				positions.push_back(transform * Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
				if (mesh->mNormals)
					normals.push_back(norm_transform * Vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
				if (mesh->mTextureCoords[0])
					tex_coords.push_back(Vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
				else
					tex_coords.push_back(Vec2(0, 0));
				if (mesh->mColors[0])
					colors.push_back(Vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b));
				else
					colors.push_back(Vec3(0, 0, 0));
			}
			Mesh polygon_mesh(positions.size());
			polygon_mesh.set_positions(positions, normals.empty());
			if (!normals.empty())
				polygon_mesh.set_normals(normals);
			polygon_mesh.set_tex_coords(tex_coords);
			polygon_mesh.set_colors(colors);

			polygon_mesh.material.set_diffuse(Vec3(1, 1, 1));
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
				polygon_mesh.material.set_ambient(Vec3(color.r, color.g, color.b));
				//polygon_mesh.material.ambient.print();

				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				polygon_mesh.material.set_diffuse(Vec3(color.r, color.g, color.b));
				//polygon_mesh.material.diffuse.print();

				material->Get(AI_MATKEY_COLOR_SPECULAR, color);
				polygon_mesh.material.set_specular(Vec3(color.r, color.g, color.b));
				//polygon_mesh.material.specular.print();

				material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
				polygon_mesh.material.set_emission(Vec3(color.r, color.g, color.b));
				//polygon_mesh.material.emission.print();

				float opacity;
				material->Get(AI_MATKEY_OPACITY, opacity);
				polygon_mesh.material.set_alpha(opacity);
				//std::cout << polygon_mesh.material.alpha << "\n";

				float shininess;
				material->Get(AI_MATKEY_SHININESS, shininess);
				polygon_mesh.material.set_shininess(shininess);
				//std::cout << polygon_mesh.material.shininess << "\n";

				/*if (shininess > 0)
					polygon_mesh.material.set_specular(Vec3(1, 1, 1));*/
			}

			return polygon_mesh;
		}

		// ...
		void processNode(aiNode* node, const aiScene* scene, std::string& directory, Matrix4x4 transform) {
			Matrix4x4 trans(0);
			aiMatrix4x4 cur_transform = node->mTransformation;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++)
					trans[i][j] = cur_transform[i][j];
			}
			transform = trans * transform;

			for (size_t i = 0; i < node->mNumMeshes; i++) {
				meshes.insert(processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory, transform));
				break;
			}

			for (unsigned int i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i], scene, directory, transform);
			}
		}

		void draw_meshes(size_t model_id, const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "draw_meshes, invalid shader type.\n\n");
			}
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "draw_meshes, invalid model id.\n\n");
			}

			shader.set_uniform_i("model_id", static_cast<GLint>(models.get_memory_id(model_id)));
			shader.set_uniform_matrix("not_instance_model", models[model_id]);

			for (const auto& [id, mesh] : meshes) {
				mesh.draw(1, shader);
			}
		}

		void draw_meshes(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "draw_meshes, invalid shader type.\n\n");
			}
			shader.set_uniform_i("model_id", -1);

			for (const auto& [id, mesh] : meshes) {
				mesh.draw(models.size(), shader);
			}
		}

	public:
		bool transparent = false;
		uint8_t border_mask = 0;

		MeshStorage meshes;
		ModelStorage models;

		GraphObject() {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "GraphObject, failed to initialize GLEW.\n\n");
			}
		}

		explicit GraphObject(size_t max_count_models) {
			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "GraphObject, failed to initialize GLEW.\n\n");
			}

			meshes.set_matrix_buffer(models.create_matrix_buffer(max_count_models));
		}

		GraphObject(const GraphObject& other) {
			transparent = other.transparent;
			border_mask = other.border_mask;
			meshes = other.meshes;
			models = other.models;

			meshes.set_matrix_buffer(models.matrix_buffer_);
		}

		GraphObject(GraphObject&& other) noexcept {
			swap(other);
		}

		GraphObject& operator=(const GraphObject& other)& {
			GraphObject object(other);
			swap(object);
			return *this;
		}

		GraphObject& operator=(GraphObject&& other)& noexcept {
			swap(other);
			return *this;
		}

		std::vector<Vec3> get_mesh_positions(size_t model_id, size_t mesh_id) const {
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid mesh id.\n\n");
			}

			const Matrix4x4& transform = models[model_id];
			std::vector<Vec3> positions;
			for (const Vec3& position : meshes[mesh_id].get_positions()) {
				positions.push_back(transform * position);
			}
			return positions;
		}

		std::vector<Vec3> get_mesh_normals(size_t model_id, size_t mesh_id) const {
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid mesh id.\n\n");
			}

			const Matrix4x4& transform = Matrix4x4::normal_transform(models[model_id]);
			std::vector<Vec3> normals;
			for (const Vec3& normal : meshes[mesh_id].get_normals()) {
				normals.push_back(transform * normal);
			}
			return normals;
		}

		Vec3 get_mesh_center(size_t model_id, size_t mesh_id) const {
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid mesh id.\n\n");
			}

			return models[model_id] * meshes[mesh_id].get_center();
		}

		Vec3 get_center(size_t model_id) const {
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_center, invalid model id.\n\n");
			}
			if (meshes.size() == 0) {
				throw GreDomainError(__FILE__, __LINE__, "get_center, object does not contain vertices.\n\n");
			}

			Vec3 center(0, 0, 0);
			std::unordered_set<Vec3> used_positions;
			for (const auto& [id, mesh] : meshes) {
				for (const Vec3& position : mesh.get_positions()) {
					if (used_positions.count(position) == 1) {
						continue;
					}

					used_positions.insert(position);
					center += position;
				}
			}
			return models[model_id] * (center / static_cast<double>(used_positions.size()));
		}

		void swap(GraphObject& other) noexcept {
			std::swap(transparent, other.transparent);
			std::swap(border_mask, other.border_mask);
			meshes.swap(other.meshes);
			models.swap(other.models);

			meshes.set_matrix_buffer(models.matrix_buffer_);
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

			if (scene == nullptr) {
				return;
			}

			for (size_t i = 0; i < scene->mNumTextures; i++) {
				aiTexture* tex = scene->mTextures[i];

				if (tex->mHeight == 0) {
					std::ofstream fout(directory + "/inline_texture" + std::to_string(i) + "." + std::string(tex->achFormatHint), std::ios::binary);
					fout.write((const char*)tex->pcData, tex->mWidth);
				}
			}

			processNode(scene->mRootNode, scene, directory, Matrix4x4::one_matrix());
		}

		void draw_depth_map() const {
			for (const auto& [id, mesh] : meshes) {
				if (!mesh.material.shadow) {
					continue;
				}

				mesh.draw(models.size(), Shader<size_t>());
			}
		}

		void draw(size_t model_id, size_t mesh_id, const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "draw, invalid shader type.\n\n");
			}
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
			}
			if (!meshes.contains(mesh_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid mesh id.\n\n");
			}

			shader.set_uniform_i("model_id", static_cast<GLint>(models.get_memory_id(model_id)));
			shader.set_uniform_matrix("not_instance_model", models[model_id]);

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
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "draw, invalid shader type.\n\n");
			}
			if (!models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
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

		void draw(const Shader<size_t>& shader) const {
			if (shader.description != ShaderType::MAIN) {
				throw GreInvalidArgument(__FILE__, __LINE__, "draw_meshes, invalid shader type.\n\n");
			}

			if (border_mask > 0) {
				glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
				glStencilMask(border_mask);
			}

			draw_meshes(shader);

			if (border_mask > 0) {
				glStencilMask(0x00);
			}
		}

		static GraphObject cube(size_t max_count_models) {
			GraphObject cube(max_count_models);

			Mesh mesh(4);
			mesh.set_positions({
				Vec3(0.5, 0.5, 0.5),
				Vec3(0.5, -0.5, 0.5),
				Vec3(-0.5, -0.5, 0.5),
				Vec3(-0.5, 0.5, 0.5)
			}, true);
			mesh.set_tex_coords({
				Vec2(1.0, 1.0),
				Vec2(1.0, 0.0),
				Vec2(0.0, 0.0),
				Vec2(0.0, 1.0)
			});
			cube.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::rotation_matrix(Vec3(0.0, 1.0, 0.0), PI / 2.0));
			cube.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::rotation_matrix(Vec3(0.0, 1.0, 0.0), PI / 2.0));
			cube.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::rotation_matrix(Vec3(0.0, 1.0, 0.0), PI / 2.0));
			cube.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::rotation_matrix(Vec3(0.0, 0.0, 1.0), PI / 2.0));
			cube.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::rotation_matrix(Vec3(0.0, 0.0, 1.0), PI));
			cube.meshes.insert(mesh);

			cube.meshes.compress();
			return cube;
		}

		static GraphObject cylinder(size_t count_points, bool real_normals, size_t max_count_models) {
			if (count_points < 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "cylinder, the number of points is less than three.\n\n");
			}

			GraphObject cylinder(max_count_models);

			std::vector<Vec3> positions;
			for (size_t i = 0; i < count_points; ++i) {
				positions.push_back(Vec3(cos((2.0 * PI / count_points) * i), 0.0, sin((2.0 * PI / count_points) * i)));
			}

			Mesh mesh(count_points);
			mesh.set_positions(positions, true);
			mesh.invert_points_order(true);
			cylinder.meshes.insert(mesh);

			mesh.apply_matrix(Matrix4x4::translation_matrix(Vec3(0.0, 1.0, 0.0)));
			mesh.invert_points_order(true);
			cylinder.meshes.insert(mesh);

			for (size_t i = 0; i < count_points; ++i) {
				size_t next = (i + 1) % count_points;

				mesh = Mesh(4);
				mesh.set_positions({
					positions[i],
					positions[next],
					positions[next] + Vec3(0.0, 1.0, 0.0),
					positions[i] + Vec3(0.0, 1.0, 0.0)
				}, !real_normals);
				if (real_normals) {
					mesh.set_normals({
						positions[i],
						positions[next],
						positions[next],
						positions[i]
					});
				}
				cylinder.meshes.insert(mesh);
			}

			cylinder.meshes.compress();
			return cylinder;
		}

		static GraphObject cone(size_t count_points, bool real_normals, size_t max_count_models) {
			if (count_points < 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "cone, the number of points is less than three.\n\n");
			}

			GraphObject cone(max_count_models);

			std::vector<Vec3> positions;
			std::vector<Vec3> normals;
			for (size_t i = 0; i < count_points; ++i) {
				positions.push_back(Vec3(cos((2.0 * PI / count_points) * i), 0.0, sin((2.0 * PI / count_points) * i)));
				normals.push_back((positions.back().horizont() ^ (Vec3(0.0, 1.0, 0.0) - positions.back())).normalize());
			}

			Mesh mesh(count_points);
			mesh.set_positions(positions, true);
			mesh.invert_points_order(true);
			cone.meshes.insert(mesh);

			for (size_t i = 0; i < count_points; ++i) {
				size_t next = (i + 1) % count_points;

				mesh = Mesh(3);
				mesh.set_positions({
					positions[i],
					positions[next],
					Vec3(0.0, 1.0, 0.0)
				}, !real_normals);
				if (real_normals) {
					mesh.set_normals({
						normals[i],
						normals[next],
						(normals[next] + normals[i]).normalize()
					});
				}
				cone.meshes.insert(mesh);
			}

			cone.meshes.compress();
			return cone;
		}

		static GraphObject sphere(size_t count_points, bool real_normals, size_t max_count_models) {
			if (count_points < 3) {
				throw GreInvalidArgument(__FILE__, __LINE__, "sphere, the number of points is less than three.\n\n");
			}

			GraphObject sphere(max_count_models);
			std::vector<Vec3> last_positions(2 * count_points, Vec3(0.0, 1.0, 0.0));
			for (size_t i = 0; i < count_points; ++i) {
				std::vector<Vec3> current_positions(2 * count_points);

				double vertical_coefficient = (PI / count_points) * (i + 1);
				for (size_t j = 0; j < 2 * count_points; ++j) {
					double horizontal_coefficient = (PI / count_points) * j;
					current_positions[j] = Vec3(cos(horizontal_coefficient) * sin(vertical_coefficient), cos(vertical_coefficient), sin(horizontal_coefficient) * sin(vertical_coefficient));
				}

				for (size_t j = 0; j < 2 * count_points; ++j) {
					size_t next = (j + 1) % (2 * count_points);

					std::vector<Vec3> positions;
					if (i == 0) {
						positions = { Vec3(0.0, 1.0, 0.0), current_positions[j], current_positions[next] };
					} else if (i == count_points - 1) {
						positions = { last_positions[j], Vec3(0.0, -1.0, 0.0), last_positions[next] };
					} else {
						positions = { last_positions[next], last_positions[j], current_positions[j], current_positions[next] };
					}

					Mesh mesh(positions.size());
					mesh.set_positions(positions, !real_normals);
					if (real_normals) {
						mesh.set_normals(positions);
					}
					sphere.meshes.insert(mesh);
				}
				last_positions = current_positions;
			}

			sphere.meshes.compress();
			return sphere;
		}
	};
}
