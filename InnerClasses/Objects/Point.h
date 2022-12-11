#pragma once


class Point : public RenderObject {
    void init(double radius = POINT_RADIUS) {
        gre::GraphObject point = gre::GraphObject::sphere(6, true, MAX_COUNT_MODELS);

        point.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(gre::Vec3(INTERFACE_TEXT_COLOR) / 255);
            mesh.material.set_diffuse(gre::Vec3(INTERFACE_TEXT_COLOR) / 255);
        });

        scene_id.second = point.models.insert(gre::Matrix::scale_matrix(radius));

        scene_id.first = scene->objects.insert(point);
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
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix((coord1 + coord2) / 2 - point));
    }

    void update_center_cut(std::pair < int, int > cut) {
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix((coord1 + coord2) / 2 - point));
    }

    void update_point_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        gre::Vec3 coord_center = (*scene).objects[center.first].get_center(center.second);
        gre::Vec3 coord_point = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point_cur = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(coord_point.symmetry(coord_center) - point_cur));
    }

    void update_line_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        gre::Vec3 coord_center1 = (*scene).objects[center.first].get_mesh_center(center.second, 0);
        gre::Vec3 coord_center2 = (*scene).objects[center.first].get_mesh_center(center.second, 1);
        gre::Line center_line(coord_center1, coord_center2);
        gre::Vec3 coord_point = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point_cur = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(center_line.symmetry(coord_point) - point_cur));
    }

    void update_plane_symmetry(std::pair < int, int > point, std::pair < int, int > center) {
        std::vector < gre::Vec3 > center_coords = (*scene).objects[center.first].get_mesh_positions(center.second, 0);
        gre::Plane center_plane(center_coords);
        gre::Vec3 coord_point = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point_cur = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(center_plane.symmetry(coord_point) - point_cur));
    }

    void update_translate(std::pair < int, int > point, std::pair < int, int > start, std::pair < int, int > end) {
        gre::Vec3 start_coord = (*scene).objects[start.first].get_center(start.second);
        gre::Vec3 end_coord = (*scene).objects[end.first].get_center(end.second);
        gre::Vec3 translate = end_coord - start_coord;
        gre::Vec3 coord_point = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point_cur = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(coord_point + translate - point_cur));
    }

    void update_cut_connect(std::pair < int, int > cut) {
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(gre::Cut(coord1, coord2).project_point(point) - point));
    }

    void update_line_connect(std::pair < int, int > line) {
        gre::Vec3 coord1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(gre::Line(coord1, coord2).project_point(point) - point));
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(gre::Plane(coords).project_point(point) - point));
    }

    void update_triangle_connect(std::pair < int, int > triangle) {
        std::vector < gre::Vec3 > coords = (*scene).objects[triangle.first].get_mesh_positions(triangle.second, 0);
        gre::Vec3 point = (*scene).objects[scene_id.first].get_center(scene_id.second);

        gre::Vec3 new_point = gre::Plane(coords).project_point(point);
        if (!new_point.in_triangle(coords[0], coords[1], coords[2])) {
            int closest = 0;
            std::vector < gre::Vec3 > projection(3);
            for (int i = 0; i < 3; i++) {
                int next = (i + 1) % 3;
                projection[i] = gre::Cut(coords[i], coords[next]).project_point(new_point);
                if ((new_point - projection[i]).length() < (new_point - projection[closest]).length())
                    closest = i;
            }
            new_point = projection[closest];
        }

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(new_point - point));
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

        (*scene).objects[scene_id.first].models.set(scene_id.second, (*scene).objects[point->scene_id.first].models[point->scene_id.second]);
        scene->objects.erase(point->scene_id.first, point->scene_id.second);
        delete point;
    }

public:
    Point(gre::Vec3 position, gre::GraphEngine* scene, double radius = POINT_RADIUS) {
        type = 0;
        action = 0;
        this->scene = scene;

        init(radius);

        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(position));
    }

    Point(std::pair < int, int > button, std::vector < RenderObject* > init_obj, gre::GraphEngine* scene) {
        type = 0;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        gre::Matrix model = (*scene).objects[scene_id.first].models[scene_id.second];
        if (!hide) {
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0)) * model.inverse());
        }
        else
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(3.0, 3.0, 3.0)) * model.inverse());
        hide ^= 1;
    }
    
    void set_border(bool flag) {
        if (flag) {
            (*scene).objects[scene_id.first].border_mask = 1;
        } else {
            (*scene).objects[scene_id.first].border_mask = 0;
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
