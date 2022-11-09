#pragma once


class Point : public RenderObject {
    void init(double radius = POINT_RADIUS) {
        eng::GraphObject point = eng::GraphObject::sphere(6, true, MAX_COUNT_MODELS);

        point.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(eng::Vect3(INTERFACE_TEXT_COLOR) / 255);
            mesh.material.set_diffuse(eng::Vect3(INTERFACE_TEXT_COLOR) / 255);
        });

        scene_id.second = point.models.insert(eng::Matrix::scale_matrix(radius));

        scene_id.first = scene->add_object(point);
    }

    void set_action(std::pair < int, int > button) {
        if (button.first == 3 && button.second == 0) {
            if (init_obj.size() == 2)
                action = 2;
            else
                action = 3;
        }
        else if (button.first == 4 && button.second == 0)
            action = 4;
        else if (button.first == 4 && button.second == 1)
            action = 5;
        else if (button.first == 4 && button.second == 2)
            action = 6;
        else if (button.first == 4 && button.second == 3)
            action = 7;
    }

    void update_center_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix((coord1 + coord2) / 2 - point));
    }

    void update_center_cut(std::pair < int, int > cut) {
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix((coord1 + coord2) / 2 - point));
    }

    void update_point_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        eng::Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        eng::Vect3 coord_point = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point_cur = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(coord_point.symmetry(coord_center) - point_cur));
    }

    void update_line_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        eng::Vect3 coord_center1 = (*scene)[center.first].get_mesh_center(center.second, 0);
        eng::Vect3 coord_center2 = (*scene)[center.first].get_mesh_center(center.second, 1);
        eng::Line center_line(coord_center1, coord_center2);
        eng::Vect3 coord_point = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point_cur = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(center_line.symmetry(coord_point) - point_cur));
    }

    void update_plane_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        std::vector < eng::Vect3 > center_coords = (*scene)[center.first].get_mesh_positions(center.second, 0);
        eng::Plane center_plane(center_coords);
        eng::Vect3 coord_point = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point_cur = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(center_plane.symmetry(coord_point) - point_cur));
    }

    void update_translate(std::pair < int, int > point, std::pair < int, int > start, std::pair < int, int > end) {
        eng::Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        eng::Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        eng::Vect3 translate = end_coord - start_coord;
        eng::Vect3 coord_point = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point_cur = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(coord_point + translate - point_cur));
    }

    void update_cut_connect(std::pair < int, int > cut) {
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(eng::Cut(coord1, coord2).project_point(point) - point));
    }

    void update_line_connect(std::pair < int, int > line) {
        eng::Vect3 coord1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 coord2 = (*scene)[line.first].get_mesh_center(line.second, 1);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(eng::Line(coord1, coord2).project_point(point) - point));
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(eng::Plane(coords).project_point(point) - point));
    }

    void update_triangle_connect(std::pair < int, int > triangle) {
        std::vector < eng::Vect3 > coords = (*scene)[triangle.first].get_mesh_positions(triangle.second, 0);
        eng::Vect3 point = (*scene)[scene_id.first].get_center(scene_id.second);

        eng::Vect3 new_point = eng::Plane(coords).project_point(point);
        if (!new_point.in_triangle(coords[0], coords[1], coords[2])) {
            int closest = 0;
            std::vector < eng::Vect3 > projection(3);
            for (int i = 0; i < 3; i++) {
                int next = (i + 1) % 3;
                projection[i] = eng::Cut(coords[i], coords[next]).project_point(new_point);
                if ((new_point - projection[i]).length() < (new_point - projection[closest]).length())
                    closest = i;
            }
            new_point = projection[closest];
        }

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(new_point - point));
    }

    void update_intersect() {
        std::vector < int > location(1, scene_id.first);
        RenderObject* point = init_obj[0]->intersect(init_obj[1]);

        if (point == nullptr) {
            if (init_obj[0]->get_type() == 1)
                update_cut_connect(init_obj[0]->scene_id);
            else if (init_obj[0]->get_type() == 2)
                update_line_connect(init_obj[0]->scene_id);
            else if (init_obj[0]->get_type() == 3)
                update_plan_connect(init_obj[0]->scene_id);
            else if (init_obj[0]->get_type() == 4)
                update_triangle_connect(init_obj[0]->scene_id);

            return;
        }

        (*scene)[scene_id.first].models.set(scene_id.second, (*scene)[point->scene_id.first].models[point->scene_id.second]);
        scene->delete_object(point->scene_id.first, point->scene_id.second);
        delete point;
    }

public:
    Point(eng::Vect3 position, eng::GraphEngine* scene, double radius = POINT_RADIUS) {
        type = 0;
        action = 0;
        this->scene = scene;

        init(radius);

        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix(position));
    }

    Point(std::pair < int, int > button, std::vector < RenderObject* > init_obj, eng::GraphEngine* scene) {
        type = 0;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        eng::Matrix model = (*scene)[scene_id.first].models[scene_id.second];
        if (!hide) {
            (*scene)[scene_id.first].models.change_left(scene_id.second, model * eng::Matrix::scale_matrix(eng::Vect3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0)) * model.inverse());
        }
        else
            (*scene)[scene_id.first].models.change_left(scene_id.second, model * eng::Matrix::scale_matrix(eng::Vect3(3.0, 3.0, 3.0)) * model.inverse());
        hide ^= 1;
    }
    
    void set_border(bool flag) {
        if (flag) {
            (*scene)[scene_id.first].border_mask = 1;
        } else {
            (*scene)[scene_id.first].border_mask = 0;
        }
    }

    void update() {
        if (action == 0)
            return;

        if (action == -1)
            update_cut_connect(init_obj[0]->scene_id);
        else if (action == -2)
            update_line_connect(init_obj[0]->scene_id);
        else if (action == -3)
            update_plan_connect(init_obj[0]->scene_id);
        else if (action == -4)
            update_triangle_connect(init_obj[0]->scene_id);
        else if (action == 1)
            update_intersect();
        else if (action == 2)
            update_center_two_points(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 3)
            update_center_cut(init_obj[0]->scene_id);
        else if (action == 4)
            update_point_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 5)
            update_line_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 6)
            update_plane_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 7)
            update_translate(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
    }

    RenderObject* intersect(RenderObject* obj) {
        return nullptr;
    }
};


bool can_connect(int type) {
    if (type == 1)
        return true;

    if (type == 2)
        return true;

    if (type == 3)
        return true;

    if (type == 4)
        return true;

    return false;
}
