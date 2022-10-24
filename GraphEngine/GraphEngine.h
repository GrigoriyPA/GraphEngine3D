#pragma once

#include "Graphic—lasses/Texture.h"
#include "CommonClasses/Vect2.h"
#include "CommonClasses/Matrix.h"
#include "CommonClasses/Random.h"
#include "CommonClasses/Cut.h"
#include "CommonClasses/Flat.h"
#include <math.h>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphic—lasses/Shader.h"
#include "Graphic—lasses/Kernel.h"
#include "Camera.h"
#include "GraphObjects/GraphObject.h"
#include "Light/Light.h"


struct TransparentObject {
	int object_id, model_id, polygon_id;
	double distance;
	GraphObject* object;

	TransparentObject(eng::Vect3 cam_position, GraphObject* object, int object_id, std::pair < int, int > description) {
		this->object = object;
		this->object_id = object_id;
		model_id = description.first;
		polygon_id = description.second;
		distance = (cam_position - object->get_polygon_center(model_id, polygon_id)).length();
	}

	bool operator <(TransparentObject other) const {
		return distance < other.distance;
	}
};


class GraphEngine {
	int shadow_width = 1024, shadow_height = 1024, active_state = 0;
	double eps = 0.00001, gamma = 2.2;

	unsigned int primary_fbo, depth_map_fbo, center_object_ssbo, screen_tex, depth_stencil_tex, depth_map_tex, screen_coord;
	std::vector < Light* > lights;
	std::unordered_map < int, GraphObject > objects;
	sf::Vector2i mouse_position;
	sf::RenderWindow* window;
	Shader main_shader, post_shader, depth_shader;

	void init_gl() {
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			std::cout << "ERROR::GRAPH_ENGINE::INIT_GL\n" << "Failed to initialize GLEW.\n";
			assert(false);
		}

		glClearColor(0.2, 0.3, 0.3, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
	}

	void set_uniforms() {
		main_shader.use();
		glUniformMatrix4fv(glGetUniformLocation(main_shader.program, "projection"), 1, GL_FALSE, &std::vector<float>(cam.get_projection_matrix())[0]);
		glUniform1i(glGetUniformLocation(main_shader.program, "diffuse_map"), 0);
		glUniform1i(glGetUniformLocation(main_shader.program, "specular_map"), 1);
		glUniform1i(glGetUniformLocation(main_shader.program, "emission_map"), 2);
		glUniform1f(glGetUniformLocation(main_shader.program, "gamma"), gamma);
		glUniform2f(glGetUniformLocation(main_shader.program, "check_point"), window->getSize().x / 2, window->getSize().y / 2);

		post_shader.use();
		Kernel().use(&post_shader);
		glUniform1i(glGetUniformLocation(post_shader.program, "grayscale"), 0);
		glUniform1i(glGetUniformLocation(post_shader.program, "offset"), 5);
		glUniform1i(glGetUniformLocation(post_shader.program, "border_width"), 7);
		glUniform3f(glGetUniformLocation(post_shader.program, "border_color"), 1, 0, 0);
		glUniform1i(glGetUniformLocation(post_shader.program, "screen_texture"), 0);
		glUniform1i(glGetUniformLocation(post_shader.program, "stencil_texture"), 1);
	}

	void set_light_uniforms() {
		for (int i = 0; i < lights.size(); i++) {
			if (lights[i] == nullptr) {
				set_default_light_uniforms(i, &main_shader);
				continue;
			}

			lights[i]->set_uniforms(i, &main_shader);
		}
	}

	void create_screen_coord() {
		glGenVertexArrays(1, &screen_coord);
		glBindVertexArray(screen_coord);

		unsigned int vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		float vertices[] = {
			 1,  1, 1, 1,
			-1,  1, 0, 1,
			-1, -1, 0, 0,
			-1, -1, 0, 0,
			 1, -1, 1, 0,
			 1,  1, 1, 1
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDeleteBuffers(1, &vbo);
	}

	void create_primary_fbo() {
		glGenTextures(1, &screen_tex);
		glBindTexture(GL_TEXTURE_2D, screen_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window->getSize().x, window->getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &depth_stencil_tex);
		glBindTexture(GL_TEXTURE_2D, depth_stencil_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, window->getSize().x, window->getSize().y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float border_Òolor[] = { 0.0, 0.0, 0.0, 0.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_Òolor);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &primary_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, primary_fbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_tex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil_tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::GRAPH_ENGINE::CREATE_PRIMARY_FBO::\nFramebuffer is not complete.\n";
			assert(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void create_depth_map_fbo() {
		glGenFramebuffers(1, &depth_map_fbo);

		glGenTextures(1, &depth_map_tex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_tex);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, lights.size() + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float border_Òolor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_Òolor);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map_tex, 0, lights.size());
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::GRAPH_ENGINE::CREATE_DEPTH_MAP_FBO::\nFramebuffer is not complete.\n";
			assert(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void create_center_object_ssbo() {
		int temp_int[2] = { -1, -1 };
		float temp_float = 1;
		glGenBuffers(1, &center_object_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, center_object_ssbo);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, center_object_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int) + sizeof(float), &temp_int, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int), sizeof(float), &temp_float);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void create_buffers() {
		create_screen_coord();
		create_primary_fbo();
		create_depth_map_fbo();
		create_center_object_ssbo();
	}

	void delete_buffers() {
		glDeleteVertexArrays(1, &screen_coord);
		glDeleteFramebuffers(1, &primary_fbo);
		glDeleteFramebuffers(1, &depth_map_fbo);
		glDeleteBuffers(1, &center_object_ssbo);
		glDeleteTextures(1, &screen_tex);
		glDeleteTextures(1, &depth_stencil_tex);
		glDeleteTextures(1, &depth_map_tex);
	}

	void draw_objects() {
		std::vector < TransparentObject > transparent_objects;
		for (std::unordered_map < int, GraphObject >::iterator object = objects.begin(); object != objects.end(); object++) {
			if (object->second.transparent) {
				for (std::pair < int, int > description : object->second.get_models())
					transparent_objects.push_back(TransparentObject(cam.position, &object->second, object->first, description));
				
				continue;
			}

			object->second.draw(cam.position, &main_shader, object->first);
		}

		std::sort(transparent_objects.rbegin(), transparent_objects.rend());
		for (TransparentObject object : transparent_objects)
			object.object->draw_polygon(&main_shader, object.object_id, object.model_id, object.polygon_id);
	}

	void draw_depth_map() {
		glViewport(0, 0, shadow_width, shadow_height);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		depth_shader.use();

		for (int i = 0; i < lights.size(); i++) {
			if (lights[i] == nullptr || !lights[i]->shadow)
				continue;

			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map_tex, 0, i);
			glUniformMatrix4fv(glGetUniformLocation(depth_shader.program, "light_space"), 1, GL_FALSE, &std::vector<float>(lights[i]->get_light_space_matrix())[0]);

			for (std::unordered_map < int, GraphObject >::iterator object = objects.begin(); object != objects.end(); object++)
				object->second.draw_depth_map();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw_primary_fbo() {
		glViewport(0, 0, window->getSize().x, window->getSize().y);
		glBindFramebuffer(GL_FRAMEBUFFER, primary_fbo);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		main_shader.use();

		set_light_uniforms();
		glUniformMatrix4fv(glGetUniformLocation(main_shader.program, "view"), 1, GL_FALSE, &std::vector<float>(cam.get_view_matrix())[0]);
		glUniform3f(glGetUniformLocation(main_shader.program, "view_pos"), cam.position.x, cam.position.y, cam.position.z);

		glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map_tex);
		draw_objects();
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw_mainbuffer() {
		glViewport(0, 0, window->getSize().x, window->getSize().y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		post_shader.use();

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(screen_coord);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screen_tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depth_stencil_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}

public:
	Camera cam;

	GraphEngine(const GraphEngine& object) {
		*this = object;
	}

	GraphEngine(sf::RenderWindow* window, double fov, double min_distance, double max_distance) {
		if (fov < eps || fov > eng::PI - eps) {
			std::cout << "ERROR::GRAPH_ENGINE::BUILDER\n" << "Invalid viewing angle.\n";
			assert(0);
		}

		if (min_distance < eps || min_distance > max_distance || abs(min_distance - max_distance) < eps) {
			std::cout << "ERROR::GRAPH_ENGINE::BUILDER\n" << "Invalid draw distance.\n";
			assert(0);
		}

		this->window = window;
		window->setActive(true);

		init_gl();

		cam = Camera(fov, min_distance, max_distance, ((double)window->getSize().x) / ((double)window->getSize().y));
		main_shader = Shader("GraphEngine/Shaders/Vertex/Main", "GraphEngine/Shaders/Fragment/Main");
		post_shader = Shader("GraphEngine/Shaders/Vertex/Post", "GraphEngine/Shaders/Fragment/Post");
		depth_shader = Shader("GraphEngine/Shaders/Vertex/Depth", "GraphEngine/Shaders/Fragment/Depth");
		lights.resize(main_shader.get_const_int_value("NR_LIGHTS"), nullptr);

		create_buffers();
		set_uniforms();
	}

	GraphEngine& operator=(const GraphEngine& other) {
		delete_buffers();

		shadow_width = other.shadow_width;
		shadow_height = other.shadow_height;
		active_state = other.active_state;
		lights = other.lights;
		objects = other.objects;
		window = other.window;
		main_shader = other.main_shader;
		post_shader = other.post_shader;
		depth_shader = other.depth_shader;
		cam = other.cam;

		create_buffers();
		set_uniforms();

		return *this;
	}

	GraphObject& operator[](int object_id) {
		if (!objects.count(object_id)) {
			std::cout << "ERROR::GRAPH_ENGINE::OPERATOR[]\n" << "Invalid object id.\n";
			assert(0);
		}

		return objects[object_id];
	}

	void set_clear_color(eng::Vect3 color) {
		glClearColor(color.x, color.y, color.z, 1.0);
	}

	void set_light(int light_id, Light* new_light) {
		if (light_id < 0 || lights.size() <= light_id) {
			std::cout << "ERROR::GRAPH_ENGINE::SET_LIGHT\n" << "Invalid light id.\n";
			assert(0);
		}

		lights[light_id] = new_light;
	}

	void set_kernel(Kernel new_kernel) {
		post_shader.use();
		new_kernel.use(&post_shader);
	}

	void set_grayscale(bool grayscale) {
		post_shader.use();
		glUniform1i(glGetUniformLocation(post_shader.program, "grayscale"), grayscale);
	}

	void set_kernel_offset(double kernel_offset) {
		if (kernel_offset < eps) {
			std::cout << "ERROR::GRAPH_ENGINE::SET_KERNEL_OFFSET\n" << "Not positive kernel offset.\n";
			assert(0);
		}

		post_shader.use();
		glUniform1f(glGetUniformLocation(post_shader.program, "offset"), kernel_offset);
	}

	void set_gamma(double gamma) {
		if (gamma < eps) {
			std::cout << "ERROR::GRAPH_ENGINE::SET_GAMMA\n" << "Not positive gamma.\n";
			assert(0);
		}

		this->gamma = gamma;

		post_shader.use();
		glUniform1f(glGetUniformLocation(main_shader.program, "gamma"), gamma);
	}

	void set_border_width(int border_width) {
		if (border_width < 0) {
			std::cout << "ERROR::GRAPH_ENGINE::SET_BORDER_WIDTH\n" << "Negative border width.\n";
			assert(0);
		}

		glUniform1i(glGetUniformLocation(post_shader.program, "border_width"), border_width);
	}

	void set_border_color(eng::Vect3 color) {
		glUniform3f(glGetUniformLocation(post_shader.program, "border_color"), color.x, color.y, color.z);
	}

	Light* get_light(int light_id) {
		if (light_id < 0 || lights.size() <= light_id) {
			std::cout << "ERROR::GRAPH_ENGINE::GET_LIGHT\n" << "Invalid light id.\n";
			assert(0);
		}

		return lights[light_id];
	}

	int get_count_lights() {
		return lights.size();
	}

	std::pair < int, int > get_center_object_id(eng::Vect3& intersect_point) {
		int temp_int[2] = { -1, -1 };
		float temp_float = 1, distance = 0;
		std::pair < int, int > center_object_id(-1, -1);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, center_object_ssbo);

		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &center_object_id.first);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(int), sizeof(int), &center_object_id.second);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int), sizeof(float), &distance);

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(int), &temp_int);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(int), sizeof(float), &temp_float);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		if (center_object_id.first != -1) {
			if (!objects.count(center_object_id.first) || center_object_id.second >= objects[center_object_id.first].get_count_models())
				center_object_id = std::make_pair(-1, -1);
			else
				center_object_id.second = objects[center_object_id.first].get_model_id(center_object_id.second);
		}

		distance = 2.0 * distance - 1;
		double min_dist = cam.get_min_distance(), max_dist = cam.get_max_distance();
		double intersect_z = 2.0 * max_dist * min_dist / ((max_dist + min_dist) * (1.0 - distance * (max_dist - min_dist) / (max_dist + min_dist)));
		intersect_point = eng::Matrix(cam.get_horizont(), cam.get_vertical(), cam.get_direction()) * eng::Vect3(0, 0, intersect_z) + cam.position;

		return center_object_id;
	}

	double get_gamma() {
		return gamma;
	}

	int add_object(GraphObject object) {
		int free_object_id = 0;
		for (; objects.count(free_object_id); free_object_id++) {}

		objects[free_object_id] = object;

		return free_object_id;
	}

	bool delete_object(int object_id, int model_id = -1) {
		if (!objects.count(object_id)) {
			std::cout << "ERROR::GRAPH_ENGINE::DELETE_OBJECT\n" << "Invalid object id.\n";
			assert(0);
		}

		if (model_id != -1)
			objects[object_id].delete_model(model_id);

		if (model_id == -1 || objects[object_id].get_count_models() == 0) {
			objects.erase(object_id);
			return true;
		}

		return false;
	}

	void switch_active() {
		if (active_state == 0) {
			window->setMouseCursorVisible(false);
			mouse_position = sf::Mouse::getPosition();
			sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2 + window->getPosition().x, window->getSize().y / 2 + window->getPosition().y));
			active_state = 1;
		}
		else {
			window->setMouseCursorVisible(true);
			sf::Mouse::setPosition(mouse_position);
			active_state = 0;
		}
	}

	void compute_event(sf::Event event) {
		switch (event.type) {
		case sf::Event::MouseMoved: {
			if (active_state == 2) {
				cam.rotate(cam.get_vertical(), (event.mouseMove.x - int(window->getSize().x / 2)) * cam.sensitivity);
				cam.rotate(cam.get_horizont(), (event.mouseMove.y - int(window->getSize().y / 2)) * cam.sensitivity);
				sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2 + window->getPosition().x, window->getSize().y / 2 + window->getPosition().y));
			}
			else if (active_state == 1) {
				active_state = 2;
			}
			break;
		}

		default:
			break;
		}
	}

	void update(double delta_time) {
		if (active_state) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
				cam.rotate(cam.get_direction(), -cam.rotate_speed * delta_time);
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
				cam.rotate(cam.get_direction(), cam.rotate_speed * delta_time);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
				delta_time *= cam.speed_delt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				cam.position += cam.speed * delta_time * cam.get_horizont();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				cam.position -= cam.speed * delta_time * cam.get_horizont();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
				cam.position += cam.speed * delta_time * cam.get_direction();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
				cam.position -= cam.speed * delta_time * cam.get_direction();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
				cam.position += cam.speed * delta_time * cam.get_vertical();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
				cam.position -= cam.speed * delta_time * cam.get_vertical();
		}
	}

	void draw() {
		window->setActive(true);

		draw_depth_map();
		draw_primary_fbo();
		draw_mainbuffer();
	}

	~GraphEngine() {
		delete_buffers();
	}
};
