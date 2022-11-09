#pragma once

#include <unordered_map>
#include "Camera.h"
#include "../GraphicClasses/Kernel.h"
#include "../GraphObjects/GraphObject.h"
#include "../Light/Light.h"


namespace eng {
	class GraphEngine {
		class TransparentObject {
			double distance_;

		public:
			size_t object_id;
			size_t model_id;
			const GraphObject* object;

			TransparentObject(const Vect3& camera_position, const GraphObject* object, size_t object_id, size_t model_id) noexcept {
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

		GLuint screen_vertex_array_ = 0;
		GLuint screen_texture_id_ = 0;
		GLuint depth_stencil_texture_id_ = 0;
		GLuint primary_frame_buffer_ = 0;
		GLuint depth_map_texture_id_ = 0;
		GLuint depth_map_frame_buffer_ = 0;
		GLuint shader_storage_buffer_ = 0;
		GLuint border_width_ = 7;
		size_t shadow_width_ = 1024;
		size_t shadow_height_ = 1024;
		uint8_t active_state_ = 0;
		double gamma_ = 2.2;
		Vect2 check_point_ = Vect2(0.0, 0.0);
		Vect3 border_color_ = Vect3(0.0, 0.0, 0.0);

		std::vector < Light* > lights_;
		std::unordered_map < int, GraphObject > objects_;
		sf::Vector2i mouse_position_;
		sf::RenderWindow* window_;
		Shader<size_t> main_shader_;
		Shader<size_t> depth_shader_;
		Shader<size_t> post_shader_;

		void set_uniforms() const {
			main_shader_.set_uniform_i("diffuse_map", 0);
			main_shader_.set_uniform_i("specular_map", 1);
			main_shader_.set_uniform_i("emission_map", 2);
			main_shader_.set_uniform_f("gamma", gamma_);
			main_shader_.set_uniform_f("check_point", check_point_);
			main_shader_.set_uniform_matrix("projection", camera.get_projection_matrix());

			post_shader_.set_uniform_i("screen_texture", 0);
			post_shader_.set_uniform_i("stencil_texture", 1);
			post_shader_.set_uniform_i("grayscale", 0);
			post_shader_.set_uniform_i("border_width", border_width_);
			post_shader_.set_uniform_f("border_color", border_color_);
			Kernel().set_uniforms(post_shader_);
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
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_CULL_FACE);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void create_screen_vertex_array() {
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
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, shadow_width_, shadow_height_, lights_.size(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_storage_buffer_);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) + sizeof(GLfloat), &init_int, GL_DYNAMIC_DRAW);
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
			for (const auto& [object_id, object] : objects_) {
				if (object.transparent) {
					for (const auto& [model_id, model] : object.models) {
						transparent_objects.emplace_back(camera.position, &object, object_id, model_id);
					}
					continue;
				}

				main_shader_.set_uniform_i("object_id", object_id);
				object.draw(main_shader_);
			}

			std::sort(transparent_objects.rbegin(), transparent_objects.rend());
			for (const TransparentObject& object : transparent_objects) {
				main_shader_.set_uniform_i("object_id", object.object_id);
				object.object->draw(object.model_id, main_shader_);
			}
		}

		void draw_depth_map() const {
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer_);
			glViewport(0, 0, shadow_width_, shadow_height_);

			glClear(GL_DEPTH_BUFFER_BIT);

			for (size_t i = 0; i < lights_.size(); ++i) {
				if (lights_[i] == nullptr || !lights_[i]->shadow) {
					continue;
				}

				depth_shader_.set_uniform_matrix("light_space", lights_[i]->get_light_space_matrix());
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map_texture_id_, 0, i);
				for (const auto& [id, object] : objects_) {
					object.draw_depth_map();
				}
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			check_gl_errors(__FILE__, __LINE__, __func__);
		}

		void draw_primary_frame_buffer() const {
			glBindFramebuffer(GL_FRAMEBUFFER, primary_frame_buffer_);
			glViewport(0, 0, window_->getSize().x, window_->getSize().y);

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
			glViewport(0, 0, window_->getSize().x, window_->getSize().y);

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
			glDeleteVertexArrays(1, &screen_vertex_array_);
			glDeleteFramebuffers(1, &primary_frame_buffer_);
			glDeleteFramebuffers(1, &depth_map_frame_buffer_);
			glDeleteBuffers(1, &shader_storage_buffer_);
			glDeleteTextures(1, &screen_texture_id_);
			glDeleteTextures(1, &depth_stencil_texture_id_);
			glDeleteTextures(1, &depth_map_texture_id_);
			check_gl_errors(__FILE__, __LINE__, __func__);

			screen_vertex_array_ = 0;
			primary_frame_buffer_ = 0;
			depth_map_frame_buffer_ = 0;
			shader_storage_buffer_ = 0;
			screen_texture_id_ = 0;
			depth_stencil_texture_id_ = 0;
			depth_map_texture_id_ = 0;
		}

	public:
		Camera camera;

		GraphEngine(sf::RenderWindow* window) {
			if (!glew_is_ok()) {
				throw EngRuntimeError(__FILE__, __LINE__, "GraphEngine, failed to initialize GLEW.\n\n");
			}
			depth_shader_ = Shader<size_t>("GraphEngine/Shaders/Vertex/Depth", "GraphEngine/Shaders/Fragment/Depth", ShaderType::DEPTH);
			main_shader_ = Shader<size_t>("GraphEngine/Shaders/Vertex/Main", "GraphEngine/Shaders/Fragment/Main", ShaderType::MAIN);
			post_shader_ = Shader<size_t>("GraphEngine/Shaders/Vertex/Post", "GraphEngine/Shaders/Fragment/Post", ShaderType::POST);


			border_color_ = Vect3(1.0, 0.0, 0.0);
			this->window_ = window;
			window->setActive(true);

			init_gl();

			check_point_ = Vect2(window->getSize().x, window->getSize().y) / 2.0;
			camera.set_screen_ratio(((double)window->getSize().x) / ((double)window->getSize().y));
			lights_.resize(std::stoi(main_shader_.get_value_frag("NR_LIGHTS")), nullptr);

			create_buffers();
			set_uniforms();
		}

		GraphEngine(const GraphEngine& object) {
			*this = object;
		}

		GraphEngine& operator=(const GraphEngine& other) {
			deallocate();

			shadow_width_ = other.shadow_width_;
			shadow_height_ = other.shadow_height_;
			active_state_ = other.active_state_;
			lights_ = other.lights_;
			objects_ = other.objects_;
			window_ = other.window_;
			main_shader_ = other.main_shader_;
			post_shader_ = other.post_shader_;
			depth_shader_ = other.depth_shader_;
			camera = other.camera;

			create_buffers();
			set_uniforms();

			return *this;
		}

		GraphObject& operator[](int object_id) {
			if (!objects_.count(object_id)) {
				//std::cout << "ERROR::GRAPH_ENGINE::OPERATOR[]\n" << "Invalid object id.\n";
				//assert(0);
			}

			return objects_[object_id];
		}

		void set_clear_color(Vect3 color) {
			glClearColor(color.x, color.y, color.z, 1.0);
		}

		void set_light(int light_id, Light* new_light) {
			if (light_id < 0 || lights_.size() <= light_id) {
				//std::cout << "ERROR::GRAPH_ENGINE::SET_LIGHT\n" << "Invalid light id.\n";
				//assert(0);
			}

			lights_[light_id] = new_light;
		}

		void set_kernel(Kernel new_kernel) {
			post_shader_.use();
			new_kernel.set_uniforms(post_shader_);
		}

		void set_grayscale(bool grayscale) {
			post_shader_.use();
			post_shader_.set_uniform_i("grayscale", grayscale);
		}

		void set_kernel_offset(double kernel_offset) {
			if (kernel_offset < eps_) {
				//std::cout << "ERROR::GRAPH_ENGINE::SET_KERNEL_OFFSET\n" << "Not positive kernel offset.\n";
				//assert(0);
			}

			post_shader_.use();
			post_shader_.set_uniform_f("offset", kernel_offset);
		}

		void set_gamma(double gamma) {
			if (gamma < eps_) {
				//std::cout << "ERROR::GRAPH_ENGINE::SET_GAMMA\n" << "Not positive gamma.\n";
				//assert(0);
			}

			this->gamma_ = gamma;

			post_shader_.use();
			post_shader_.set_uniform_f("gamma", gamma);
		}

		void set_border_width(int border_width) {
			if (border_width < 0) {
				//std::cout << "ERROR::GRAPH_ENGINE::SET_BORDER_WIDTH\n" << "Negative border width.\n";
				//assert(0);
			}

			post_shader_.set_uniform_i("border_width", border_width);
		}

		void set_border_color(Vect3 color) {
			post_shader_.set_uniform_f("border_color", color);
		}

		Light* get_light(int light_id) {
			if (light_id < 0 || lights_.size() <= light_id) {
				//std::cout << "ERROR::GRAPH_ENGINE::GET_LIGHT\n" << "Invalid light id.\n";
				//assert(0);
			}

			return lights_[light_id];
		}

		int get_count_lights() {
			return lights_.size();
		}

		std::pair < int, int > get_center_object_id(Vect3& intersect_point) {
			int temp_int[2] = { -1, -1 };
			float temp_float = 1, distance = 0;
			std::pair < int, int > center_object_id(-1, -1);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_storage_buffer_);

			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &center_object_id.first);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(int), sizeof(int), &center_object_id.second);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int), sizeof(float), &distance);

			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(int), &temp_int);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int), sizeof(float), &temp_float);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			if (center_object_id.first != -1) {
				if (!objects_.count(center_object_id.first) || center_object_id.second == -1 || center_object_id.second >= objects_[center_object_id.first].models.size())
					center_object_id = std::make_pair(-1, -1);
				else
					center_object_id.second = objects_[center_object_id.first].models.get_id(center_object_id.second);
			}

			distance = 2.0 * distance - 1;
			double min_dist = camera.get_min_distance(), max_dist = camera.get_max_distance();
			double intersect_z = 2.0 * max_dist * min_dist / ((max_dist + min_dist) * (1.0 - distance * (max_dist - min_dist) / (max_dist + min_dist)));
			intersect_point = Matrix(camera.get_horizont(), camera.get_vertical(), camera.get_direction()) * Vect3(0, 0, intersect_z) + camera.position;

			return center_object_id;
		}

		double get_gamma() {
			return gamma_;
		}

		void swap(GraphEngine& other) noexcept {
			/*std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);
			std::swap(, other.);*/
		}

		int add_object(GraphObject object) {
			int free_object_id = 0;
			for (; objects_.count(free_object_id); free_object_id++) {}

			objects_[free_object_id] = object;

			return free_object_id;
		}

		bool delete_object(int object_id, int model_id = -1) {
			if (!objects_.count(object_id)) {
				//std::cout << "ERROR::GRAPH_ENGINE::DELETE_OBJECT\n" << "Invalid object id.\n";
				//assert(0);
			}

			if (model_id != -1)
				objects_[object_id].models.erase(model_id);

			if (model_id == -1 || objects_[object_id].models.size() == 0) {
				objects_.erase(object_id);
				return true;
			}

			return false;
		}

		void switch_active() {
			if (active_state_ == 0) {
				window_->setMouseCursorVisible(false);
				mouse_position_ = sf::Mouse::getPosition();
				sf::Mouse::setPosition(sf::Vector2i(window_->getSize().x / 2 + window_->getPosition().x, window_->getSize().y / 2 + window_->getPosition().y));
				active_state_ = 1;
			} else {
				window_->setMouseCursorVisible(true);
				sf::Mouse::setPosition(mouse_position_);
				active_state_ = 0;
			}
		}

		void compute_event(sf::Event event) {
			switch (event.type) {
			case sf::Event::MouseMoved: {
				if (active_state_ == 2) {
					camera.rotate(camera.get_vertical(), (event.mouseMove.x - int(window_->getSize().x / 2)) * camera.sensitivity);
					camera.rotate(camera.get_horizont(), (event.mouseMove.y - int(window_->getSize().y / 2)) * camera.sensitivity);
					sf::Mouse::setPosition(sf::Vector2i(window_->getSize().x / 2 + window_->getPosition().x, window_->getSize().y / 2 + window_->getPosition().y));
				} else if (active_state_ == 1) {
					active_state_ = 2;
				}
				break;
			}

			default:
				break;
			}
		}

		void update(double delta_time) {
			if (active_state_) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
					camera.rotate(camera.get_direction(), -camera.rotation_speed * delta_time);
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
					camera.rotate(camera.get_direction(), camera.rotation_speed * delta_time);

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
					delta_time *= camera.speed_delt;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
					camera.position += camera.speed * delta_time * camera.get_horizont();
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
					camera.position -= camera.speed * delta_time * camera.get_horizont();
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
					camera.position += camera.speed * delta_time * camera.get_direction();
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
					camera.position -= camera.speed * delta_time * camera.get_direction();
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
					camera.position += camera.speed * delta_time * camera.get_vertical();
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
					camera.position -= camera.speed * delta_time * camera.get_vertical();
			}
		}

		void draw() {
			window_->setActive(true);

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
