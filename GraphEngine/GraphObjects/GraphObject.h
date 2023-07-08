#pragma once

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <unordered_map>
#include <unordered_set>
#include "MeshStorage.h"
#include "ModelStorage.h"


namespace gre {
    class GraphObject {
        Texture load_texture_by_type(const aiMaterial* material, aiTextureType type, const aiScene* scene, const std::string& directory, std::unordered_map<std::string, Texture>& uploaded_textures) {
            aiString texture_path;
            aiTextureMapping mapping;
            unsigned int uv_index;

            aiReturn result = material->GetTexture(type, 0, &texture_path, &mapping, &uv_index);
            if (result != aiReturn_SUCCESS) {
#ifdef _DEBUG
                std::cout << "Material loading warning, unable to load texture parameters. Error code: " << result << "\n\n";
#endif // _DEBUG
                return Texture();
            }

#ifdef _DEBUG
            if (mapping != aiTextureMapping_UV) {
                std::cout << "Material loading warning, unable to implement not UV texture mapping.\n\n";
            }
            if (uv_index > 0) {
                std::cout << "Material loading warning, unable to use more than one UV channel.\n\n";
            }
#endif // _DEBUG

            const std::string& path(texture_path.data);
            if (uploaded_textures.contains(path)) {
                return uploaded_textures[path];
            }

            Texture result_texture;
            if (path[0] == '*') {
                const aiTexture* texture = scene->mTextures[std::stoi(path.substr(1, path.size() - 1))];
                result_texture.load_from_memory(reinterpret_cast<void*>(texture->pcData), std::max(texture->mHeight, 1u) * texture->mWidth, true);
            }
            else {
                result_texture.load_from_file(directory + "/" + path, true);
            }
            
            return result_texture;
        }

        void load_material_textures(Material& mesh_material, const aiMaterial* material, const aiScene* scene, const std::string& directory, std::unordered_map<std::string, Texture>& uploaded_textures) {
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
#ifdef _DEBUG
                if (material->GetTextureCount(aiTextureType_DIFFUSE) > 1) {
                    std::cout << "Material loading warning, unable to load all diffuse textures.\n\n";
                }
#endif // _DEBUG

                mesh_material.diffuse_map = load_texture_by_type(material, aiTextureType_DIFFUSE, scene, directory, uploaded_textures);
            }

            if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
#ifdef _DEBUG
                if (material->GetTextureCount(aiTextureType_SPECULAR) > 1) {
                    std::cout << "Material loading warning, unable to load all specular textures.\n\n";
                }
#endif // _DEBUG

                mesh_material.specular_map = load_texture_by_type(material, aiTextureType_SPECULAR, scene, directory, uploaded_textures);
            }

            if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
#ifdef _DEBUG
                if (material->GetTextureCount(aiTextureType_EMISSIVE) > 1) {
                    std::cout << "Material loading warning, unable to load all emissive textures.\n\n";
                }
#endif // _DEBUG

                mesh_material.emission_map = load_texture_by_type(material, aiTextureType_EMISSIVE, scene, directory, uploaded_textures);
            }

#ifdef _DEBUG
            if (material->GetTextureCount(aiTextureType_AMBIENT) > 0) {
                std::cout << "Material loading warning, unable to load ambient textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_HEIGHT) > 0) {
                std::cout << "Material loading warning, unable to load height textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
                std::cout << "Material loading warning, unable to load normals textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_SHININESS) > 0) {
                std::cout << "Material loading warning, unable to load shininess textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_OPACITY) > 0) {
                std::cout << "Material loading warning, unable to load opacity textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0) {
                std::cout << "Material loading warning, unable to load displacement textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
                std::cout << "Material loading warning, unable to load lightmap textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_REFLECTION) > 0) {
                std::cout << "Material loading warning, unable to load reflection textures.\n\n";
            }
            if (material->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
                std::cout << "Material loading warning, there are unknown textures.\n\n";
            }
#endif // _DEBUG
        }

        Material load_material_data(const aiMaterial* material, const aiScene* scene, const std::string& directory, std::unordered_map<std::string, Texture>& uploaded_textures) {
            Material mesh_material;

            aiColor3D color(0.0, 0.0, 0.0);
            aiReturn result = material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_ambient(color.r, color.g, color.b);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load ambient color. Error code: " << result << "\n\n";
            }
#endif // _DEBUG
            
            color = aiColor3D(0.0, 0.0, 0.0);
            result = material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_diffuse(color.r, color.g, color.b);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load diffuse color. Error code: " << result << "\n\n";
            }
#endif // _DEBUG
            
            color = aiColor3D(0.0, 0.0, 0.0);
            result = material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_specular(color.r, color.g, color.b);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load specular color. Error code: " << result << "\n\n";
            }
#endif // _DEBUG
            
            color = aiColor3D(0.0, 0.0, 0.0);
            result = material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_emission(color.r, color.g, color.b);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load emissive color. Error code: " << result << "\n\n";
            }
#endif // _DEBUG
            
            float opacity;
            result = material->Get(AI_MATKEY_OPACITY, opacity);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_alpha(opacity);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load opacity. Error code: " << result << "\n\n";
            }
#endif // _DEBUG

            float shininess;
            result = material->Get(AI_MATKEY_OPACITY, shininess);
            if (result == aiReturn_SUCCESS) {
                mesh_material.set_shininess(shininess);
            }
#ifdef _DEBUG
            else {
                std::cout << "Material loading warning, unable to load shininess. Error code: " << result << "\n\n";
            }
#endif // _DEBUG

            load_material_textures(mesh_material, material, scene, directory, uploaded_textures);

            return mesh_material;
        }

        Mesh load_mesh_data(const aiMesh* mesh) {
#ifdef _DEBUG
            if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT) {
                std::cout << "Mesh loading warning, unable to load point primitive type.\n\n";
            }
            if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) {
                std::cout << "Mesh loading warning, unable to load line primitive type.\n\n";
            }
            if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) {
                std::cout << "Mesh loading warning, unable to load polygon primitive type.\n\n";
            }
#endif // _DEBUG

            Mesh polygon_mesh(mesh->mNumVertices);

            // Loading index array of mesh
            std::vector<GLuint> indices;
            indices.reserve(3ull * mesh->mNumFaces);
            for (size_t face_id = 0; face_id < mesh->mNumFaces; ++face_id) {
#ifdef _DEBUG
                if (mesh->mFaces[face_id].mNumIndices != 3) {
                    std::cout << "Mesh loading warning, unable to load not triangle face.\n\n";
                }
#endif // _DEBUG

                for (size_t i = 0; i < mesh->mFaces[face_id].mNumIndices; ++i) {
                    indices.push_back(static_cast<GLuint>(mesh->mFaces[face_id].mIndices[i]));
                }
            }
            polygon_mesh.set_indices(indices);

            // Loading vertex positions
            std::vector<Vec3> positions;
            positions.reserve(mesh->mNumVertices);
            for (size_t i = 0; i < mesh->mNumVertices; ++i) {
                positions.emplace_back(mesh->mVertices[i]);
            }
            polygon_mesh.set_positions(positions, !mesh->HasNormals());

            // Loading normals of vertexes
            if (mesh->HasNormals()) {
                std::vector<Vec3> normals;
                normals.reserve(mesh->mNumVertices);
                for (size_t i = 0; i < mesh->mNumVertices; ++i) {
                    normals.emplace_back(mesh->mNormals[i]);
                }
                polygon_mesh.set_normals(normals);
            }

            // Loading texture coordinates of vertexes
            if (mesh->GetNumUVChannels() > 0) {
#ifdef _DEBUG
                if (mesh->GetNumUVChannels() > 1) {
                    std::cout << "Mesh loading warning, unable to load all texture coordinates.\n\n";
                }
                if (mesh->mNumUVComponents[0] != 2) {
                    std::cout << "Mesh loading warning, unable to load not 2D texture coordinates.\n\n";
                }
#endif // _DEBUG

                std::vector<Vec2> tex_coords;
                tex_coords.reserve(mesh->mNumVertices);
                for (size_t i = 0; i < mesh->mNumVertices; ++i) {
                    tex_coords.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
                }
                polygon_mesh.set_tex_coords(tex_coords);
            }

            // Loading colors of vertexes
            if (mesh->GetNumColorChannels() > 0) {
#ifdef _DEBUG
                if (mesh->GetNumColorChannels() > 1) {
                    std::cout << "Mesh loading warning, unable to load all texture colors.\n\n";
                }
#endif // _DEBUG

                std::vector<Vec3> colors;
                colors.reserve(mesh->mNumVertices);
                for (size_t i = 0; i < mesh->mNumVertices; ++i) {
#ifdef _DEBUG
                    if (mesh->mColors[0][i].a != 1.0) {
                        std::cout << "Mesh loading warning, unable to load alpha component of vertex color.\n\n";
                    }
#endif // _DEBUG
                    colors.push_back(Vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b));
                }
                polygon_mesh.set_colors(colors);
            }

            return polygon_mesh;
        }

        void process_node(const aiNode* node, Matrix4x4 transform, const std::vector<Mesh>& scene_meshes) {
            transform = Matrix4x4(node->mTransformation) * transform;

            for (size_t i = 0; i < node->mNumMeshes; ++i) {
                Mesh mesh = scene_meshes[node->mMeshes[i]];
                mesh.apply_matrix(transform);
                meshes.insert(mesh);
            }

            for (size_t i = 0; i < node->mNumChildren; ++i) {
                process_node(node->mChildren[i], transform, scene_meshes);
            }
        }

        // MAIN shader expected
        void draw_meshes(size_t model_id, const Shader& shader) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "draw_meshes, invalid model id.\n\n");
            }
#endif // _DEBUG

            shader.set_uniform_i("model_id", static_cast<GLint>(models.get_memory_id(model_id)));
            shader.set_uniform_matrix("not_instance_model", models[model_id]);

            for (const auto& [id, mesh] : meshes) {
                mesh.draw(1, shader);
            }
        }

        // MAIN shader expected
        void draw_meshes(const Shader& shader) const {
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
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid model id.\n\n");
            }
            if (!meshes.contains(mesh_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_positions, invalid mesh id.\n\n");
            }
#endif // _DEBUG

            const Matrix4x4& transform = models[model_id];
            std::vector<Vec3> positions = meshes[mesh_id].get_positions();
            for (Vec3& position : positions) {
                position = transform * position;
            }
            return positions;
        }

        std::vector<Vec3> get_mesh_normals(size_t model_id, size_t mesh_id) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid model id.\n\n");
            }
            if (!meshes.contains(mesh_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_normals, invalid mesh id.\n\n");
            }
#endif // _DEBUG

            const Matrix4x4& transform = Matrix4x4::normal_transform(models[model_id]);
            std::vector<Vec3> normals = meshes[mesh_id].get_normals();
            for (Vec3& normal : normals) {
                normal = transform * normal;
            }
            return normals;
        }

        Vec3 get_mesh_center(size_t model_id, size_t mesh_id) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid model id.\n\n");
            }
            if (!meshes.contains(mesh_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_mesh_center, invalid mesh id.\n\n");
            }
#endif // _DEBUG

            return models[model_id] * meshes[mesh_id].get_center();
        }

        Vec3 get_center(size_t model_id) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "get_center, invalid model id.\n\n");
            }
            if (meshes.size() == 0) {
                throw GreDomainError(__FILE__, __LINE__, "get_center, object does not contain vertices.\n\n");
            }
#endif // _DEBUG

            Vec3 center(0, 0, 0);
            std::unordered_set<Vec3> used_positions;
            for (const auto& [id, mesh] : meshes) {
                for (const Vec3& position : mesh.get_positions()) {
                    if (used_positions.contains(position)) {
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

        void load_from_file(const std::string& path) {
            meshes.clear();

            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_MakeLeftHanded | aiProcess_Triangulate);

#ifdef _DEBUG
            if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == NULL) {
                throw GreRuntimeError(__FILE__, __LINE__, "load_from_file, failed to load model, description \\/\n" + std::string(importer.GetErrorString()) + "\n\n");
            }
#endif // _DEBUG

            // Loading all materials
            std::unordered_map<std::string, Texture> uploaded_textures;
            const std::string& directory = path.substr(0, path.find_last_of('/'));
            std::vector<Material> materials;
            materials.reserve(scene->mNumMaterials);
            for (size_t i = 0; i < scene->mNumMaterials; ++i) {
                materials.push_back(load_material_data(scene->mMaterials[i], scene, directory, uploaded_textures));
            }

            // Loading all meshes
            std::vector<Mesh> scene_meshes;
            scene_meshes.reserve(scene->mNumMeshes);
            for (size_t i = 0; i < scene->mNumMeshes; ++i) {
                scene_meshes.push_back(load_mesh_data(scene->mMeshes[i]));
                scene_meshes.back().material = materials[scene->mMeshes[i]->mMaterialIndex];
                scene_meshes.back().material.use_vertex_color = scene->mMeshes[i]->GetNumColorChannels() > 0;
            }

            process_node(scene->mRootNode, Matrix4x4::one_matrix(), scene_meshes);
        }

        void draw_depth_map() const {
            for (const auto& [id, mesh] : meshes) {
                if (!mesh.material.shadow) {
                    continue;
                }

                mesh.draw(models.size(), Shader());
            }
        }

        // MAIN shader expected
        void draw(size_t model_id, size_t mesh_id, const Shader& shader) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
            }
            if (!meshes.contains(mesh_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid mesh id.\n\n");
            }
#endif // _DEBUG

            shader.set_uniform_i("model_id", static_cast<GLint>(models.get_memory_id(model_id)));
            shader.set_uniform_matrix("not_instance_model", models[model_id]);

            if (border_mask > 0) {
                glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
                glStencilMask(border_mask);
            }

            meshes[mesh_id].draw(1, shader);

            if (border_mask > 0) {
                glStencilMask(0x00);
#ifdef _DEBUG
                check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
            }
        }

        // MAIN shader expected
        void draw(size_t model_id, const Shader& shader) const {
#ifdef _DEBUG
            if (!models.contains(model_id)) {
                throw GreOutOfRange(__FILE__, __LINE__, "draw, invalid model id.\n\n");
            }
#endif // _DEBUG

            if (border_mask > 0) {
                glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
                glStencilMask(border_mask);
            }

            draw_meshes(model_id, shader);

            if (border_mask > 0) {
                glStencilMask(0x00);
#ifdef _DEBUG
                check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
            }
        }

        // MAIN shader expected
        void draw(const Shader& shader) const {
            if (border_mask > 0) {
                glStencilFunc(GL_ALWAYS, border_mask, 0xFF);
                glStencilMask(border_mask);
            }

            draw_meshes(shader);

            if (border_mask > 0) {
                glStencilMask(0x00);
#ifdef _DEBUG
                check_gl_errors(__FILE__, __LINE__, __func__);
#endif // _DEBUG
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
#ifdef _DEBUG
            if (count_points < 3) {
                throw GreInvalidArgument(__FILE__, __LINE__, "cylinder, the number of points is less than three.\n\n");
            }
#endif // _DEBUG

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
#ifdef _DEBUG
            if (count_points < 3) {
                throw GreInvalidArgument(__FILE__, __LINE__, "cone, the number of points is less than three.\n\n");
            }
#endif // _DEBUG

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
#ifdef _DEBUG
            if (count_points < 3) {
                throw GreInvalidArgument(__FILE__, __LINE__, "sphere, the number of points is less than three.\n\n");
            }
#endif // _DEBUG

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
                    }
                    else if (i == count_points - 1) {
                        positions = { last_positions[j], Vec3(0.0, -1.0, 0.0), last_positions[next] };
                    }
                    else {
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
