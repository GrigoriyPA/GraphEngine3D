#pragma once


class Line : public RenderObject {
    void init() {
        gre::GraphObject line = gre::GraphObject::cylinder(12, true, MAX_COUNT_MODELS);

        line.meshes.apply_func([](auto& mesh) {
            mesh.material.set_ambient(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_diffuse(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
            mesh.material.set_specular(gre::Vec3(INTERFACE_BORDER_COLOR) / 255);
        });

        scene_id.second = line.models.insert(gre::Matrix::one_matrix(4));

        scene_id.first = scene->objects.insert(line);
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

    void update_line(gre::Vec3 point1, gre::Vec3 point2) {
        if ((point1 - point2).length() < eps)
            point2 += gre::Vec3(1, 0, 0);

        gre::Vec3 direct = (point2 - point1).normalize();
        gre::Vec3 horizont = direct.horizont();
        gre::Vec3 vertical = direct ^ horizont;
        double length = (point2 - point1).length() * 100.0;

        (*scene).objects[scene_id.first].models.set(scene_id.second, gre::Matrix::scale_matrix(gre::Vec3(POINT_RADIUS * 0.2, length, POINT_RADIUS * 0.2)));
        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix(horizont, -direct, vertical));
        (*scene).objects[scene_id.first].models.change_left(scene_id.second, gre::Matrix::translation_matrix((point1 + point2 + direct * length) / 2));
    }

    void update_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        
        update_line(coord1, coord2);
    }

    void update_cut(std::pair < int, int > cut) {
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);

        update_line(coord1, coord2);
    }

    void update_perpendicular_to_line(std::pair < int, int > point, std::pair < int, int > line) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 point2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        gre::Line line_cur(point1, point2);

        gre::Vec3 proj_point = line_cur.project_point(coord);
        if (proj_point == coord)
            proj_point = coord + line_cur.get_direction().horizont();

        update_line(coord, proj_point);
    }

    void update_perpendicular_to_plane(std::pair < int, int > point, std::pair < int, int > plane) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Plane plane_cur(coords);

        update_line(coord, coord + plane_cur.get_normal());
    }

    void update_parallel_to_line(std::pair < int, int > point, std::pair < int, int > line) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 point2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        gre::Line line_cur(point1, point2);

        update_line(coord, coord + line_cur.get_direction());
    }

    void update_parallel_to_plans(std::pair < int, int > point, std::pair < int, int > plane1, std::pair < int, int > plane2) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        std::vector < gre::Vec3 > coords1 = (*scene).objects[plane1.first].get_mesh_positions(plane1.second, 0);
        std::vector < gre::Vec3 > coords2 = (*scene).objects[plane2.first].get_mesh_positions(plane2.second, 0);
        gre::Plane plane_cur1(coords1);
        gre::Plane plane_cur2(coords2);
        gre::Vec3 direction = (plane_cur1.get_normal() ^ plane_cur2.get_normal()).normalize();

        update_line(coord, coord + direction);
    }

    void update_point_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        gre::Vec3 coord_center = (*scene).objects[center.first].get_center(center.second);
        gre::Vec3 coord1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);

        update_line(coord1.symmetry(coord_center), coord2.symmetry(coord_center));
    }

    void update_line_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        gre::Vec3 coord_center1 = (*scene).objects[center.first].get_mesh_center(center.second, 0);
        gre::Vec3 coord_center2 = (*scene).objects[center.first].get_mesh_center(center.second, 1);
        gre::Line center_line(coord_center1, coord_center2);
        gre::Vec3 coord1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);

        update_line(center_line.symmetry(coord1), center_line.symmetry(coord2));
    }

    void update_plane_symmetry(std::pair < int, int > line, std::pair < int, int > center) {
        std::vector < gre::Vec3 > center_coords = (*scene).objects[center.first].get_mesh_positions(center.second, 0);
        gre::Plane center_plane(center_coords);
        gre::Vec3 coord1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);

        update_line(center_plane.symmetry(coord1), center_plane.symmetry(coord2));
    }

    void update_translate(std::pair < int, int > line, std::pair < int, int > start, std::pair < int, int > end) {
        gre::Vec3 start_coord = (*scene).objects[start.first].get_center(start.second);
        gre::Vec3 end_coord = (*scene).objects[end.first].get_center(end.second);
        gre::Vec3 translate = end_coord - start_coord;
        gre::Vec3 coord1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);

        update_line(coord1 + translate, coord2 + translate);
    }

    void update_bisector_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        gre::Vec3 coord3 = (*scene).objects[point3.first].get_center(point3.second);
        gre::Vec3 direction = ((coord1 - coord2).normalize() + (coord3 - coord2).normalize()) / 2;

        update_line(coord2, coord2 + direction);
    }

    void update_bisector_two_lines(std::pair < int, int > line1, std::pair < int, int > line2) {
        gre::Vec3 coord1 = (*scene).objects[line1.first].get_mesh_center(line1.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line1.first].get_mesh_center(line1.second, 1);
        gre::Vec3 coord3 = (*scene).objects[line2.first].get_mesh_center(line2.second, 0);
        gre::Vec3 coord4 = (*scene).objects[line2.first].get_mesh_center(line2.second, 1);
        gre::Line line_cur1(coord1, coord2);
        gre::Line line_cur2(coord3, coord4);

        gre::Vec3 direction1 = line_cur1.get_direction();
        gre::Vec3 direction2 = line_cur2.get_direction();
        if (direction1 * direction2 * special_coefficient < 0)
            direction2 *= -1;
        gre::Vec3 direction = (direction1 + direction2) / 2;

        gre::Vec3 intersection = line_cur1.intersect(line_cur2);
        intersection = (intersection + line_cur1.project_point(intersection)) / 2;

        update_line(intersection, intersection + direction);
    }

    void update_perpendicular_to_line_on_plane(std::pair < int, int > point, std::pair < int, int > line, std::pair < int, int > plane) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 point2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Line line_cur(point1, point2);
        gre::Plane plane_cur(coords);

        gre::Vec3 proj_point = line_cur.project_point(coord);
        if (proj_point == coord)
            proj_point = coord + (line_cur.get_direction() ^ plane_cur.get_normal());

        update_line(coord, proj_point);
    }

    void update_midperpendicular_two_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > plane) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Line line_cur(coord1, coord2);
        gre::Plane plane_cur(coords);
        gre::Vec3 direction = (line_cur.get_direction() ^ plane_cur.get_normal()).normalize();

        update_line((coord1 + coord2) / 2, (coord1 + coord2) / 2 + direction);
    }

    void update_midperpendicular_cut(std::pair < int, int > cut, std::pair < int, int > plane) {
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Line line_cur(coord1, coord2);
        gre::Plane plane_cur(coords);
        gre::Vec3 direction = (line_cur.get_direction() ^ plane_cur.get_normal()).normalize();

        update_line((coord1 + coord2) / 2, (coord1 + coord2) / 2 + direction);
    }

    RenderObject* intersect_cut(gre::Line line_cur, RenderObject* cut) {
        gre::Vec3 coord1 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        gre::Cut cut_ot(coord1, coord2);

        if (!cut_ot.is_intersect(line_cur))
            return nullptr;

        RenderObject* point = new Point(cut_ot.intersect(line_cur), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(gre::Line line_cur, RenderObject* line) {
        gre::Vec3 coord1 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 1);
        gre::Line line_ot(coord1, coord2);

        if (!line_cur.is_intersect(line_ot))
            return nullptr;

        RenderObject* point = new Point(line_cur.intersect(line_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { this, line };

        return point;
    }

    void update_plan_connect(std::pair < int, int > plane) {
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Plane plane_proj(coords);
        gre::Vec3 point1 = plane_proj.project_point((*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 0));
        gre::Vec3 point2 = plane_proj.project_point((*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 1));

        update_line(point1, point2);
    }

    void update_intersect() {
        RenderObject* line = init_obj[0]->intersect(init_obj[1]);

        if (line == nullptr) {
            if (init_obj[0]->get_type() == 3)
                update_plan_connect(init_obj[0]->scene_id);

            return;
        }

        (*scene).objects[scene_id.first].models.set(scene_id.second, (*scene).objects[line->scene_id.first].models[line->scene_id.second]);
        scene->objects.erase(line->scene_id.first, line->scene_id.second);
        delete line;
    }

public:
    Line(gre::Vec3 point1, gre::Vec3 point2, gre::GraphEngine* scene) {
        action = -1;
        type = 2;
        this->scene = scene;
        
        init();
        update_line(point1, point2);
    }

    Line(std::pair < int, int > button, std::vector < RenderObject* > init_obj, gre::GraphEngine* scene) {
        type = 2;
        this->scene = scene;
        this->init_obj = init_obj;

        set_action(button);
        init();
        update();
    }

    void switch_hide() {
        gre::Matrix model = (*scene).objects[scene_id.first].models[scene_id.second];
        if (!hide) {
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(1.0 / 3.0, 1.0, 1.0 / 3.0)) * model.inverse());
        } else {
            (*scene).objects[scene_id.first].models.change_left(scene_id.second, model * gre::Matrix::scale_matrix(gre::Vec3(3.0, 1.0, 3.0)) * model.inverse());
        }
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

        gre::Vec3 coord1 = (*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[scene_id.first].get_mesh_center(scene_id.second, 1);
        gre::Line line_cur(coord1, coord2);

        if (obj->get_type() == 1)
            return intersect_cut(line_cur, obj);
        if (obj->get_type() == 2)
            return intersect_line(line_cur, obj);
    }
};
