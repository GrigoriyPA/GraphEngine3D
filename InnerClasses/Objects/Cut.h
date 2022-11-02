#pragma once


class Cut : public RenderObject {
    void init() {
        eng::GraphObject cut = get_cylinder(12, true, MAX_COUNT_MODELS);

        cut.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_diffuse(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_specular(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
        });

        scene_id.second = cut.add_model();

        scene_id.first = scene->add_object(cut);
    }

    void set_action(std::pair < int, int > button) {
        if (button.first == 1 && button.second == 1)
            action = 0;
        else if (button.first == 4 && button.second == 0)
            action = 2;
        else if (button.first == 4 && button.second == 1)
            action = 3;
        else if (button.first == 4 && button.second == 2)
            action = 4;
        else if (button.first == 4 && button.second == 3)
            action = 5;
    }

    void update_cut(eng::Vect3 point1, eng::Vect3 point2) {
        eng::Vect3 direct = (point2 - point1).normalize();
        eng::Vect3 horizont = direct.horizont();
        eng::Vect3 vertical = direct ^ horizont;
        double length = (point2 - point1).length();

        (*scene)[scene_id.first].set_matrix(eng::Matrix::scale_matrix(eng::Vect3(POINT_RADIUS * 0.2, length, POINT_RADIUS * 0.2)), scene_id.second);
        (*scene)[scene_id.first].change_matrix_left(eng::Matrix(horizont, -direct, vertical), scene_id.second);
        (*scene)[scene_id.first].change_matrix_left(eng::Matrix::translation_matrix(point2), scene_id.second);
    }

    void update_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        
        update_cut(coord1, coord2);
    }

    void update_point_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        eng::Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);

        update_cut(coord1.symmetry(coord_center), coord2.symmetry(coord_center));
    }

    void update_line_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        eng::Vect3 coord_center1 = (*scene)[center.first].get_mesh_center(center.second, 0);
        eng::Vect3 coord_center2 = (*scene)[center.first].get_mesh_center(center.second, 1);
        eng::Line center_line(coord_center1, coord_center2);
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);

        update_cut(center_line.symmetry(coord1), center_line.symmetry(coord2));
    }

    void update_plane_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        std::vector < eng::Vect3 > center_coords = (*scene)[center.first].get_mesh_positions(center.second, 0);
        eng::Flat center_plane(center_coords);
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);

        update_cut(center_plane.symmetry(coord1), center_plane.symmetry(coord2));
    }

    void update_translate(std::pair < int, int > cut, std::pair < int, int > start, std::pair < int, int > end) {
        eng::Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        eng::Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        eng::Vect3 translate = end_coord - start_coord;
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);

        update_cut(coord1 + translate, coord2 + translate);
    }

    RenderObject* intersect_cut(eng::Cut cut_cur, RenderObject* cut) {
        eng::Vect3 coord1 = (*scene)[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        eng::Cut cut_ot(coord1, coord2);

        if (!cut_cur.is_intersect(cut_ot))
            return nullptr;

        RenderObject* point = new Point(cut_cur.intersect(cut_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { this, cut };

        return point;
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Flat plane_proj(coords);
        eng::Vect3 point1 = plane_proj.project_point((*scene)[scene_id.first].get_mesh_center(scene_id.second, 0));
        eng::Vect3 point2 = plane_proj.project_point((*scene)[scene_id.first].get_mesh_center(scene_id.second, 1));

        update_cut(point1, point2);
    }

    void update_intersect() {
        RenderObject* cut = init_obj[0]->intersect(init_obj[1]);

        if (cut == nullptr) {
            if (init_obj[0]->get_type() == 3)
                update_plan_connect(init_obj[0]->scene_id);

            return;
        }

        (*scene)[scene_id.first].set_matrix((*scene)[cut->scene_id.first].get_matrix(cut->scene_id.second), scene_id.second);
        scene->delete_object(cut->scene_id.first, cut->scene_id.second);
        delete cut;
    }

public:
    Cut(eng::Vect3 point1, eng::Vect3 point2, GraphEngine* scene) {
        action = -1;
        type = 1;
        this->scene = scene;

        init();
        update_cut(point1, point2);
    }

    Cut(std::pair < int, int > button, std::vector < RenderObject* > init_obj, GraphEngine* scene) {
        type = 1;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        eng::Matrix model = (*scene)[scene_id.first].get_matrix(scene_id.second);
        if (!hide) {
            (*scene)[scene_id.first].change_matrix_left(model * eng::Matrix::scale_matrix(eng::Vect3(1.0 / 3.0, 1, 1.0 / 3.0)) * model.inverse(), scene_id.second);
        } else
            (*scene)[scene_id.first].change_matrix_left(model * eng::Matrix::scale_matrix(eng::Vect3(3.0, 1.0, 3.0)) * model.inverse(), scene_id.second);
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
            update_two_points(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 1)
            update_intersect();
        else if (action == 2)
            update_point_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 3)
            update_line_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 4)
            update_plane_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 5)
            update_translate(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
    }

    RenderObject* intersect(RenderObject* obj) {
        if (obj->get_type() > type)
            return obj->intersect(this);

        eng::Vect3 coord1 = (*scene)[scene_id.first].get_mesh_center(scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[scene_id.first].get_mesh_center(scene_id.second, 1);
        eng::Cut cut_cur(coord1, coord2);

        return intersect_cut(cut_cur, obj);
    }
};
