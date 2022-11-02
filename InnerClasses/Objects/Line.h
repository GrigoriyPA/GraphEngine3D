#pragma once


class Line : public RenderObject {
    void init() {
        eng::GraphObject line = get_cylinder(12, true, MAX_COUNT_MODELS);

        line.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_diffuse(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_specular(eng::Vect3(INTERFACE_BORDER_COLOR) / 255);
        });

        scene_id.second = line.models.insert(eng::Matrix::one_matrix(4));

        scene_id.first = scene->add_object(line);
    }

    void set_action(std::pair < int, int > button) {
        if (button.first == 1 && button.second == 2) {
            if (init_obj.size() == 1)
                action = 2;
            else
                action = 0;
        }
        else if (button.first == 3 && button.second == 1) {
            if (init_obj[1]->get_type() == 0)
                std::swap(init_obj[0], init_obj[1]);

            if (init_obj[1]->get_type() < 3)
                action = 3;
            else
                action = 4;
        }
        else if (button.first == 3 && button.second == 2) {
            if (init_obj[1]->get_type() == 0)
                std::swap(init_obj[0], init_obj[1]);
            else if (init_obj.size() == 3 && init_obj[2]->get_type() == 0)
                std::swap(init_obj[0], init_obj[2]);

            if (init_obj.size() == 2)
                action = 5;
            else
                action = 6;
        }
        else if (button.first == 4 && button.second == 0)
            action = 7;
        else if (button.first == 4 && button.second == 1)
            action = 8;
        else if (button.first == 4 && button.second == 2)
            action = 9;
        else if (button.first == 4 && button.second == 3)
            action = 10;
        else if (button.first == 5 && button.second == 0) {
            if (init_obj.size() == 3)
                action = 11;
            else
                action = 12;
        }
        else if (button.first == 5 && button.second == 1) {
            std::sort(init_obj.begin(), init_obj.end(), [&](RenderObject* a, RenderObject* b) {
                return a->get_type() < b->get_type();
            });
            action = 13;
        }
        else if (button.first == 5 && button.second == 2) {
            std::sort(init_obj.begin(), init_obj.end(), [&](RenderObject* a, RenderObject* b) {
                return a->get_type() < b->get_type();
            });
            if (init_obj.size() == 3)
                action = 14;
            else
                action = 15;
        }
    }

    void update_line(eng::Vect3 point1, eng::Vect3 point2) {
        if ((point1 - point2).length() < eps)
            point2 += eng::Vect3(1, 0, 0);

        eng::Vect3 direct = (point2 - point1).normalize();
        eng::Vect3 horizont = direct.horizont();
        eng::Vect3 vertical = direct ^ horizont;
        double length = (point2 - point1).length() * 100.0;

        (*scene)[scene_id.first].models.set(scene_id.second, eng::Matrix::scale_matrix(eng::Vect3(POINT_RADIUS * 0.2, length, POINT_RADIUS * 0.2)));
        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix(horizont, -direct, vertical));
        (*scene)[scene_id.first].models.change_left(scene_id.second, eng::Matrix::translation_matrix((point1 + point2 + direct * length) / 2));
    }

    void update_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        
        update_line(coord1, coord2);
    }

    void update_cut(std::pair < int, int > cut) {
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);

        update_line(coord1, coord2);
    }

    void update_perpendicular_to_line(std::pair < int, int > point, std::pair < int, int > line) {
        eng::Vect3 coord = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 point2 = (*scene)[line.first].get_mesh_center(line.second, 1);
        eng::Line line_cur(point1, point2);

        eng::Vect3 proj_point = line_cur.project_point(coord);
        if (proj_point == coord)
            proj_point = coord + line_cur.get_direction().horizont();

        update_line(coord, proj_point);
    }

    void update_perpendicular_to_plane(std::pair < int, int > point, std::pair < int, int > plane) {
        eng::Vect3 coord = (*scene)[point.first].get_center(point.second);
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Plane plane_cur(coords);

        update_line(coord, coord + plane_cur.get_normal());
    }

    void update_parallel_to_line(std::pair < int, int > point, std::pair < int, int > line) {
        eng::Vect3 coord = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 point2 = (*scene)[line.first].get_mesh_center(line.second, 1);
        eng::Line line_cur(point1, point2);

        update_line(coord, coord + line_cur.get_direction());
    }

    void update_parallel_to_plans(std::pair < int, int > point, std::pair < int, int > plane1, std::pair < int, int > plane2) {
        eng::Vect3 coord = (*scene)[point.first].get_center(point.second);
        std::vector < eng::Vect3 > coords1 = (*scene)[plane1.first].get_mesh_positions(plane1.second, 0);
        std::vector < eng::Vect3 > coords2 = (*scene)[plane2.first].get_mesh_positions(plane2.second, 0);
        eng::Plane plane_cur1(coords1);
        eng::Plane plane_cur2(coords2);
        eng::Vect3 direction = (plane_cur1.get_normal() ^ plane_cur2.get_normal()).normalize();

        update_line(coord, coord + direction);
    }

    void update_point_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        eng::Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        eng::Vect3 coord1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 coord2 = (*scene)[line.first].get_mesh_center(line.second, 1);

        update_line(coord1.symmetry(coord_center), coord2.symmetry(coord_center));
    }

    void update_line_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        eng::Vect3 coord_center1 = (*scene)[center.first].get_mesh_center(center.second, 0);
        eng::Vect3 coord_center2 = (*scene)[center.first].get_mesh_center(center.second, 1);
        eng::Line center_line(coord_center1, coord_center2);
        eng::Vect3 coord1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 coord2 = (*scene)[line.first].get_mesh_center(line.second, 1);

        update_line(center_line.symmetry(coord1), center_line.symmetry(coord2));
    }

    void update_plane_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        std::vector < eng::Vect3 > center_coords = (*scene)[center.first].get_mesh_positions(center.second, 0);
        eng::Plane center_plane(center_coords);
        eng::Vect3 coord1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 coord2 = (*scene)[line.first].get_mesh_center(line.second, 1);

        update_line(center_plane.symmetry(coord1), center_plane.symmetry(coord2));
    }

    void update_translate(std::pair < int, int > line, std::pair < int, int > start, std::pair < int, int > end) {
        eng::Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        eng::Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        eng::Vect3 translate = end_coord - start_coord;
        eng::Vect3 coord1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 coord2 = (*scene)[line.first].get_mesh_center(line.second, 1);

        update_line(coord1 + translate, coord2 + translate);
    }

    void update_bisector_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        eng::Vect3 coord3 = (*scene)[point3.first].get_center(point3.second);
        eng::Vect3 direction = ((coord1 - coord2).normalize() + (coord3 - coord2).normalize()) / 2;

        update_line(coord2, coord2 + direction);
    }

    void update_bisector_two_lines(std::pair < int, int > line1, std::pair < int, int > line2) {
        eng::Vect3 coord1 = (*scene)[line1.first].get_mesh_center(line1.second, 0);
        eng::Vect3 coord2 = (*scene)[line1.first].get_mesh_center(line1.second, 1);
        eng::Vect3 coord3 = (*scene)[line2.first].get_mesh_center(line2.second, 0);
        eng::Vect3 coord4 = (*scene)[line2.first].get_mesh_center(line2.second, 1);
        eng::Line line_cur1(coord1, coord2);
        eng::Line line_cur2(coord3, coord4);

        eng::Vect3 direction1 = line_cur1.get_direction();
        eng::Vect3 direction2 = line_cur2.get_direction();
        if (direction1 * direction2 * special_coefficient < 0)
            direction2 *= -1;
        eng::Vect3 direction = (direction1 + direction2) / 2;

        eng::Vect3 intersection = line_cur1.intersect(line_cur2);
        intersection = (intersection + line_cur1.project_point(intersection)) / 2;

        update_line(intersection, intersection + direction);
    }

    void update_perpendicular_to_line_on_plane(std::pair < int, int > point, std::pair < int, int > line, std::pair < int, int > plane) {
        eng::Vect3 coord = (*scene)[point.first].get_center(point.second);
        eng::Vect3 point1 = (*scene)[line.first].get_mesh_center(line.second, 0);
        eng::Vect3 point2 = (*scene)[line.first].get_mesh_center(line.second, 1);
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Line line_cur(point1, point2);
        eng::Plane plane_cur(coords);

        eng::Vect3 proj_point = line_cur.project_point(coord);
        if (proj_point == coord)
            proj_point = coord + (line_cur.get_direction() ^ plane_cur.get_normal());

        update_line(coord, proj_point);
    }

    void update_midperpendicular_two_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > plane) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Line line_cur(coord1, coord2);
        eng::Plane plane_cur(coords);
        eng::Vect3 direction = (line_cur.get_direction() ^ plane_cur.get_normal()).normalize();

        update_line((coord1 + coord2) / 2, (coord1 + coord2) / 2 + direction);
    }

    void update_midperpendicular_cut(std::pair < int, int > cut, std::pair < int, int > plane) {
        eng::Vect3 coord1 = (*scene)[cut.first].get_mesh_center(cut.second, 0);
        eng::Vect3 coord2 = (*scene)[cut.first].get_mesh_center(cut.second, 1);
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Line line_cur(coord1, coord2);
        eng::Plane plane_cur(coords);
        eng::Vect3 direction = (line_cur.get_direction() ^ plane_cur.get_normal()).normalize();

        update_line((coord1 + coord2) / 2, (coord1 + coord2) / 2 + direction);
    }

    RenderObject* intersect_cut(eng::Line line_cur, RenderObject* cut) {
        eng::Vect3 coord1 = (*scene)[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        eng::Cut cut_ot(coord1, coord2);

        if (!cut_ot.is_intersect(line_cur))
            return nullptr;

        RenderObject* point = new Point(cut_ot.intersect(line_cur), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(eng::Line line_cur, RenderObject* line) {
        eng::Vect3 coord1 = (*scene)[line->scene_id.first].get_mesh_center(line->scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[line->scene_id.first].get_mesh_center(line->scene_id.second, 1);
        eng::Line line_ot(coord1, coord2);

        if (!line_cur.is_intersect(line_ot))
            return nullptr;

        RenderObject* point = new Point(line_cur.intersect(line_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { this, line };

        return point;
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < eng::Vect3 > coords = (*scene)[plane.first].get_mesh_positions(plane.second, 0);
        eng::Plane plane_proj(coords);
        eng::Vect3 point1 = plane_proj.project_point((*scene)[scene_id.first].get_mesh_center(scene_id.second, 0));
        eng::Vect3 point2 = plane_proj.project_point((*scene)[scene_id.first].get_mesh_center(scene_id.second, 1));

        update_line(point1, point2);
    }

    void update_intersect() {
        RenderObject* line = init_obj[0]->intersect(init_obj[1]);

        if (line == nullptr) {
            if (init_obj[0]->get_type() == 3)
                update_plan_connect(init_obj[0]->scene_id);

            return;
        }

        (*scene)[scene_id.first].models.set(scene_id.second, (*scene)[line->scene_id.first].models[line->scene_id.second]);
        scene->delete_object(line->scene_id.first, line->scene_id.second);
        delete line;
    }

public:
    Line(eng::Vect3 point1, eng::Vect3 point2, GraphEngine* scene) {
        action = -1;
        type = 2;
        this->scene = scene;
        
        init();
        update_line(point1, point2);
    }

    Line(std::pair < int, int > button, std::vector < RenderObject* > init_obj, GraphEngine* scene) {
        type = 2;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        eng::Matrix model = (*scene)[scene_id.first].models[scene_id.second];
        if (!hide) {
            (*scene)[scene_id.first].models.change_left(scene_id.second, model * eng::Matrix::scale_matrix(eng::Vect3(1.0 / 3.0, 1.0, 1.0 / 3.0)) * model.inverse());
        } else {
            (*scene)[scene_id.first].models.change_left(scene_id.second, model * eng::Matrix::scale_matrix(eng::Vect3(3.0, 1.0, 3.0)) * model.inverse());
        }
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
            update_cut(init_obj[0]->scene_id);
        else if (action == 3)
            update_perpendicular_to_line(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 4)
            update_perpendicular_to_plane(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 5)
            update_parallel_to_line(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 6)
            update_parallel_to_plans(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 7)
            update_point_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 8)
            update_line_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 9)
            update_plane_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 10)
            update_translate(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 11)
            update_bisector_three_points(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 12)
            update_bisector_two_lines(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 13)
            update_perpendicular_to_line_on_plane(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 14)
            update_midperpendicular_two_points(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 15)
            update_midperpendicular_cut(init_obj[0]->scene_id, init_obj[1]->scene_id);
    }

    RenderObject* intersect(RenderObject* obj) {
        if (obj->get_type() > type)
            return obj->intersect(this);

        eng::Vect3 coord1 = (*scene)[scene_id.first].get_mesh_center(scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[scene_id.first].get_mesh_center(scene_id.second, 1);
        eng::Line line_cur(coord1, coord2);

        if (obj->get_type() == 1)
            return intersect_cut(line_cur, obj);
        if (obj->get_type() == 2)
            return intersect_line(line_cur, obj);
    }
};
