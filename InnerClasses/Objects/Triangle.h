#pragma once


class Triangle : public RenderObject {
    void init() {
        eng::GraphObject triangle(1);

        eng::Mesh mesh(3);
        mesh.material.set_ambient(eng::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_diffuse(eng::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_specular(eng::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_shininess(64);
        triangle.meshes.insert(mesh);

        mesh = eng::Mesh(3);
        mesh.material.set_diffuse(eng::Vec3(INTERFACE_BORDER_COLOR) / 255);
        mesh.set_border_width(3);
        mesh.frame = true;
        triangle.meshes.insert(mesh);

        triangle.meshes.insert(mesh);

        scene_id.second = triangle.models.insert(eng::Matrix::one_matrix(4));
        scene_id.first = scene->objects.insert(triangle);
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

    void update_triangle(std::vector < eng::Vec3 > points) {
        eng::Vec3 normal = ((points[0] - points[1]) ^ (points[0] - points[2])).normalize();
        eng::Vec3 delt = normal * eps;

        (*scene).objects[scene_id.first].meshes.modify(0, (*scene).objects[scene_id.first].meshes.get(0).set_positions({
        points[0],
        points[1],
        points[2]
            }, true));

        (*scene).objects[scene_id.first].meshes.modify(1, (*scene).objects[scene_id.first].meshes.get(1).set_positions({
        points[0] - delt,
        points[1] - delt,
        points[2] - delt
            }, true));

        (*scene).objects[scene_id.first].meshes.modify(2, (*scene).objects[scene_id.first].meshes.get(2).set_positions({
        points[2] + delt,
        points[1] + delt,
        points[0] + delt
            }, true));
    }

    void update_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        eng::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        eng::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        eng::Vec3 coord3 = (*scene).objects[point3.first].get_center(point3.second);
        update_triangle({ coord1, coord2, coord3 });
    }

    void update_point_cut(std::pair < int, int > point, std::pair < int, int > cut) {
        eng::Vec3 coord1 = (*scene).objects[point.first].get_center(point.second);
        eng::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        eng::Vec3 coord3 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);
        update_triangle({ coord1, coord2, coord3 });
    }

    void update_point_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        eng::Vec3 coord_center = (*scene).objects[center.first].get_center(center.second);
        std::vector < eng::Vec3 > coords = (*scene).objects[triangle.first].get_mesh_positions(triangle.second, 0);

        std::reverse(coords.begin(), coords.end());
        for (eng::Vec3& el : coords)
            el = el.symmetry(coord_center);

        update_triangle(coords);
    }

    void update_line_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        eng::Vec3 coord_center1 = (*scene).objects[center.first].get_mesh_center(center.second, 0);
        eng::Vec3 coord_center2 = (*scene).objects[center.first].get_mesh_center(center.second, 1);
        eng::Line center_line(coord_center1, coord_center2);
        std::vector < eng::Vec3 > coords = (*scene).objects[triangle.first].get_mesh_positions(triangle.second, 0);

        for (eng::Vec3& el : coords)
            el = center_line.symmetry(el);

        update_triangle(coords);
    }

    void update_plane_symmetry(std::pair < int, int > triangle, std::pair < int, int > center) {
        std::vector < eng::Vec3 > center_coords = (*scene).objects[center.first].get_mesh_positions(center.second, 0);
        eng::Plane center_plane(center_coords);
        std::vector < eng::Vec3 > coords = (*scene).objects[triangle.first].get_mesh_positions(triangle.second, 0);

        std::reverse(coords.begin(), coords.end());
        for (eng::Vec3& el : coords)
            el = center_plane.symmetry(el);

        update_triangle(coords);
    }

    void update_translate(std::pair < int, int > triangle, std::pair < int, int > start, std::pair < int, int > end) {
        eng::Vec3 start_coord = (*scene).objects[start.first].get_center(start.second);
        eng::Vec3 end_coord = (*scene).objects[end.first].get_center(end.second);
        eng::Vec3 translate = end_coord - start_coord;
        std::vector < eng::Vec3 > coords = (*scene).objects[triangle.first].get_mesh_positions(triangle.second, 0);

        for (eng::Vec3& el : coords)
            el += translate;

        update_triangle(coords);
    }

    RenderObject* intersect_cut(std::vector < eng::Vec3 > triangle, RenderObject* cut) {
        eng::Vec3 coord1 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        eng::Vec3 coord2 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        eng::Cut cut_ot(coord1, coord2);
        eng::Plane plane(triangle);

        if (!plane.is_intersect(cut_ot))
            return nullptr;

        eng::Vec3 intersection = plane.intersect(cut_ot);
        if (!intersection.in_triangle(triangle[0], triangle[1], triangle[2]))
            return nullptr;

        RenderObject* point = new Point(intersection, scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(std::vector < eng::Vec3 > triangle, RenderObject* line) {
        eng::Vec3 coord1 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 0);
        eng::Vec3 coord2 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 1);
        eng::Line line_ot(coord1, coord2);
        eng::Plane plane(triangle);

        if (!plane.is_intersect(line_ot))
            return nullptr;

        eng::Vec3 intersection = plane.intersect(line_ot);
        if (!intersection.in_triangle(triangle[0], triangle[1], triangle[2]))
            return nullptr;

        RenderObject* point = new Point(intersection, scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { line, this };

        return point;
    }

    RenderObject* intersect_plane(std::vector < eng::Vec3 > triangle, RenderObject* plane) {
        std::vector < eng::Vec3 > coords = (*scene).objects[plane->scene_id.first].get_mesh_positions(plane->scene_id.second, 0);
        eng::Plane plane_ot(coords);
        eng::Plane plane_cur(triangle);

        if (!plane_cur.is_intersect(plane_ot))
            return nullptr;

        eng::Line intersection = plane_cur.intersect(plane_ot);

        std::vector < eng::Vec3 > intersect_coords;
        for (int i = 0; i < 3; i++) {
            int j = (i + 1) % 3;
            eng::Cut cut(triangle[i], triangle[j]);

            if (!cut.is_intersect(intersection))
                continue;

            eng::Vec3 point = cut.intersect(intersection);
            bool add = true;
            for (eng::Vec3 el : intersect_coords)
                add = add && el != point;

            if (add)
                intersect_coords.push_back(point);
        }
        if (intersect_coords.size() == 0)
            return nullptr;
        if (intersect_coords.size() == 1)
            intersect_coords.push_back(intersect_coords[0]);

        RenderObject* cut = new Cut(intersect_coords[0], intersect_coords[1], scene);
        cut->action = 1;
        cut->init_obj = { plane, this };

        return cut;
    }

public:
    Triangle(std::pair < int, int > button, std::vector < RenderObject* > init_obj, eng::GraphEngine* scene) {
        type = 4;
        this->scene = scene;
        this->init_obj = init_obj;

        init();
        set_action(button);
        update();
    }

    void switch_hide() {
        if (!hide) {
            (*scene).objects[scene_id.first].meshes.apply_func([](auto& mesh) { mesh.material.set_alpha(0.25); });
            (*scene).objects[scene_id.first].transparent = true;
        }
        else {
            (*scene).objects[scene_id.first].meshes.apply_func([](auto& mesh) { mesh.material.set_alpha(1); });
            (*scene).objects[scene_id.first].transparent = false;
        }
        hide ^= 1;
    }

    void set_border(bool flag) {
        if (flag) {
            (*scene).objects[scene_id.first].border_mask = 0b100;
        } else {
            (*scene).objects[scene_id.first].border_mask = 0;
        }
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

    RenderObject* intersect(RenderObject* obj) {
        if (obj->get_type() > type)
            return obj->intersect(this);

        std::vector < eng::Vec3 > coords = (*scene).objects[scene_id.first].get_mesh_positions(scene_id.second, 0);

        if (obj->get_type() == 1)
            return intersect_cut(coords, obj);
        if (obj->get_type() == 2)
            return intersect_line(coords, obj);
        if (obj->get_type() == 3)
            return intersect_plane(coords, obj);

        return nullptr;
    }
};
