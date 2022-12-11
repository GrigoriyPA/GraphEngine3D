#pragma once


class Cut : public RenderObject {
    void init() {
        gre::GraphObject cut = gre::GraphObject::cylinder(12, true, MAX_COUNT_MODELS);

        cut.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_diffuse(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_specular(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
        });

        scene_id.second = cut.models.insert(gre::Matrix::one_matrix(4));

        scene_id.first = scene->objects.insert(cut);
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

    void update_cut(gre::Vec3 point1, gre::Vec3 point2) {
        gre::Vec3 direct = (point2 - point1).normalize();
        gre::Vec3 horizont = direct.horizont();
        gre::Vec3 vertical = direct ^ horizont;
        double length = (point2 - point1).length();

        (*scene).objects[scene_id.first].models.set(scene_id.second, gre::Matrix::scale_matrix(gre::Vec3(POINT_RADIUS * 0.2, length, POINT_RADIUS * 0.2)));
        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix(horizont, -direct, vertical));
        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix(point2));
    }

    void update_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        
        update_cut(coord1, coord2);
    }

    void update_point_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        gre::Vec3 coord_center = (*scene).objects[center.first].get_center(center.second);
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);

        update_cut(coord1.symmetry(coord_center), coord2.symmetry(coord_center));
    }

    void update_line_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        gre::Vec3 coord_center1 = (*scene).objects[center.first].get_mesh_center(center.second, 0);
        gre::Vec3 coord_center2 = (*scene).objects[center.first].get_mesh_center(center.second, 1);
        gre::Line center_line(coord_center1, coord_center2);
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);

        update_cut(center_line.symmetry(coord1), center_line.symmetry(coord2));
    }

    void update_plane_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        std::vector < gre::Vec3 > center_coords = (*scene).objects[center.first].get_mesh_positions(center.second, 0);
        gre::Plane center_plane(center_coords);
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);

        update_cut(center_plane.symmetry(coord1), center_plane.symmetry(coord2));
    }

    void update_translate(std::pair < int, int > cut, std::pair < int, int > start, std::pair < int, int > end) {
        gre::Vec3 start_coord = (*scene).objects[start.first].get_center(start.second);
        gre::Vec3 end_coord = (*scene).objects[end.first].get_center(end.second);
        gre::Vec3 translate = end_coord - start_coord;
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);

        update_cut(coord1 + translate, coord2 + translate);
    }

    RenderObject* intersect_cut(gre::Cut cut_cur, RenderObject* cut) {
        gre::Vec3 coord1 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        gre::Cut cut_ot(coord1, coord2);

        if (!cut_cur.is_intersect(cut_ot))
            return nullptr;

        RenderObject* point = new Point(cut_cur.intersect(cut_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { this, cut };

        return point;
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Plane plane_proj(coords);
        gre::Vec3 point1 = plane_proj.project_point((*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 0));
        gre::Vec3 point2 = plane_proj.project_point((*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 1));

        update_cut(point1, point2);
    }

    void update_intersect() {
        RenderObject* cut = init_obj[0]->intersect(init_obj[1]);

        if (cut == nullptr) {
            if (init_obj[0]->get_type() == 3)
                update_plan_connect(init_obj[0]->scene_id);

            return;
        }

        (*scene).objects[scene_id.first].models.set(scene_id.second, (*scene).objects[cut->scene_id.first].models[cut->scene_id.second]);
        scene->objects.erase(cut->scene_id.first, cut->scene_id.second);
        delete cut;
    }

public:
    Cut(gre::Vec3 point1, gre::Vec3 point2, gre::GraphEngine* scene) {
        action = -1;
        type = 1;
        this->scene = scene;

        init();
        update_cut(point1, point2);
    }

    Cut(std::pair < int, int > button, std::vector < RenderObject* > init_obj, gre::GraphEngine* scene) {
        type = 1;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        gre::Matrix model = (*scene).objects[scene_id.first].models[scene_id.second];
        if (!hide) {
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(1.0 / 3.0, 1, 1.0 / 3.0)) * model.inverse());
        } else
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(3.0, 1.0, 3.0)) * model.inverse());
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

        gre::Vec3 coord1 = (*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 1);
        gre::Cut cut_cur(coord1, coord2);

        return intersect_cut(cut_cur, obj);
    }
};
