#pragma once

#include "CamerasStorage.h"
#include "DefaultControlSystem.h"
#include "GraphObjectStorage.h"
#include "LightStorage.h"
#include "../GraphicClasses/Kernel.h"


namespace gre {
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

		inline static GLuint screen_vertex_array_ = 0;

		GLuint screen_texture_id_ = 0;
		GLuint depth_stencil_texture_id_ = 0;
		GLuint primary_frame_buffer_ = 0;

		bool grayscale_ = false;
		uint32_t border_width_ = 7;
		double gamma_ = 2.2;
		Vec3 border_color_ = Vec3(1.0, 0.0, 0.0);
		Vec3 clear_color_ = Vec3(0.0);
		Kernel kernel_ = Kernel();

		Shader<size_t> main_shader_;
		Shader<size_t> depth_shader_;
		Shader<size_t> post_shader_;
		sf::RenderWindow* window_;
		
		void set_active() const {
			if (!window_->setActive(true)) {
				throw GreRuntimeError(__FILE__, __LINE__, "set_active, failed to activate window.\n\n");
			}
		}

		void set_uniforms() const {
			main_shader_.set_uniform_i("diffuse_map", 0);
			main_shader_.set_uniform_i("specular_map", 1);
			main_shader_.set_uniform_i("emission_map", 2);
			main_shader_.set_uniform_i("shadow_maps", 3);
			main_shader_.set_uniform_f("gamma", static_cast<GLfloat>(gamma_));

			post_shader_.set_uniform_i("screen_texture", 0);
			post_shader_.set_uniform_i("stencil_texture", 1);
			post_shader_.set_uniform_i("grayscale", grayscale_);
			post_shader_.set_uniform_i("border_width", border_width_);
			post_shader_.set_uniform_f("border_color", border_color_);
			kernel_.set_uniforms(post_shader_);
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
				throw GreRuntimeError(__FILE__, __LINE__, "create_primary_frame_buffer, framebuffer is not complete.\n\n");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_objects(const Camera& camera) const {
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
			lights.set_framebuffer();

			for (const auto& [light_id, light] : lights) {
				lights.set_depth_map_texture(light_id);

				depth_shader_.set_uniform_matrix("light_space", light->get_light_space_matrix());
				for (const auto& [object_id, object] : objects) {
					object.draw_depth_map();
				}
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_primary_frame_buffer(const Camera& camera) const {
			glBindFramebuffer(GL_FRAMEBUFFER, primary_frame_buffer_);
			camera.set_uniforms(main_shader_);
			
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D_ARRAY, lights.depth_map_texture_id_);
			glActiveTexture(GL_TEXTURE0);

			draw_objects(camera);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glActiveTexture(GL_TEXTURE0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_mainbuffer(const Camera& camera) const {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			camera.set_viewport(post_shader_);

			glDisable(GL_DEPTH_TEST);

			glBindVertexArray(screen_vertex_array_);

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
			glDeleteTextures(1, &screen_texture_id_);
			glDeleteTextures(1, &depth_stencil_texture_id_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			primary_frame_buffer_ = 0;
			screen_texture_id_ = 0;
			depth_stencil_texture_id_ = 0;
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
		inline static DefaultControlSystem default_control_system;

		GraphObjectStorage objects;
		LightStorage lights;
		CamerasStorage cameras;

		explicit GraphEngine(sf::RenderWindow* window) {
			window_ = window;
			set_active();

			if (!glew_is_ok()) {
				throw GreRuntimeError(__FILE__, __LINE__, "GraphEngine, failed to initialize GLEW.\n\n");
			}

			depth_shader_ = gre::Shader<size_t>("GraphEngine/Shaders/Vertex/Depth", "GraphEngine/Shaders/Fragment/Depth", gre::ShaderType::DEPTH);
			post_shader_ = gre::Shader<size_t>("GraphEngine/Shaders/Vertex/Post", "GraphEngine/Shaders/Fragment/Post", gre::ShaderType::POST);
			main_shader_ = gre::Shader<size_t>("GraphEngine/Shaders/Vertex/Main", "GraphEngine/Shaders/Fragment/Main", gre::ShaderType::MAIN);
			
			const sf::ContextSettings& settings = window->getSettings();
			if (!depth_shader_.check_window_settings(settings) || !post_shader_.check_window_settings(settings) || !main_shader_.check_window_settings(settings)) {
				throw GreRuntimeError(__FILE__, __LINE__, "GraphEngine, invalid OpenGL version.\n\n");
			}
			set_uniforms();

			cameras.insert(Camera(window, &default_control_system));

			init_gl();
			lights.create_depth_map_frame_buffer(std::stoi(main_shader_.get_value_frag("NR_LIGHTS")));
			cameras.create_shader_storage_buffer(std::stoi(main_shader_.get_value_frag("NR_CAMERAS")), main_shader_);
			create_screen_vertex_array();
			create_primary_frame_buffer();
		}

		GraphEngine(const GraphEngine& other) {
			window_ = other.window_;
			set_active();

			grayscale_ = other.grayscale_;
			border_width_ = other.border_width_;
			gamma_ = other.gamma_;
			border_color_ = other.border_color_;
			clear_color_ = other.clear_color_;
			kernel_ = other.kernel_;

			objects = other.objects;
			lights = other.lights;
			cameras = other.cameras;

			main_shader_ = other.main_shader_;
			depth_shader_ = other.depth_shader_;
			post_shader_ = other.post_shader_;
			set_uniforms();

			init_gl();
			create_screen_vertex_array();
			create_primary_frame_buffer();
		}

		GraphEngine(GraphEngine&& other) noexcept {
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
			if (less_equality(gamma, 0.0)) {
				throw GreInvalidArgument(__FILE__, __LINE__, "set_gamma, not positive gamma.\n\n");
			}

			set_active();
			post_shader_.set_uniform_f("gamma", static_cast<GLfloat>(gamma));
			gamma_ = gamma;
			return *this;
		}

		void set_border_color(double red, double green, double blue) {
			set_border_color(Vec3(red, green, blue));
		}

		GraphEngine& set_border_color(const Vec3& color) {
			check_color_value(__FILE__, __LINE__, __func__, color);

			set_active();
			post_shader_.set_uniform_f("border_color", color);
			border_color_ = color;
			return *this;
		}

		void set_clear_color(double red, double green, double blue) {
			set_clear_color(Vec3(red, green, blue));
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

		bool get_grayscale() const noexcept {
			return grayscale_;
		}

		uint32_t get_border_width() const noexcept {
			return border_width_;
		}

		double get_gamma() const noexcept {
			return gamma_;
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

		ObjectDesc get_check_object(size_t camera_id, Vec3& intersect_point) {
			if (!cameras.contains(camera_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_check_object, invalid camera id.\n\n");
			}

			ObjectDesc result = cameras.get_check_object(camera_id, intersect_point);

			result.exist = result.object_id >= 0 && result.model_id >= 0 && objects.contains(result.object_id) && objects[result.object_id].models.contains_memory(result.model_id);
			return result;
		}

		ObjectDesc get_check_object(size_t camera_id) {
			if (!cameras.contains(camera_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_check_object, invalid camera id.\n\n");
			}

			ObjectDesc result = cameras.get_check_object(camera_id);

			result.exist = result.object_id >= 0 && result.model_id >= 0 && objects.contains(result.object_id) && objects[result.object_id].models.contains_memory(result.model_id);
			return result;
		}

		void swap(GraphEngine& other) {
			std::swap(window_, other.window_);
			set_active();

			std::swap(grayscale_, other.grayscale_);
			std::swap(border_width_, other.border_width_);
			std::swap(gamma_, other.gamma_);
			std::swap(border_color_, other.border_color_);
			std::swap(clear_color_, other.clear_color_);
			std::swap(kernel_, other.kernel_);

			objects.swap(other.objects);
			lights.swap(other.lights);
			cameras.swap(other.cameras);

			main_shader_.swap(other.main_shader_);
			depth_shader_.swap(other.depth_shader_);
			post_shader_.swap(other.post_shader_);
			set_uniforms();

			std::swap(screen_texture_id_, other.screen_texture_id_);
			std::swap(depth_stencil_texture_id_, other.depth_stencil_texture_id_);
			std::swap(primary_frame_buffer_, other.primary_frame_buffer_);
			init_gl();
		}

		void draw() {
			set_active();

			draw_depth_map();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			check_gl_errors(__FILE__, __LINE__, __func__);

			cameras.update_storage();
			lights.set_uniforms(main_shader_);
			for (const auto& [id, camera] : cameras) {
				main_shader_.set_uniform_i("camera_id", cameras.get_memory_id(id));

				draw_primary_frame_buffer(camera);
				draw_mainbuffer(camera);
			}
		}

		~GraphEngine() {
			deallocate();
		}
	};
}
