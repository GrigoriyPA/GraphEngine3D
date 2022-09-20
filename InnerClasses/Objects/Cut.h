#pragma once


class Cut : public RenderObject {
    void init(int& cuts_location) {
        if (cuts_location == -1) {
            GraphObject cut = get_cylinder(12, true, MAX_COUNT_MODELS);

            Material material;
            material.ambient = Vect3(INTERFACE_BORDER_COLOR) / 255;
            material.diffuse = Vect3(INTERFACE_BORDER_COLOR) / 255;
            material.specular = Vect3(INTERFACE_BORDER_COLOR) / 255;
            cut.set_material(material);

            scene_id.second = cut.add_model();

            cuts_location = scene->add_object(cut);
        }
        else {
            scene_id.second = (*scene)[cuts_location].add_model();
        }
        scene_id.first = cuts_location;
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

    void update_cut(Vect3 point1, Vect3 point2) {
        Vect3 direct = (point2 - point1).normalize();
        Vect3 horizont = direct.horizont();
        Vect3 vertical = direct ^ horizont;
        double length = (point2 - point1).length();

        (*scene)[scene_id.first].set_matrix(scale_matrix(Vect3(POINT_RADIUS * 0.2, length, POINT_RADIUS * 0.2)), scene_id.second);
        (*scene)[scene_id.first].change_matrix(Matrix(horizont, -direct, vertical), scene_id.second);
        (*scene)[scene_id.first].change_matrix(trans_matrix(point2), scene_id.second);
    }

    void update_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        
        update_cut(coord1, coord2);
    }

    void update_point_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        Vect3 coord1 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 1);

        update_cut(coord1.symmetry(coord_center), coord2.symmetry(coord_center));
    }

    void update_line_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        Vect3 coord_center1 = (*scene)[center.first].get_polygon_center(center.second, 0);
        Vect3 coord_center2 = (*scene)[center.first].get_polygon_center(center.second, 1);
        Line3 center_line(coord_center1, coord_center2);
        Vect3 coord1 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 1);

        update_cut(center_line.symmetry(coord1), center_line.symmetry(coord2));
    }

    void update_plane_symmetry(std::pair < int, int > cut, std::pair < int, int > center) {
        std::vector < Vect3 > center_coords = (*scene)[center.first].get_polygon_positions(center.second, 0);
        Flat center_plane(center_coords);
        Vect3 coord1 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 1);

        update_cut(center_plane.symmetry(coord1), center_plane.symmetry(coord2));
    }

    void update_translate(std::pair < int, int > cut, std::pair < int, int > start, std::pair < int, int > end) {
        Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        Vect3 translate = end_coord - start_coord;
        Vect3 coord1 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 1);

        update_cut(coord1 + translate, coord2 + translate);
    }

    RenderObject* intersect_cut(Cut3 cut_cur, RenderObject* cut, std::vector < int >& location) {
        Vect3 coord1 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 0);
        Vect3 coord2 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 1);
        Cut3 cut_ot(coord1, coord2);

        if (!cut_cur.is_intersect(cut_ot))
            return nullptr;

        RenderObject* point = new Point(cut_cur.intersect(cut_ot), location[0], scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { this, cut };

        return point;
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);
        Flat plane_proj(coords);
        Vect3 point1 = plane_proj.project_point((*scene)[scene_id.first].get_polygon_center(scene_id.second, 0));
        Vect3 point2 = plane_proj.project_point((*scene)[scene_id.first].get_polygon_center(scene_id.second, 1));

        update_cut(point1, point2);
    }

    void update_intersect() {
        std::vector < int > location(2, scene_id.first);
        RenderObject* cut = init_obj[0]->intersect(init_obj[1], location);

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
    Cut(Vect3 point1, Vect3 point2, int& cuts_location, GraphEngine* scene) {
        action = -1;
        type = 1;
        this->scene = scene;

        init(cuts_location);
        update_cut(point1, point2);
    }

    Cut(std::pair < int, int > button, std::vector < RenderObject* > init_obj, int& cuts_location, GraphEngine* scene) {
        type = 1;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init(cuts_location);
        update();
    }

    void switch_hide() {
        if (!hide)
            (*scene)[scene_id.first].central_scaling(Vect3(1.0 / 3.0, 1, 1.0 / 3.0), scene_id.second);
        else
            (*scene)[scene_id.first].central_scaling(Vect3(3, 1, 3), scene_id.second);
        hide ^= 1;
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

    RenderObject* intersect(RenderObject* obj, std::vector < int >& location) {
        if (obj->get_type() > type)
            return obj->intersect(this, location);

        Vect3 coord1 = (*scene)[scene_id.first].get_polygon_center(scene_id.second, 0);
        Vect3 coord2 = (*scene)[scene_id.first].get_polygon_center(scene_id.second, 1);
        Cut3 cut_cur(coord1, coord2);

        return intersect_cut(cut_cur, obj, location);
    }
};
