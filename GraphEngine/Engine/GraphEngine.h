#pragma once

#include "Camera.h"
#include "GraphObjectStorage.h"
#include "../GraphicClasses/Kernel.h"
#include "../Light/Light.h"


namespace eng {
	class GraphEngine {
		class TransparentObject {
			double distance_;

		public:
			size_t object_id;
			size_t model_id;
			const GraphObject* object;

			TransparentObject(const Vec3& camera_position, const GraphObject* object, size_t object_id, size_t model_id) noexcept {
				this->object_id = object_id;
				this->model_id = model_id;
				this->object = object;
				distance_ = (camera_position - object->get_center(model_id)).length();
			}

			bool operator<(const TransparentObject& other) const noexcept {
				return distance_ < other.distance_;
			}
		};

		inline static double eps_ = 1e-5;
		inline static GLuint screen_vertex_array_ = 0;

		GLuint screen_texture_id_ = 0;
		GLuint depth_stencil_texture_id_ = 0;
		GLuint primary_frame_buffer_ = 0;
		GLuint depth_map_texture_id_ = 0;
		GLuint depth_map_frame_buffer_ = 0;
		GLuint shader_storage_buffer_ = 0;

		bool grayscale_ = false;
		uint32_t border_width_ = 7; 
		double gamma_ = 2.2;
		Vec2 check_point_ = Vec2(0.0);
		Vec3 border_color_ = Vec3(0.0);
		Vec3 clear_color_ = Vec3(0.0);
		Kernel kernel_ = Kernel();

		size_t shadow_width_ = 1024;
		size_t shadow_height_ = 1024;

		std::vector<Light*> lights_;
		Shader<size_t> main_shader_;
		Shader<size_t> depth_shader_;
		Shader<size_t> post_shader_;
		sf::RenderWindow* window_;
		
		void set_active() const {
			if (!window_->setActive(true)) {
				throw EngRuntimeError(__FILE__, __LINE__, "set_active, failed to activate window.\n\n");
			}
		}

		void set_uniforms() const {
			main_shader_.set_uniform_i("diffuse_map", 0);
			main_shader_.set_uniform_i("specular_map", 1);
			main_shader_.set_uniform_i("emission_map", 2);
			main_shader_.set_uniform_f("gamma", static_cast<GLfloat>(gamma_));
			main_shader_.set_uniform_f("check_point", static_cast<GLfloat>(check_point_.x * window_->getSize().x), static_cast<GLfloat>(check_point_.y * window_->getSize().y));

			post_shader_.set_uniform_i("screen_texture", 0);
			post_shader_.set_uniform_i("stencil_texture", 1);
			post_shader_.set_uniform_i("grayscale", grayscale_);
			post_shader_.set_uniform_i("border_width", border_width_);
			post_shader_.set_uniform_f("border_color", border_color_);
			kernel_.set_uniforms(post_shader_);
		}

		void set_light_uniforms() const {
			for (size_t i = 0; i < lights_.size(); ++i) {
				if (lights_[i] == nullptr) {
					Light::set_exist(false, i, main_shader_);
					continue;
				}

				lights_[i]->set_uniforms(i, main_shader_);
			}
		}

		void init_gl() const {
			glClearColor(static_cast<GLclampf>(clear_color_.x), static_cast<GLclampf>(clear_color_.y), static_cast<GLclampf>(clear_color_.z), static_cast<GLclampf>(1.0));
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_CULL_FACE);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_primary_frame_buffer() {
			glGenTextures(1, &screen_texture_id_);
			glBindTexture(GL_TEXTURE_2D, screen_texture_id_);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_->getSize().x, window_->getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &depth_stencil_texture_id_);
			glBindTexture(GL_TEXTURE_2D, depth_stencil_texture_id_);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, window_->getSize().x, window_->getSize().y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat border_ñolor[] = { 0.0, 0.0, 0.0, 0.0 };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_ñolor);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenFramebuffers(1, &primary_frame_buffer_);
			glBindFramebuffer(GL_FRAMEBUFFER, primary_frame_buffer_);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture_id_, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil_texture_id_, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				throw EngRuntimeError(__FILE__, __LINE__, "create_primary_frame_buffer, framebuffer is not complete.\n\n");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_depth_map_frame_buffer() {
			if (lights_.empty()) {
				return;
			}

			glGenTextures(1, &depth_map_texture_id_);
			glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_texture_id_);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(shadow_width_), static_cast<GLsizei>(shadow_height_), static_cast<GLsizei>(lights_.size()), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat border_ñolor[] = { 1.0, 1.0, 1.0, 1.0 };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_ñolor);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			glGenFramebuffers(1, &depth_map_frame_buffer_);
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer_);

			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map_texture_id_, 0, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				throw EngRuntimeError(__FILE__, __LINE__, "create_depth_map_frame_buffer, framebuffer is not complete.\n\n");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_shader_storage_buffer() {
			glGenBuffers(1, &shader_storage_buffer_);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			GLint init_int[2] = { -1, -1 };
			GLfloat init_float = 1;
			glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) + sizeof(GLfloat), &init_int, GL_DYNAMIC_READ);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint), sizeof(GLfloat), &init_float);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_buffers() {
			create_screen_vertex_array();
			create_primary_frame_buffer();
			create_depth_map_frame_buffer();
			create_shader_storage_buffer();
		}

		void draw_objects() const {
			std::vector<TransparentObject> transparent_objects;
			for (const auto& [object_id, object] : objects) {
				if (object.transparent) {
					for (const auto& [model_id, model] : object.models) {
						transparent_objects.emplace_back(camera.position, &object, object_id, model_id);
					}
					continue;
				}

				main_shader_.set_uniform_i("object_id", static_cast<GLint>(object_id));
				object.draw(main_shader_);
			}

			std::sort(transparent_objects.rbegin(), transparent_objects.rend());
			for (const TransparentObject& object : transparent_objects) {
				main_shader_.set_uniform_i("object_id", static_cast<GLint>(object.object_id));
				object.object->draw(object.model_id, main_shader_);
			}
		}

		void draw_depth_map() const {
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer_);
			glViewport(0, 0, static_cast<GLsizei>(shadow_width_), static_cast<GLsizei>(shadow_height_));

			glClear(GL_DEPTH_BUFFER_BIT);

			for (size_t i = 0; i < lights_.size(); ++i) {
				if (lights_[i] == nullptr || !lights_[i]->shadow) {
					continue;
				}

				depth_shader_.set_uniform_matrix("light_space", lights_[i]->get_light_space_matrix());
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, static_cast<GLint>(depth_map_texture_id_), 0, static_cast<GLint>(i));
				for (const auto& [id, object] : objects) {
					object.draw_depth_map();
				}
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_primary_frame_buffer() const {
			glBindFramebuffer(GL_FRAMEBUFFER, primary_frame_buffer_);
			glViewport(0, 0, static_cast<GLsizei>(window_->getSize().x), static_cast<GLsizei>(window_->getSize().y));

			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			set_light_uniforms();
			main_shader_.set_uniform_f("view_pos", camera.position);
			main_shader_.set_uniform_matrix("view", camera.get_view_matrix());

			glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_texture_id_);
			draw_objects();
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_mainbuffer() const {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, static_cast<GLsizei>(window_->getSize().x), static_cast<GLsizei>(window_->getSize().y));

			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);

			glBindVertexArray(screen_vertex_array_);

			post_shader_.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, screen_texture_id_);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depth_stencil_texture_id_);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindVertexArray(0);

			glEnable(GL_DEPTH_TEST);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void deallocate() {
			glDeleteFramebuffers(1, &primary_frame_buffer_);
			glDeleteFramebuffers(1, &depth_map_frame_buffer_);
			glDeleteBuffers(1, &shader_storage_buffer_);
			glDeleteTextures(1, &screen_texture_id_);
			glDeleteTextures(1, &depth_stencil_texture_id_);
			glDeleteTextures(1, &depth_map_texture_id_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			primary_frame_buffer_ = 0;
			depth_map_frame_buffer_ = 0;
			shader_storage_buffer_ = 0;
			screen_texture_id_ = 0;
			depth_stencil_texture_id_ = 0;
			depth_map_texture_id_ = 0;
		}

		static void create_screen_vertex_array() {
			if (screen_vertex_array_ != 0) {
				return;
			}

			glGenVertexArrays(1, &screen_vertex_array_);
			glBindVertexArray(screen_vertex_array_);

			GLuint vertex_buffer;
			glGenBuffers(1, &vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

			GLfloat vertices[] = {
				 1.0,  1.0, 1.0, 1.0,
				-1.0,  1.0, 0.0, 1.0,
				-1.0, -1.0, 0.0, 0.0,
				-1.0, -1.0, 0.0, 0.0,
				 1.0, -1.0, 1.0, 0.0,
				 1.0,  1.0, 1.0, 1.0
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(0));
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDeleteBuffers(1, &vertex_buffer);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

	public:
		struct ObjectDesc {
			bool exist = false;
			size_t object_id = 0;
			size_t model_id = 0;
		};

		GraphObjectStorage objects;
		Camera camera;

		explicit GraphEngine(sf::RenderWindow* window) : camera(window) {
			window_ = window;
			set_active();

			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "GraphEngine, failed to initialize GLEW.\n\n");
			}

			depth_shader_ = eng::Shader<size_t>("GraphEngine/Shaders/Vertex/Depth", "GraphEngine/Shaders/Fragment/Depth", eng::ShaderType::DEPTH);
			post_shader_ = eng::Shader<size_t>("GraphEngine/Shaders/Vertex/Post", "GraphEngine/Shaders/Fragment/Post", eng::ShaderType::POST);
			main_shader_ = eng::Shader<size_t>("GraphEngine/Shaders/Vertex/Main", "GraphEngine/Shaders/Fragment/Main", eng::ShaderType::MAIN);
			set_uniforms();
			
			const sf::ContextSettings& settings = window->getSettings();
			if (!depth_shader_.check_window_settings(settings) || !post_shader_.check_window_settings(settings) || !main_shader_.check_window_settings(settings)) {
				throw EngRuntimeError(__FILE__, __LINE__, "GraphEngine, invalid OpenGL version.\n\n");
			}

			lights_.resize(std::stoi(main_shader_.get_value_frag("NR_LIGHTS")), nullptr);

			camera.set_screen_ratio(static_cast<double>(window->getSize().x) / static_cast<double>(window->getSize().y));

			init_gl();
			create_buffers();
		}

		GraphEngine(const GraphEngine& other) : camera(other.window_) {
			window_ = other.window_;
			set_active();

			shadow_width_ = other.shadow_width_;
			shadow_height_ = other.shadow_height_;
			grayscale_ = other.grayscale_;
			border_width_ = other.border_width_;
			gamma_ = other.gamma_;
			check_point_ = other.check_point_;
			border_color_ = other.border_color_;
			clear_color_ = other.clear_color_;
			lights_ = other.lights_;
			kernel_ = other.kernel_;
			objects = other.objects;
			camera = other.camera;

			main_shader_ = other.main_shader_;
			depth_shader_ = other.depth_shader_;
			post_shader_ = other.post_shader_;
			set_uniforms();

			init_gl();
			create_buffers();
		}

		GraphEngine(GraphEngine&& other) noexcept : camera(other.window_) {
			swap(other);
		}

		GraphEngine& operator=(const GraphEngine& other)& {
			GraphEngine object(other);
			swap(object);
			return *this;
		}

		GraphEngine& operator=(GraphEngine&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		GraphEngine& set_grayscale(bool grayscale) {
			set_active();
			post_shader_.set_uniform_i("grayscale", grayscale);
			grayscale_ = grayscale;
			return *this;
		}

		GraphEngine& set_border_width(uint32_t border_width) {
			set_active();
			post_shader_.set_uniform_i("border_width", border_width);
			border_width_ = border_width;
			return *this;
		}

		GraphEngine& set_gamma(double gamma) {
			if (less_equality(gamma, 0.0, eps_)) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_gamma, not positive gamma.\n\n");
			}

			set_active();
			post_shader_.set_uniform_f("gamma", static_cast<GLfloat>(gamma));
			gamma_ = gamma;
			return *this;
		}

		GraphEngine& set_check_point(const Vec2& point) {
			if (point.x < 0.0 || 1.0 < point.x || point.y < 0.0 || 1.0 < point.y) {
				throw EngInvalidArgument(__FILE__, __LINE__, "set_check_point, invalid point coordinate.\n\n");
			}

			set_active();
			check_point_ = Vec2(point.x, 1.0 - point.y);
			main_shader_.set_uniform_f("check_point", static_cast<GLfloat>(check_point_.x * window_->getSize().x), static_cast<GLfloat>(check_point_.y * window_->getSize().y));
			return *this;
		}

		GraphEngine& set_border_color(const Vec3& color) {
			check_color_value(__FILE__, __LINE__, __func__, color);

			set_active();
			post_shader_.set_uniform_f("border_color", color);
			border_color_ = color;
			return *this;
		}

		GraphEngine& set_clear_color(const Vec3& color) {
			check_color_value(__FILE__, __LINE__, __func__, color);

			set_active();
			glClearColor(static_cast<GLclampf>(color.x), static_cast<GLclampf>(color.y), static_cast<GLclampf>(color.z), static_cast<GLclampf>(1.0));
			clear_color_ = color;
			return *this;
		}

		GraphEngine& set_kernel(const Kernel& kernel) {
			set_active();
			kernel.set_uniforms(post_shader_);
			kernel_ = kernel;
			return *this;
		}

		GraphEngine& set_shadow_resolution(size_t width, size_t height) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_texture_id_);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(lights_.size()), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);

			shadow_width_ = width;
			shadow_height_ = height;
			return *this;
		}

		GraphEngine& set_light(size_t light_id, Light* new_light) {
			if (lights_.size() <= light_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "set_light, invalid light id.\n\n");
			}

			lights_[light_id] = new_light;
			return *this;
		}

		bool get_grayscale() const noexcept {
			return grayscale_;
		}

		uint32_t get_border_width() const noexcept {
			return border_width_;
		}

		double get_gamma() const noexcept {
			return gamma_;
		}

		Vec2 get_check_point() const noexcept {
			return check_point_;
		}

		Vec3 get_border_color() const noexcept {
			return border_color_;
		}

		Vec3 get_clear_color() const noexcept {
			return clear_color_;
		}

		Kernel get_kernel() const noexcept {
			return kernel_;
		}

		Light* get_light(size_t light_id) {
			if (lights_.size() <= light_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_light, invalid light id.\n\n");
			}

			return lights_[light_id];
		}

		const Light* get_light(size_t light_id) const {
			if (lights_.size() <= light_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_light, invalid light id.\n\n");
			}

			return lights_[light_id];
		}

		size_t get_count_lights() const noexcept {
			return lights_.size();
		}

		ObjectDesc get_check_object(Vec3& intersect_point) const {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			GLint data_int[2] = { -1, -1 };
			GLfloat distance = 0.0;
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint), data_int);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint), sizeof(GLfloat), &distance);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);

			intersect_point = camera.convert_point(Vec3(2.0 * check_point_ - Vec2(1.0), distance));

			if (data_int[0] < 0 || data_int[1] < 0 || !objects.contains(data_int[0]) || !objects[data_int[0]].models.contains_memory(data_int[1])) {
				return ObjectDesc();
			}
			return { true, static_cast<size_t>(data_int[0]), static_cast<size_t>(data_int[1]) };
		}

		ObjectDesc get_check_object() const {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			GLint data_int[2] = { -1, -1 };
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLint), data_int);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);

			if (data_int[0] < 0 || data_int[1] < 0 || !objects.contains(data_int[0]) || !objects[data_int[0]].models.contains_memory(data_int[1])) {
				return ObjectDesc();
			}
			return { true, static_cast<size_t>(data_int[0]), static_cast<size_t>(data_int[1]) };
		}

		void swap(GraphEngine& other) {
			std::swap(window_, other.window_);
			set_active();

			std::swap(shadow_width_, other.shadow_width_);
			std::swap(shadow_height_, other.shadow_height_);
			std::swap(grayscale_, other.grayscale_);
			std::swap(border_width_, other.border_width_);
			std::swap(gamma_, other.gamma_);
			std::swap(check_point_, other.check_point_);
			std::swap(border_color_, other.border_color_);
			std::swap(clear_color_, other.clear_color_);
			std::swap(lights_, other.lights_);
			std::swap(kernel_, other.kernel_);
			objects.swap(other.objects);
			std::swap(camera, other.camera);

			main_shader_.swap(other.main_shader_);
			depth_shader_.swap(other.depth_shader_);
			post_shader_.swap(other.post_shader_);
			set_uniforms();

			std::swap(screen_texture_id_, other.screen_texture_id_);
			std::swap(depth_stencil_texture_id_, other.depth_stencil_texture_id_);
			std::swap(primary_frame_buffer_, other.primary_frame_buffer_);
			std::swap(depth_map_texture_id_, other.depth_map_texture_id_);
			std::swap(depth_map_frame_buffer_, other.depth_map_frame_buffer_);
			std::swap(shader_storage_buffer_, other.shader_storage_buffer_);
			init_gl();
		}

		void draw() const {
			set_active();

			main_shader_.set_uniform_matrix("projection", camera.get_projection_matrix());

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			GLint init_int = -1;
			GLfloat init_float = 1;

			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint), &init_int);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint), sizeof(GLfloat), &init_float);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
			
			draw_depth_map();
			draw_primary_frame_buffer();
			draw_mainbuffer();
		}

		~GraphEngine() {
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
