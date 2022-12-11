#pragma once

#include "Objects/RenderObject.h"


class RenderingSequence {
	bool moving = false;
	int input_state = 0, active_object = -1;
	std::pair < int, int > active_button = std::make_pair(0, 0);
	double point_distance = 1, scroll = 0, eps = 0.00005;

	std::vector < RenderObject* > selected_objects;
	std::vector < RenderObject* > objects;
	std::map < std::pair < int, int >, int > object_id;
	gre::Vec3 stable_point, temp_point, intersect_point;
	gre::GraphEngine* scene;

	bool can_switch_to_point() {
		return is_available(active_button, 0, selected_objects) && (active_button.first != 2 || active_button.second > 1);
	}

	void check_active_button(std::pair < int, int > cur_active_button) {
		if (active_button == cur_active_button)
			return;

		moving = false;
		for (int i = 1; i < objects.size(); i++) {
			objects[i]->connect = false;

			if (cur_active_button.first == 2 && cur_active_button.second == 0 && !objects[i]->get_visibility()) {
				objects[i]->switch_visibility();
				objects[i]->switch_hide();
			}
			if (active_button.first == 2 && active_button.second == 0 && objects[i]->get_hide()) {
				objects[i]->switch_hide();
				objects[i]->switch_visibility();
			}
		}
		
		for (RenderObject* object : selected_objects) {
			if (!object->target)
				object->set_border(false);
		}
		selected_objects.clear();

		active_button = cur_active_button;
		if (!can_switch_to_point() && input_state == 1)
			switch_input_state();
	}

	void check_button_state() {
		if (input_state == 1 && !can_switch_to_point())
			switch_input_state();

		if (active_button.first == 0 || !is_ready(active_button, selected_objects))
			return;

		if (active_button.first == 3 && active_button.second == 0) {
			add_object(new Point(active_button, selected_objects, scene));
		}
		else if (active_button.first == 1 && active_button.second == 1) {
			add_object(new Cut(active_button, selected_objects, scene));
		}
		else if (active_button.first == 1 && active_button.second == 2 ||
				active_button.first == 3 && active_button.second == 1 ||
				active_button.first == 3 && active_button.second == 2 ||
				active_button.first == 5 && active_button.second == 1 ||
				active_button.first == 5 && active_button.second == 2) {

			add_object(new Line(active_button, selected_objects, scene));
		}
		else if (active_button.first == 1 && active_button.second == 3 ||
				active_button.first == 3 && active_button.second == 3 ||
				active_button.first == 3 && active_button.second == 4 ||
				active_button.first == 3 && active_button.second == 5) {
			add_object(new Plane(active_button, selected_objects, scene));
		}
		else if (active_button.first == 1 && active_button.second == 4) {
			add_object(new Triangle(active_button, selected_objects, scene));
		}
		else if (active_button.first == 2 && active_button.second == 0) {
			selected_objects[0]->switch_hide();
		}
		else if (active_button.first == 2 && active_button.second == 1) {
			delete_object(selected_objects[0]);
			selected_objects.clear();
		}
		else if (active_button.first == 2 && active_button.second == 2) {
			if (selected_objects.size() == 1) {
				selected_objects[0]->action = 0;
				selected_objects[0]->init_obj.clear();
			}
			else {
				if (selected_objects[0]->get_type() != 0)
					std::swap(selected_objects[0], selected_objects[1]);

				if (add_edge(selected_objects[0], selected_objects[1])) {
					selected_objects[0]->action = -selected_objects[1]->get_type();
					selected_objects[0]->init_obj.push_back(selected_objects[1]);
				}
			}
		}
		else if (active_button.first == 2 && active_button.second == 3) {
			RenderObject* intersect = selected_objects[0]->intersect(selected_objects[1]);
			if (intersect != nullptr)
				add_object(intersect);
		}
		else if (active_button.first == 4 && active_button.second == 0 ||
				active_button.first == 4 && active_button.second == 1 ||
				active_button.first == 4 && active_button.second == 2 ||
				active_button.first == 4 && active_button.second == 3) {
			if (selected_objects[0]->get_type() == 0)
				add_object(new Point(active_button, selected_objects, scene));
			else if (selected_objects[0]->get_type() == 1)
				add_object(new Cut(active_button, selected_objects, scene));
			else if (selected_objects[0]->get_type() == 2)
				add_object(new Line(active_button, selected_objects, scene));
			else if (selected_objects[0]->get_type() == 3)
				add_object(new Plane(active_button, selected_objects, scene));
			else if (selected_objects[0]->get_type() == 4)
				add_object(new Triangle(active_button, selected_objects, scene));
		}
		else if (active_button.first == 5 && active_button.second == 0) {
			if (selected_objects[0]->get_type() == 0) {
				add_object(new Line(active_button, selected_objects, scene));
			}
			if (selected_objects[0]->get_type() < 3) {
				add_object(new Line(active_button, selected_objects, scene));
				int id = add_object(new Line(active_button, selected_objects, scene));
				objects[id]->special_coefficient = -1;
				objects[id]->moved = true;
			}
			else {
				add_object(new Plane(active_button, selected_objects, scene));
				int id = add_object(new Plane(active_button, selected_objects, scene));
				objects[id]->special_coefficient = -1;
				objects[id]->moved = true;
			}
		}

		for (RenderObject* object : selected_objects) {
			if (!object->target)
				object->set_border(false);
		}
		selected_objects.clear();
	}

	bool add_edge(RenderObject* a, RenderObject* b) {
		a->moved = true;
		for (RenderObject* object : objects) {
			for (RenderObject* obj : object->init_obj)
				object->moved = object->moved || obj->moved;
		}

		if (b->moved)
			return false;

		std::vector < RenderObject* > new_objects;
		for (RenderObject* object : objects) {
			if (!object->moved) {
				object_id[object->scene_id] = new_objects.size();
				new_objects.push_back(object);
				continue;
			}
		}
		for (RenderObject* object : objects) {
			if (object->moved) {
				object_id[object->scene_id] = new_objects.size();
				new_objects.push_back(object);
				continue;
			}
		}
		objects = new_objects;
	}

	void switch_input_state() {
		objects[0]->switch_visibility();
		input_state ^= 1;
	}

	void update_objects() {
		for (RenderObject* object : objects) {
			if (!object->get_visibility()) {
				object->switch_visibility();
				object->temp_vis = true;
			}
		}
		for (RenderObject* object : objects) {
			for (RenderObject* obj : object->init_obj)
				object->moved = object->moved || obj->moved;

			if (!object->moved)
				continue;

			object->update();
		}
		for (RenderObject* object : objects) {
			object->moved = false;
			if (object->temp_vis) {
				object->switch_visibility();
				object->temp_vis = false;
			}
		}
	}

public:
	RenderingSequence(gre::GraphEngine* scene) {
		this->scene = scene;

		object_id[std::make_pair(-1, -1)] = -1;

		temp_point = scene->cameras[0].position + scene->cameras[0].get_direction() * point_distance;
		add_object(new Point(temp_point, scene));
		objects[0]->switch_visibility();
	}

	int get_cross_state() {
		if (input_state == 1)
			return 0;

		if (moving)
			return 3;

		if (active_object == -1)
			return 1;

		if (active_button.first == 0 && objects[active_object]->can_move())
			return 2;

		if (std::count(selected_objects.begin(), selected_objects.end(), objects[active_object]))
			return 2;

		if (can_switch_to_point() && objects[active_object]->target && can_connect(objects[active_object]->get_type()))
			return 3;

		if (is_available(active_button, objects[active_object]->get_type(), selected_objects))
			return 2;

		return 1;
	}

	int add_object(RenderObject* object) {
		object_id[object->scene_id] = objects.size();
		objects.push_back(object);

		return objects.size() - 1;
	}

	void delete_object(RenderObject* cur_object) {
		for (RenderObject* object : objects) {
			if (object == cur_object)
				object->moved = true;

			for (RenderObject* obj : object->init_obj)
				object->moved = object->moved || obj->moved;
		}

		std::vector < RenderObject* > new_objects;
		for (RenderObject* object : objects) {
			if (!object->moved) {
				object_id[object->scene_id] = new_objects.size();
				new_objects.push_back(object);
				continue;
			}

			object_id.erase(object->scene_id);
			object->delete_object();
			delete object;
		}
		objects = new_objects;
	}

	void compute_event(sf::Event event, std::pair < int, int > cur_active_button) {
		check_active_button(cur_active_button);
        switch (event.type) {
            case sf::Event::MouseButtonPressed: {
                if (event.mouseButton.button == sf::Mouse::Left) {
					if (active_button.first == 0) {
						if (get_cross_state() == 2) {
							moving = true;
							stable_point = intersect_point;
							objects[active_object]->connect = true;
							if (objects[active_object]->target) {
								for (RenderObject* object : objects) {
									if (!object->target || !object->can_move())
										continue;

									object->connect = true;
								}
							}
						}
					}
					else if (input_state == 0) {
						if (get_cross_state() == 2) {
							std::vector < RenderObject* >::iterator it = std::find(selected_objects.begin(), selected_objects.end(), objects[active_object]);
							if (it != selected_objects.end()) {
								if (!objects[active_object]->target)
									objects[active_object]->set_border(false);
								selected_objects.erase(it);
							}
							else {
								objects[active_object]->set_border(true);
								selected_objects.push_back(objects[active_object]);
								check_button_state();
							}
						}
						else if (get_cross_state() == 3) {
							int id = add_object(new Point(intersect_point, scene));
							objects[id]->action = -objects[active_object]->get_type();
							objects[id]->init_obj.push_back(objects[active_object]);
							objects[id]->update();
							objects[id]->set_border(true);
							selected_objects.push_back(objects[id]);
							check_button_state();
						}
					}
					else {
						int id = add_object(new Point(temp_point, scene));
						objects[id]->set_border(true);
						selected_objects.push_back(objects[id]);
						check_button_state();
					}
                }
				else if (event.mouseButton.button == sf::Mouse::Middle) {
					if (input_state == 1 || can_switch_to_point())
						switch_input_state();
				}
				else if (event.mouseButton.button == sf::Mouse::Right) {
					if (active_object > 0) {
						bool new_state = objects[active_object]->target ^ 1;
						if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && !sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
							for (RenderObject* object : objects)
								object->target = false;
						}
						objects[active_object]->target = new_state;

						for (RenderObject* object : objects) {
							if (object->target)
								object->set_border(true);
							else
								object->set_border(false);
						}
						for (RenderObject* object : selected_objects)
							object->set_border(true);
					}
				}
                break;
            }

			case sf::Event::MouseButtonReleased: {
				if (event.mouseButton.button == sf::Mouse::Left) {
					if (active_button.first == 0) {
						moving = false;
						for (RenderObject* object : objects)
							object->connect = false;
					}
				}
				break;
			}

            case sf::Event::MouseWheelScrolled: {
				scroll += event.mouseWheelScroll.delta;
				if (active_button.first != 0 && input_state == 1)
					point_distance *= pow(SCROLL_SENSITIVITY, event.mouseWheelScroll.delta);
                break;
            }

			case sf::Event::KeyReleased: {
				if (event.key.code == sf::Keyboard::R) {
					if (input_state == 1 || can_switch_to_point())
						switch_input_state();
				}
				break;
			}

            default:
                break;
        }
	}

	void update(std::pair < int, int > cur_active_button) {
		int cam_id = 0;

		check_active_button(cur_active_button);
		auto obj_id = scene->get_check_object(cam_id, intersect_point);
		active_object = object_id[{-1, -1}];
		if (obj_id.exist) {
			active_object = object_id[{obj_id.object_id, obj_id.model_id}];
		}

		gre::Matrix trans = gre::Matrix::translation_matrix(scene->cameras[cam_id].get_change_vector(stable_point));
		stable_point = trans * stable_point;
		scene->cameras[cam_id].update();

		double delt = pow(SCROLL_SENSITIVITY, scroll);
		gre::Vec3 new_point = (stable_point - scene->cameras[cam_id].position) * delt + scene->cameras[cam_id].position;
		trans = trans * gre::Matrix::translation_matrix(new_point - stable_point);
		stable_point = new_point;
		scroll = 0;

		if ((trans * gre::Vec3(0, 0, 0)).length() > eps) {
			for (RenderObject* object : objects) {
				if (object->connect)
					object->move(trans);
			}
		}

		if (input_state == 1) {
			gre::Vec3 new_temp_point = scene->cameras[cam_id].position + scene->cameras[cam_id].get_direction() * point_distance;
			if ((new_temp_point - temp_point).length() > eps)
				objects[0]->move(gre::Matrix::translation_matrix(new_temp_point - temp_point));
			temp_point = new_temp_point;
		}

		update_objects();
	}
};
