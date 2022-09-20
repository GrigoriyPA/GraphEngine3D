#pragma once


class Plane : public RenderObject {
    void init() {
        GraphObject plane(1);
        plane.set_border_bit(1);
        plane.transparent = true;

        int polygon_id = plane.add_polygon(4);
        plane[polygon_id].material.ambient = Vect3(INTERFACE_TEXT_COLOR) / 255;
        plane[polygon_id].material.diffuse = Vect3(INTERFACE_TEXT_COLOR) / 255;
        plane[polygon_id].material.specular = Vect3(INTERFACE_TEXT_COLOR) / 255;
        plane[polygon_id].material.shininess = 64;
        plane[polygon_id].material.alpha = 0.25;

        polygon_id = plane.add_polygon(plane[polygon_id]);

        scene_id.second = plane.add_model();
        scene_id.first = scene->add_object(plane);
    }

    void set_action(std::pair < int, int > button) {
        if (button.first == 1 && button.second == 3) {
            if (init_obj.size() == 2 && init_obj[1]->get_type() == 0)
                std::swap(init_obj[0], init_obj[1]);

            if (init_obj.size() == 3)
                action = 0;
            else if (init_obj[0]->get_type() == 0)
                action = 1;
            else
                action = 2;
        }
        else if (button.first == 3 && button.second == 3) {
            if (0 < init_obj[1]->get_type() && init_obj[1]->get_type() < 3)
                std::swap(init_obj[0], init_obj[1]);

            if (init_obj[1]->get_type() == 0)
                action = 3;
            else
                action = 4;
        }
        else if (button.first == 3 && button.second == 4) {
            if (init_obj[1]->get_type() == 0)
                std::swap(init_obj[0], init_obj[1]);
            else if (init_obj.size() == 3 && init_obj[2]->get_type() == 0)
                std::swap(init_obj[0], init_obj[2]);

            if (init_obj.size() == 2)
                action = 5;
            else
                action = 6;
        }
        else if (button.first == 3 && button.second == 5) {
            if (init_obj.size() == 2)
                action = 7;
            else
                action = 8;
        }
        else if (button.first == 4 && button.second == 0)
            action = 9;
        else if (button.first == 4 && button.second == 1)
            action = 10;
        else if (button.first == 4 && button.second == 2)
            action = 11;
        else if (button.first == 4 && button.second == 3)
            action = 12;
        else if (button.first == 5 && button.second == 0)
            action = 13;
    }

    void update_plane(std::vector < Vect3 > points) {
        Vect3 center = (points[0] + points[1] + points[2]) / 3;
        Vect3 normal = ((points[0] - points[1]) ^ (points[0] - points[2])).normalize();
        Vect3 horizont = normal.horizont();
        Vect3 vertical = normal ^ horizont;

        double k = 2.0, sz = 0;
        for (int i = 0; i < 3; i++) {
            int next = (i + 1) % 3;
            sz = std::max(sz, abs((points[i] - points[next]) * horizont));
            sz = std::max(sz, abs((points[i] - points[next]) * vertical));
        }
        horizont *= sz * k;
        vertical *= sz * k;

        (*scene)[scene_id.first][0].set_positions({
        center + horizont + vertical,
        center + horizont - vertical,
        center - horizont - vertical,
        center - horizont + vertical
            });

        (*scene)[scene_id.first][1].set_positions({
        center - horizont + vertical,
        center - horizont - vertical,
        center + horizont - vertical,
        center + horizont + vertical
            });

        (*scene)[scene_id.first].set_center();
    }

    void update_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        Vect3 coord3 = (*scene)[point3.first].get_center(point3.second);
        update_plane({ coord1, coord2, coord3 });
    }

    void update_point_line(std::pair < int, int > point, std::pair < int, int > line) {
        Vect3 coord1 = (*scene)[point.first].get_center(point.second);
        Vect3 coord2 = (*scene)[line.first].get_polygon_center(line.second, 0);
        Vect3 coord3 = (*scene)[line.first].get_polygon_center(line.second, 1);
        update_plane({ coord1, coord2, coord3 });
    }

    void update_two_lines(std::pair < int, int > line1, std::pair < int, int > line2) {
        Vect3 coord1 = (*scene)[line1.first].get_polygon_center(line1.second, 0);
        Vect3 coord2 = (*scene)[line1.first].get_polygon_center(line1.second, 1);
        Vect3 coord3 = (*scene)[line2.first].get_polygon_center(line2.second, 0);
        Vect3 coord4 = (*scene)[line2.first].get_polygon_center(line2.second, 1);
        Vect3 direct1 = (coord2 - coord1).normalize();
        Vect3 direct2 = (coord4 - coord3).normalize();

        if ((direct1 ^ direct2).length() > eps)
            update_plane({ coord3, coord4, coord3 + direct1 });
        else
            update_plane({ coord1, coord3, coord4 });
    }

    void update_perpendicular_to_line(std::pair < int, int > line, std::pair < int, int > point) {
        Vect3 coord = (*scene)[point.first].get_center(point.second);
        Vect3 point1 = (*scene)[line.first].get_polygon_center(line.second, 0);
        Vect3 point2 = (*scene)[line.first].get_polygon_center(line.second, 1);
        Line3 line_cur(point1, point2);
        Vect3 horizont = line_cur.get_direction().horizont();
        Vect3 vertical = horizont ^ line_cur.get_direction();

        update_plane({ coord, coord + horizont, coord + vertical });
    }

    void update_perpendicular_to_plane(std::pair < int, int > line, std::pair < int, int > plane) {
        Vect3 point1 = (*scene)[line.first].get_polygon_center(line.second, 0);
        Vect3 point2 = (*scene)[line.first].get_polygon_center(line.second, 1);
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);
        Line3 line_cur(point1, point2);
        Flat plane_cur(coords);

        Vect3 horizont = plane_cur.get_normal();
        if ((horizont ^ line_cur.get_direction()).length() < eps)
            horizont = horizont.horizont();

        update_plane({ point1, point2, point1 + horizont });
    }

    void update_parallel_to_plane(std::pair < int, int > point, std::pair < int, int > plane) {
        Vect3 coord = (*scene)[point.first].get_center(point.second);
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);
        Flat plane_cur(coords);
        Vect3 horizont = plane_cur.get_normal().horizont();
        Vect3 vertical = horizont ^ plane_cur.get_normal();

        update_plane({ coord, coord + horizont, coord + vertical });
    }

    void update_parallel_to_lines(std::pair < int, int > point, std::pair < int, int > line1, std::pair < int, int > line2) {
        Vect3 coord = (*scene)[point.first].get_center(point.second);
        Vect3 coord1 = (*scene)[line1.first].get_polygon_center(line1.second, 0);
        Vect3 coord2 = (*scene)[line1.first].get_polygon_center(line1.second, 1);
        Vect3 coord3 = (*scene)[line2.first].get_polygon_center(line2.second, 0);
        Vect3 coord4 = (*scene)[line2.first].get_polygon_center(line2.second, 1);
        Line3 line_cur1(coord1, coord2);
        Line3 line_cur2(coord3, coord4);
        Vect3 direction1 = line_cur1.get_direction();

        Vect3 direction2 = line_cur2.get_direction();
        if ((direction1 ^ direction2).length() < eps)
            direction2 = (line_cur2.p0 - line_cur1.p0).normalize();

        update_plane({ coord, coord + direction1, coord + direction2 });
    }

    void update_center_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        Vect3 normal = (coord2 - coord1).normalize();
        Vect3 horizont = normal.horizont();
        Vect3 vertical = normal ^ horizont;
        Vect3 p0 = (coord1 + coord2) / 2;

        update_plane({ p0, p0 + horizont, p0 + vertical });
    }

    void update_center_cut(std::pair < int, int > cut) {
        Vect3 coord1 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 1);
        Vect3 normal = (coord2 - coord1).normalize();
        Vect3 horizont = normal.horizont();
        Vect3 vertical = normal ^ horizont;
        Vect3 p0 = (coord1 + coord2) / 2;

        update_plane({ p0, p0 + horizont, p0 + vertical });
    }

    void update_point_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);

        coords.pop_back();
        for (Vect3& el : coords)
            el = el.symmetry(coord_center);

        update_plane(coords);
    }

    void update_line_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        Vect3 coord_center1 = (*scene)[center.first].get_polygon_center(center.second, 0);
        Vect3 coord_center2 = (*scene)[center.first].get_polygon_center(center.second, 1);
        Line3 center_line(coord_center1, coord_center2);
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);

        coords.pop_back();
        for (Vect3& el : coords)
            el = center_line.symmetry(el);

        update_plane(coords);
    }

    void update_plane_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        std::vector < Vect3 > center_coords = (*scene)[center.first].get_polygon_positions(center.second, 0);
        Flat center_plane(center_coords);
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);

        coords.pop_back();
        for (Vect3& el : coords)
            el = center_plane.symmetry(el);

        update_plane(coords);
    }

    void update_translate(std::pair < int, int > plane, std::pair < int, int > start, std::pair < int, int > end) {
        Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        Vect3 translate = end_coord - start_coord;
        std::vector < Vect3 > coords = (*scene)[plane.first].get_polygon_positions(plane.second, 0);

        coords.pop_back();
        for (Vect3& el : coords)
            el += translate;

        update_plane(coords);
    }

    void update_bisector(std::pair < int, int > plane1, std::pair < int, int > plane2) {
        std::vector < Vect3 > coords1 = (*scene)[plane1.first].get_polygon_positions(plane1.second, 0);
        std::vector < Vect3 > coords2 = (*scene)[plane2.first].get_polygon_positions(plane2.second, 0);
        Flat plane_cur1(coords1);
        Flat plane_cur2(coords2);

        std::vector < Vect3 > pos(3);
        for (int i = 0; i < 3; i++)
            pos[i] = (coords1[i] + coords2[i]) / 2;

        if (!plane_cur1.is_intersect(plane_cur2)) {
            update_plane(pos);
            return;
        }

        Line3 intersection = plane_cur1.intersect(plane_cur2);
        pos[0] = intersection.p0;
        pos[1] = intersection.p0 + intersection.get_direction();

        Vect3 direction1 = plane_cur1.get_normal() ^ intersection.get_direction();
        Vect3 direction2 = plane_cur2.get_normal() ^ intersection.get_direction();
        if (direction1 * direction2 * special_coefficient < 0)
            direction2 *= -1;
        pos[2] = intersection.p0 + (direction1 + direction2) / 2;

        update_plane(pos);
    }

    RenderObject* intersect_cut(Flat plane_cur, RenderObject* cut, std::vector < int >& location) {
        Vect3 coord1 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 0);
        Vect3 coord2 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 1);
        Cut3 cut_ot(coord1, coord2);

        if (!plane_cur.is_intersect(cut_ot))
            return nullptr;

        RenderObject* point = new Point(plane_cur.intersect(cut_ot), location[0], scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(Flat plane_cur, RenderObject* line, std::vector < int >& location) {
        Vect3 coord1 = (*scene)[line->scene_id.first].get_polygon_center(line->scene_id.second, 0);
        Vect3 coord2 = (*scene)[line->scene_id.first].get_polygon_center(line->scene_id.second, 1);
        Line3 line_ot(coord1, coord2);

        if (!plane_cur.is_intersect(line_ot))
            return nullptr;

        RenderObject* point = new Point(plane_cur.intersect(line_ot), location[0], scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { line, this };

        return point;
    }

    RenderObject* intersect_plane(Flat plane_cur, RenderObject* plane, std::vector < int >& location) {
        std::vector < Vect3 > coords = (*scene)[plane->scene_id.first].get_polygon_positions(plane->scene_id.second, 0);
        Flat plane_ot(coords);

        if (!plane_cur.is_intersect(plane_ot))
            return nullptr;

        Line3 intersection = plane_cur.intersect(plane_ot);

        RenderObject* line = new Line(intersection.p0, intersection.p0 + intersection.get_direction(), location[2], scene);
        line->action = 1;
        line->init_obj = { this, plane };

        return line;
    }
    
public:
    Plane(std::pair < int, int > button, std::vector < RenderObject* > init_obj, GraphEngine* scene) {
        type = 3;
        this->scene = scene;
        this->init_obj = init_obj;

        init();
        set_action(button);
        update();
    }

    void switch_hide() {
        Material material = (*scene)[scene_id.first][0].material;
        if (!hide)
            material.alpha = 0.1;
        else
            material.alpha = 0.25;
        (*scene)[scene_id.first].set_material(material);
        hide ^= 1;
    }

    void update() {
        if (action == 0)
            update_three_points(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 1)
            update_point_line(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 2)
            update_two_lines(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 3)
            update_perpendicular_to_line(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 4)
            update_perpendicular_to_plane(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 5)
            update_parallel_to_plane(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 6)
            update_parallel_to_lines(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 7)
            update_center_two_points(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 8)
            update_center_cut(init_obj[0]->scene_id);
        else if (action == 9)
            update_point_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 10)
            update_line_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 11)
            update_plane_symmetry(init_obj[0]->scene_id, init_obj[1]->scene_id);
        else if (action == 12)
            update_translate(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 13)
            update_bisector(init_obj[0]->scene_id, init_obj[1]->scene_id);
    }

    RenderObject* intersect(RenderObject* obj, std::vector < int >& location) {
        if (obj->get_type() > type)
            return obj->intersect(this, location);

        std::vector < Vect3 > coords = (*scene)[scene_id.first].get_polygon_positions(scene_id.second, 0);
        Flat plane_cur(coords);

        if (obj->get_type() == 1)
            return intersect_cut(plane_cur, obj, location);
        if (obj->get_type() == 2)
            return intersect_line(plane_cur, obj, location);
        if (obj->get_type() == 3)
            return intersect_plane(plane_cur, obj, location);
    }
};
