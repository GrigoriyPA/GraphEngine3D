#pragma once


class RenderObject {
protected:
	bool visibility = true, hide = false;
	double eps = 0.00005;
	eng::Matrix save_matrix = eng::Matrix(4, 4, 0);

	int type;
	GraphEngine* scene;

	void change_matrix(eng::Matrix trans) {
		if (visibility)
			(*scene)[scene_id.first].models.change_left(scene_id.second, trans);
		else
			save_matrix = trans * save_matrix;
	}

public:
	double special_coefficient = 1;
	bool connect = false, target = false, moved = false, temp_vis = false;

	int action;
	std::pair < int, int > scene_id;
	std::vector < RenderObject* > init_obj;

	int get_type() {
		return type;
	}

	bool get_visibility() {
		return visibility;
	}

	bool get_hide() {
		return hide;
	}

	bool delete_object() {
		return (*scene).delete_object(scene_id.first, scene_id.second);
	}

	void switch_visibility() {
		eng::Matrix cur_matrix = (*scene)[scene_id.first].models[scene_id.second];
		if (visibility)
			(*scene)[scene_id.first].models.set(scene_id.second, eng::Matrix(4, 4, 0));
		else
			(*scene)[scene_id.first].models.set(scene_id.second, save_matrix);
		visibility ^= 1;
		save_matrix = cur_matrix;
	}

	bool can_move() {
		if (action < 0)
			return true;

		if (action > 0)
			return false;

		for (RenderObject* object : init_obj) {
			if (object->get_type() > 0 || object->action > 0)
				return false;
		}

		return true;
	}

	void move(eng::Matrix trans) {
		if (moved)
			return;

		if (type == 0 && action <= 0) {
			moved = true;
			change_matrix(trans);
			return;
		}

		for (RenderObject* object : init_obj) {
			if (object->moved)
				continue;

			object->moved = true;
			object->change_matrix(trans);
		}
	}

	virtual void set_border(bool flag) = 0;

	virtual void switch_hide() = 0;

	virtual void update() = 0;

	virtual RenderObject* intersect(RenderObject* obj) = 0;
};


#include "Point.h"
#include "Cut.h"
#include "Line.h"
#include "Plane.h"
#include "Triangle.h"


bool is_available(std::pair < int, int > button, int type, std::vector < RenderObject* > selected_objects) {
	if (button.first == 1 && button.second == 0)
		return type == 0;

	if (button.first == 1 && button.second == 1)
		return type == 0;

	if (button.first == 1 && button.second == 2)
		return type < 2;

	if (button.first == 1 && button.second == 3)
		return type < 3;

	if (button.first == 1 && button.second == 4)
		return type < 2;

	if (button.first == 2 && button.second == 0)
		return true;

	if (button.first == 2 && button.second == 1)
		return true;

	if (button.first == 2 && button.second == 2) {
		if (selected_objects.size() == 0 && (type == 0 || can_connect(type)))
			return true;

		if (selected_objects.size() == 1 && selected_objects[0]->get_type() == 0 && can_connect(type))
			return true;

		return selected_objects.size() == 1 && selected_objects[0]->get_type() != 0 && type == 0;
	}

	if (button.first == 2 && button.second == 3)
		return type > 0;

	if (button.first == 3 && button.second == 0) {
		if (selected_objects.size() == 0)
			return type < 2;

		return type == 0;
	}

	if (button.first == 3 && button.second == 1) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects[0]->get_type() == 0)
			return 0 < type && type < 5;

		return type == 0;
	}

	if (button.first == 3 && button.second == 2) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects.size() == 1) {
			if (selected_objects[0]->get_type() == 0)
				return 0 < type && type < 5;
			if (selected_objects[0]->get_type() < 3)
				return type == 0;
			return type == 0 || 2 < type && type < 5;
		}

		if (selected_objects[0]->get_type() == 0 || selected_objects[1]->get_type() == 0)
			return 2 < type && type < 5;

		return type == 0;
	}

	if (button.first == 3 && button.second == 3) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects[0]->get_type() == 0)
			return 0 < type && type < 3;

		if (selected_objects[0]->get_type() < 3)
			return type == 0 || 2 < type && type < 5;

		return 0 < type && type < 3;
	}

	if (button.first == 3 && button.second == 4) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects.size() == 1) {
			if (selected_objects[0]->get_type() == 0)
				return 0 < type && type < 5;
			if (selected_objects[0]->get_type() < 3)
				return type < 3;
			return type == 0;
		}

		if (selected_objects[0]->get_type() == 0 || selected_objects[1]->get_type() == 0)
			return 0 < type && type < 3;

		return type == 0;
	}

	if (button.first == 3 && button.second == 5) {
		if (selected_objects.size() == 0)
			return type < 2;

		return type == 0;
	}

	if (button.first == 4 && button.second == 0) {
		if (selected_objects.size() == 0)
			return type < 5;

		return type == 0;
	}

	if (button.first == 4 && button.second == 1) {
		if (selected_objects.size() == 0)
			return type < 5;

		return 0 < type && type < 3;
	}

	if (button.first == 4 && button.second == 2) {
		if (selected_objects.size() == 0)
			return type < 5;

		return 2 < type && type < 5;
	}

	if (button.first == 4 && button.second == 3) {
		if (selected_objects.size() == 0)
			return type < 5;

		return type == 0;
	}

	if (button.first == 5 && button.second == 0) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects.size() == 2)
			return type == 0;

		if (selected_objects[0]->get_type() == 0)
			return type == 0;

		if (selected_objects[0]->get_type() < 3)
			return 0 < type && type < 3;

		return 2 < type && type < 5;
	}

	if (button.first == 5 && button.second == 1) {
		if (selected_objects.size() == 0)
			return type < 5;

		if (selected_objects[0]->get_type() == 0 && type == 0)
			return false;

		if (0 < selected_objects[0]->get_type() && selected_objects[0]->get_type() < 3 && 0 < type && type < 3)
			return false;

		if (2 < selected_objects[0]->get_type() && selected_objects[0]->get_type() < 5 && 2 < type && type < 5)
			return false;

		if (selected_objects.size() == 1)
			return true;

		if (selected_objects[1]->get_type() == 0 && type == 0)
			return false;

		if (0 < selected_objects[1]->get_type() && selected_objects[1]->get_type() < 3 && 0 < type && type < 3)
			return false;

		if (2 < selected_objects[1]->get_type() && selected_objects[1]->get_type() < 5 && 2 < type && type < 5)
			return false;

		return true;
	}

	if (button.first == 5 && button.second == 2) {
		if (selected_objects.size() == 0)
			return type < 5 && type != 2;

		if (selected_objects.size() == 1) {
			if (selected_objects[0]->get_type() == 0)
				return type == 0 || 2 < type && type < 5;

			if (selected_objects[0]->get_type() == 1)
				return 2 < type && type < 5;

			return type < 2;
		}

		if (selected_objects[0]->get_type() == 0 && selected_objects[1]->get_type() == 0)
			return 2 < type && type < 5;

		return type == 0;
	}

	return false;
}


bool is_ready(std::pair < int, int > button, std::vector < RenderObject* > selected_objects) {
	if (button.first == 1 && button.second == 0)
		return true;

	if (button.first == 1 && button.second == 1)
		return selected_objects.size() == 2;
	
	if (button.first == 1 && button.second == 2) {
		if (selected_objects.size() == 1)
			return selected_objects[0]->get_type() == 1;
		return selected_objects.size() == 2;
	}

	if (button.first == 1 && button.second == 3)
		return selected_objects.size() == 3 || selected_objects.size() == 2 && (selected_objects[0]->get_type() > 0 || selected_objects[1]->get_type() > 0);

	if (button.first == 1 && button.second == 4)
		return selected_objects.size() == 3 || selected_objects.size() == 2 && (selected_objects[0]->get_type() > 0 || selected_objects[1]->get_type() > 0);

	if (button.first == 2 && button.second == 0)
		return true;

	if (button.first == 2 && button.second == 1)
		return true;

	if (button.first == 2 && button.second == 2) {
		if (selected_objects.size() == 1 && selected_objects[0]->get_type() == 0 && selected_objects[0]->action < 0)
			return true;

		return selected_objects.size() == 2;
	}

	if (button.first == 2 && button.second == 3)
		return selected_objects.size() == 2;

	if (button.first == 3 && button.second == 0) {
		if (selected_objects.size() == 1)
			return selected_objects[0]->get_type() == 1;

		return selected_objects.size() == 2;
	}

	if (button.first == 3 && button.second == 1)
		return selected_objects.size() == 2;

	if (button.first == 3 && button.second == 2) {
		if (selected_objects.size() == 2)
			return selected_objects[0]->get_type() == 0 && selected_objects[1]->get_type() < 3 || selected_objects[1]->get_type() == 0 && selected_objects[0]->get_type() < 3;

		return selected_objects.size() == 3;
	}

	if (button.first == 3 && button.second == 3)
		return selected_objects.size() == 2;

	if (button.first == 3 && button.second == 4) {
		if (selected_objects.size() == 2) {
			if (selected_objects[0]->get_type() == 0)
				return 2 < selected_objects[1]->get_type() && selected_objects[1]->get_type() < 5;
			if (selected_objects[1]->get_type() == 0)
				return 2 < selected_objects[0]->get_type() && selected_objects[0]->get_type() < 5;
			return false;
		}

		return selected_objects.size() == 3;
	}

	if (button.first == 3 && button.second == 5) {
		if (selected_objects.size() == 1)
			return selected_objects[0]->get_type() == 1;

		return selected_objects.size() == 2;
	}

	if (button.first == 4 && button.second == 0)
		return selected_objects.size() == 2;

	if (button.first == 4 && button.second == 1)
		return selected_objects.size() == 2;

	if (button.first == 4 && button.second == 2)
		return selected_objects.size() == 2;

	if (button.first == 4 && button.second == 3)
		return selected_objects.size() == 3;

	if (button.first == 5 && button.second == 0)
		return selected_objects.size() == 3 || selected_objects.size() == 2 && selected_objects[0]->get_type() > 0;

	if (button.first == 5 && button.second == 1)
		return selected_objects.size() == 3;

	if (button.first == 5 && button.second == 2) {
		if (selected_objects.size() == 2)
			return selected_objects[0]->get_type() == 1 || selected_objects[1]->get_type() == 1;

		return selected_objects.size() == 3;
	}

	return false;
}
