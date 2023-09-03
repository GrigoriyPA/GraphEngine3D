#pragma once

#include "../Camera/Camera.hpp"


// Cameras storage implementation
namespace gre {
	// Object under check point of camera
	struct ObjectDescription {
		bool exist = false;
		size_t object_id = 0;
		size_t model_id = 0;
	};

	class CamerasStorage : public AssociativeStorage<Camera> {
		using TBase = AssociativeStorage<Camera>;
		friend class GraphEngine;

		GLuint shader_storage_buffer_ = 0;

		Shader* shader_ = nullptr;
		size_t max_count_cameras_ = 0;

		bool is_actual_ = false;
		GLint* intersect_id_ = nullptr;
		GLfloat* intersect_dist_ = nullptr;

		GLint* init_int_ = nullptr;
		GLfloat* init_float_ = nullptr;

		// Constructors
		CamerasStorage();

		CamerasStorage(const CamerasStorage& other);

		// Fetching check object internal
		ObjectDescription get_check_object(size_t id, Vec3& intersect_point);

		ObjectDescription get_check_object(size_t id);

		void load_buffer_data() noexcept;

		// Drop SSBO value
		void update_storage() noexcept;

		// Private functions
		void swap(CamerasStorage& other) noexcept;

		void deallocate() noexcept override;

		// Uploading into shader

		// MAIN shader expected
		void create_shader_storage_buffer(size_t max_count_cameras, Shader& shader);

	public:
		// Getters
		size_t get_max_count_cameras() const noexcept;

		// Modifications
		size_t insert(Camera value) override;

		// Event handlers (for all cameras)
		void switch_active();

		void compute_event(const sf::Event& event);

		void update();
	};
}  // namespace gre
