#include "CamerasStorage.hpp"


// CamerasStorage
namespace gre {
	// Constructors
	CamerasStorage::CamerasStorage() : TBase() {
	}

	CamerasStorage::CamerasStorage(const CamerasStorage& other) : TBase(other) {
		if (other.max_count_cameras_ > 0) {
			intersect_id_ = new GLint[2 * other.max_count_cameras_];
			intersect_dist_ = new GLfloat[other.max_count_cameras_];
			init_int_ = new GLint[2 * other.max_count_cameras_];
			init_float_ = new GLfloat[other.max_count_cameras_];
			for (size_t i = 0; i < other.max_count_cameras_; ++i) {
				init_int_[2 * i] = -1;
				init_int_[2 * i + 1] = -1;
				init_float_[i] = 1;
			}
		}

		create_shader_storage_buffer(other.max_count_cameras_, *other.shader_);

		glBindBuffer(GL_COPY_READ_BUFFER, other.shader_storage_buffer_);
		glBindBuffer(GL_COPY_WRITE_BUFFER, shader_storage_buffer_);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (2 * sizeof(GLint) + sizeof(GLfloat)) * max_count_cameras_);

		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);

		GRE_CHECK_GL_ERRORS;
	}

	CamerasStorage::CamerasStorage(CamerasStorage&& other) noexcept {
		swap(other);
	}

	CamerasStorage& CamerasStorage::operator=(CamerasStorage other)& {
		swap(other);
		return *this;
	}

	// Getters
	size_t CamerasStorage::get_max_count_cameras() const noexcept {
		return max_count_cameras_;
	}

	// Modifications
	size_t CamerasStorage::insert(Camera value) {
		GRE_ENSURE(size() < max_count_cameras_, GreRuntimeError, "exceeding the limit of " << max_count_cameras_ << " cameras");
		return TBase::insert(value);
	}

	// Event handlers (for all cameras)
	void CamerasStorage::switch_active() {
		for (auto& [id, camera] : *this) {
			camera.switch_active();
		}
	}

	void CamerasStorage::compute_event(const sf::Event& event) {
		for (auto& [id, camera] : *this) {
			camera.compute_event(event);
		}
	}

	void CamerasStorage::update() {
		for (auto& [id, camera] : *this) {
			camera.update();
		}
	}

	// Internal methods

	// Fetching check object internal
	ObjectDescription CamerasStorage::get_check_object(size_t id, Vec3& intersect_point) {
		GRE_ENSURE(contains(id), GreOutOfRange, "invalid camera id");

		load_buffer_data();

		const Camera& camera = (*this)[id];
		size_t memory_id = get_memory_id(id);

		intersect_point = camera.convert_point(intersect_dist_[memory_id]);

		GLint object_id = intersect_id_[memory_id];
		GLint model_id = intersect_id_[memory_id + max_count_cameras_];
		return { .exist = object_id >= 0 && model_id >= 0, .object_id = static_cast<size_t>(object_id), .model_id = static_cast<size_t>(model_id) };
	}

	ObjectDescription CamerasStorage::get_check_object(size_t id) {
		GRE_ENSURE(contains(id), GreOutOfRange, "invalid camera id");

		load_buffer_data();

		size_t memory_id = get_memory_id(id);
		GLint object_id = intersect_id_[memory_id];
		GLint model_id = intersect_id_[memory_id + max_count_cameras_];
		return { .exist = object_id >= 0 && model_id >= 0, .object_id = static_cast<size_t>(object_id), .model_id = static_cast<size_t>(model_id) };
	}

	void CamerasStorage::load_buffer_data() noexcept {
		if (is_actual_) {
			return;
		}
		is_actual_ = true;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint) * max_count_cameras_, intersect_id_);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, intersect_dist_);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GRE_CHECK_GL_ERRORS;
	}

	// Drop SSBO value
	void CamerasStorage::update_storage() noexcept {
		is_actual_ = false;

		shader_->use();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint) * max_count_cameras_, init_int_);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, init_float_);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GRE_CHECK_GL_ERRORS;
	}

	// Private functions
	void CamerasStorage::swap(CamerasStorage& other) noexcept {
		TBase::swap(other);

		std::swap(shader_storage_buffer_, other.shader_storage_buffer_);
		std::swap(is_actual_, other.is_actual_);
		std::swap(intersect_id_, other.intersect_id_);
		std::swap(intersect_dist_, other.intersect_dist_);
		std::swap(init_int_, other.init_int_);
		std::swap(init_float_, other.init_float_);
		std::swap(shader_, other.shader_);
		std::swap(max_count_cameras_, other.max_count_cameras_);
	}

	void CamerasStorage::deallocate() noexcept {
		glDeleteBuffers(1, &shader_storage_buffer_);
		GRE_CHECK_GL_ERRORS;

		delete[] intersect_id_;
		delete[] intersect_dist_;
		delete[] init_int_;
		delete[] init_float_;

		shader_storage_buffer_ = 0;
		intersect_id_ = nullptr;
		intersect_dist_ = nullptr;
		init_int_ = nullptr;
		init_float_ = nullptr;
	}

	// Uploading into shader

	// MAIN shader expected
	void CamerasStorage::create_shader_storage_buffer(size_t max_count_cameras, Shader& shader) {
		shader_ = &shader;
		max_count_cameras_ = max_count_cameras;

		delete[] intersect_id_;
		delete[] intersect_dist_;
		delete[] init_int_;
		delete[] init_float_;
		intersect_id_ = new GLint[2 * max_count_cameras_];
		intersect_dist_ = new GLfloat[max_count_cameras_];
		init_int_ = new GLint[2 * max_count_cameras_];
		init_float_ = new GLfloat[max_count_cameras_];
		for (size_t i = 0; i < max_count_cameras_; ++i) {
			init_int_[2 * i] = -1;
			init_int_[2 * i + 1] = -1;
			init_float_[i] = 1;
		}

		shader.use();
		glGenBuffers(1, &shader_storage_buffer_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

		glBufferData(GL_SHADER_STORAGE_BUFFER, (2 * sizeof(GLint) + sizeof(GLfloat)) * max_count_cameras_, init_int_, GL_DYNAMIC_READ);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * max_count_cameras_, sizeof(GLfloat) * max_count_cameras_, init_float_);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GRE_CHECK_GL_ERRORS;
	}

	CamerasStorage::~CamerasStorage() {
		deallocate();
	}
}  // namespace gre
