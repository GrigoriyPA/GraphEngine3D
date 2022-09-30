#pragma once


class Triangle : public RenderObject {
    void init() {
        GraphObject triangle(1);
        triangle.set_border_bit(2);

        int polygon_id = triangle.add_polygon(3);
        triangle[polygon_id].material.ambient = eng::Vect3(INTERFACE_TEXT_COLOR) / 255;
        triangle[polygon_id].material.diffuse = eng::Vect3(INTERFACE_TEXT_COLOR) / 255;
        triangle[polygon_id].material.specular = eng::Vect3(INTERFACE_TEXT_COLOR) / 255;
        triangle[polygon_id].material.shininess = 64;

        polygon_id = triangle.add_polygon(3);
        triangle[polygon_id].material.diffuse = eng::Vect3(INTERFACE_BORDER_COLOR) / 255;
        triangle[polygon_id].border_width = 3;
        triangle[polygon_id].frame = true;

        polygon_id = triangle.add_polygon(triangle[polygon_id]);

        scene_id.second = triangle.add_model();
        scene_id.first = scene->add_object(triangle);
    }

    void set_action(std::pair < int, int > button) {
        if (button.first == 1 && button.second == 4) {
            if (init_obj.size() == 2 && init_obj[1]->get_type() == 0)
                std::swap(init_obj[0], init_obj[1]);

            if (init_obj.size() == 3)
                action = 0;
            else
                action = 1;
        }
        else if (button.first == 4 && button.second == 0)
            action = 2;
        else if (button.first == 4 && button.second == 1)
            action = 3;
        else if (button.first == 4 && button.second == 2)
            action = 4;
        else if (button.first == 4 && button.second == 3)
            action = 5;
    }

    void update_triangle(std::vector < eng::Vect3 > points) {
        eng::Vect3 normal = ((points[0] - points[1]) ^ (points[0] - points[2])).normalize();
        eng::Vect3 delt = normal * eps;

        (*scene)[scene_id.first][0].set_positions({
        points[0],
        points[1],
        points[2]
            });

        (*scene)[scene_id.first][1].set_positions({
        points[0] - delt,
        points[1] - delt,
        points[2] - delt
            });

        (*scene)[scene_id.first][2].set_positions({
        points[2] + delt,
        points[1] + delt,
        points[0] + delt
            });

        (*scene)[scene_id.first].set_center();
    }

    void update_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        eng::Vect3 coord1 = (*scene)[point1.first].get_center(point1.second);
        eng::Vect3 coord2 = (*scene)[point2.first].get_center(point2.second);
        eng::Vect3 coord3 = (*scene)[point3.first].get_center(point3.second);
        update_triangle({ coord1, coord2, coord3 });
    }

    void update_point_cut(std::pair < int, int > point, std::pair < int, int > cut) {
        eng::Vect3 coord1 = (*scene)[point.first].get_center(point.second);
        eng::Vect3 coord2 = (*scene)[cut.first].get_polygon_center(cut.second, 0);
        eng::Vect3 coord3 = (*scene)[cut.first].get_polygon_center(cut.second, 1);
        update_triangle({ coord1, coord2, coord3 });
    }

    void update_point_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        eng::Vect3 coord_center = (*scene)[center.first].get_center(center.second);
        std::vector < eng::Vect3 > coords = (*scene)[triangle.first].get_polygon_positions(triangle.second, 0);

        std::reverse(coords.begin(), coords.end());
        for (eng::Vect3& el : coords)
            el = el.symmetry(coord_center);

        update_triangle(coords);
    }

    void update_line_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        eng::Vect3 coord_center1 = (*scene)[center.first].get_polygon_center(center.second, 0);
        eng::Vect3 coord_center2 = (*scene)[center.first].get_polygon_center(center.second, 1);
        Line3 center_line(coord_center1, coord_center2);
        std::vector < eng::Vect3 > coords = (*scene)[triangle.first].get_polygon_positions(triangle.second, 0);

        for (eng::Vect3& el : coords)
            el = center_line.symmetry(el);

        update_triangle(coords);
    }

    void update_plane_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        std::vector < eng::Vect3 > center_coords = (*scene)[center.first].get_polygon_positions(center.second, 0);
        Flat center_plane(center_coords);
        std::vector < eng::Vect3 > coords = (*scene)[triangle.first].get_polygon_positions(triangle.second, 0);

        std::reverse(coords.begin(), coords.end());
        for (eng::Vect3& el : coords)
            el = center_plane.symmetry(el);

        update_triangle(coords);
    }

    void update_translate(std::pair < int, int > triangle, std::pair < int, int > start, std::pair < int, int > end) {
        eng::Vect3 start_coord = (*scene)[start.first].get_center(start.second);
        eng::Vect3 end_coord = (*scene)[end.first].get_center(end.second);
        eng::Vect3 translate = end_coord - start_coord;
        std::vector < eng::Vect3 > coords = (*scene)[triangle.first].get_polygon_positions(triangle.second, 0);

        for (eng::Vect3& el : coords)
            el += translate;

        update_triangle(coords);
    }

    RenderObject* intersect_cut(std::vector < eng::Vect3 > triangle, RenderObject* cut, std::vector < int >& location) {
        eng::Vect3 coord1 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[cut->scene_id.first].get_polygon_center(cut->scene_id.second, 1);
        Cut3 cut_ot(coord1, coord2);
        Flat plane(triangle);

        if (!plane.is_intersect(cut_ot))
            return nullptr;

        eng::Vect3 intersection = plane.intersect(cut_ot);
        if (!intersection.in_triangle(triangle[0], triangle[1], triangle[2]))
            return nullptr;

        RenderObject* point = new Point(intersection, location[0], scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(std::vector < eng::Vect3 > triangle, RenderObject* line, std::vector < int >& location) {
        eng::Vect3 coord1 = (*scene)[line->scene_id.first].get_polygon_center(line->scene_id.second, 0);
        eng::Vect3 coord2 = (*scene)[line->scene_id.first].get_polygon_center(line->scene_id.second, 1);
        Line3 line_ot(coord1, coord2);
        Flat plane(triangle);

        if (!plane.is_intersect(line_ot))
            return nullptr;

        eng::Vect3 intersection = plane.intersect(line_ot);
        if (!intersection.in_triangle(triangle[0], triangle[1], triangle[2]))
            return nullptr;

        RenderObject* point = new Point(intersection, location[0], scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { line, this };

        return point;
    }

    RenderObject* intersect_plane(std::vector < eng::Vect3 > triangle, RenderObject* plane, std::vector < int >& location) {
        std::vector < eng::Vect3 > coords = (*scene)[plane->scene_id.first].get_polygon_positions(plane->scene_id.second, 0);
        Flat plane_ot(coords);
        Flat plane_cur(triangle);

        if (!plane_cur.is_intersect(plane_ot))
            return nullptr;

        Line3 intersection = plane_cur.intersect(plane_ot);

        std::vector < eng::Vect3 > intersect_coords;
        for (int i = 0; i < 3; i++) {
            int j = (i + 1) % 3;
            Cut3 cut(triangle[i], triangle[j]);

            if (!cut.is_intersect(intersection))
                continue;

            eng::Vect3 point = cut.intersect(intersection);
            bool add = true;
            for (eng::Vect3 el : intersect_coords)
                add = add && el != point;

            if (add)
                intersect_coords.push_back(point);
        }
        if (intersect_coords.size() == 0)
            return nullptr;
        if (intersect_coords.size() == 1)
            intersect_coords.push_back(intersect_coords[0]);

        RenderObject* cut = new Cut(intersect_coords[0], intersect_coords[1], location[1], scene);
        cut->action = 1;
        cut->init_obj = { plane, this };

        return cut;
    }

public:
    Triangle(std::pair < int, int > button, std::vector < RenderObject* > init_obj, GraphEngine* scene) {
        type = 4;
        this->scene = scene;
        this->init_obj = init_obj;

        init();
        set_action(button);
        update();
    }

    void switch_hide() {
        if (!hide) {
            (*scene)[scene_id.first][0].material.alpha = 0.25;
            (*scene)[scene_id.first][1].material.alpha = 0.25;
            (*scene)[scene_id.first][2].material.alpha = 0.25;
            (*scene)[scene_id.first].transparent = true;
        }
        else {
            (*scene)[scene_id.first][0].material.alpha = 1;
            (*scene)[scene_id.first][1].material.alpha = 1;
            (*scene)[scene_id.first][2].material.alpha = 1;
            (*scene)[scene_id.first].transparent = false;
        }
        hide ^= 1;
    }

    void update() {
        if (action == 0)
            update_three_points(init_obj[0]->scene_id, init_obj[1]->scene_id, init_obj[2]->scene_id);
        else if (action == 1)
            update_point_cut(init_obj[0]->scene_id, init_obj[1]->scene_id);
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

        std::vector < eng::Vect3 > coords = (*scene)[scene_id.first].get_polygon_positions(scene_id.second, 0);

        if (obj->get_type() == 1)
            return intersect_cut(coords, obj, location);
        if (obj->get_type() == 2)
            return intersect_line(coords, obj, location);
        if (obj->get_type() == 3)
            return intersect_plane(coords, obj, location);

        return nullptr;
    }
};
