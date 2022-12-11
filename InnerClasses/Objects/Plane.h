#pragma once


class Plane : public RenderObject {
    void init() {
        gre::GraphObject plane(1);
        plane.transparent = true;

        gre::Mesh mesh(4);
        mesh.material.set_ambient(gre::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_diffuse(gre::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_specular(gre::Vec3(INTERFACE_TEXT_COLOR) / 255);
        mesh.material.set_shininess(64);
        mesh.material.set_alpha(0.25);
        plane.meshes.insert(mesh);

        plane.meshes.insert(mesh);

        scene_id.second = plane.models.insert(gre::Matrix::one_matrix(4));
        scene_id.first = scene->objects.insert(plane);
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

    void update_plane(std::vector < gre::Vec3 > points) {
        gre::Vec3 center = (points[0] + points[1] + points[2]) / 3;
        gre::Vec3 normal = ((points[0] - points[1]) ^ (points[0] - points[2])).normalize();
        gre::Vec3 horizont = normal.horizont();
        gre::Vec3 vertical = normal ^ horizont;

        double k = 2.0, sz = 0;
        for (int i = 0; i < 3; i++) {
            int next = (i + 1) % 3;
            sz = std::max(sz, abs((points[i] - points[next]) * horizont));
            sz = std::max(sz, abs((points[i] - points[next]) * vertical));
        }
        horizont *= sz * k;
        vertical *= sz * k;

        (*scene).objects[scene_id.first].meshes.modify(0, (*scene).objects[scene_id.first].meshes.get(0).set_positions({
        center + horizont + vertical,
        center + horizont - vertical,
        center - horizont - vertical,
        center - horizont + vertical
            }, true));

        (*scene).objects[scene_id.first].meshes.modify(1, (*scene).objects[scene_id.first].meshes.get(1).set_positions({
        center - horizont + vertical,
        center - horizont - vertical,
        center + horizont - vertical,
        center + horizont + vertical
            }, true));
    }

    void update_three_points(std::pair < int, int > point1, std::pair < int, int > point2, std::pair < int, int > point3) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        gre::Vec3 coord3 = (*scene).objects[point3.first].get_center(point3.second);
        update_plane({ coord1, coord2, coord3 });
    }

    void update_point_line(std::pair < int, int > point, std::pair < int, int > line) {
        gre::Vec3 coord1 = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 coord2 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 coord3 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        update_plane({ coord1, coord2, coord3 });
    }

    void update_two_lines(std::pair < int, int > line1, std::pair < int, int > line2) {
        gre::Vec3 coord1 = (*scene).objects[line1.first].get_mesh_center(line1.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line1.first].get_mesh_center(line1.second, 1);
        gre::Vec3 coord3 = (*scene).objects[line2.first].get_mesh_center(line2.second, 0);
        gre::Vec3 coord4 = (*scene).objects[line2.first].get_mesh_center(line2.second, 1);
        gre::Vec3 direct1 = (coord2 - coord1).normalize();
        gre::Vec3 direct2 = (coord4 - coord3).normalize();

        if ((direct1 ^ direct2).length() > eps)
            update_plane({ coord3, coord4, coord3 + direct1 });
        else
            update_plane({ coord1, coord3, coord4 });
    }

    void update_perpendicular_to_line(std::pair < int, int > line, std::pair < int, int > point) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 point1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 point2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        gre::Line line_cur(point1, point2);
        gre::Vec3 horizont = line_cur.get_direction().horizont();
        gre::Vec3 vertical = horizont ^ line_cur.get_direction();

        update_plane({ coord, coord + horizont, coord + vertical });
    }

    void update_perpendicular_to_plane(std::pair < int, int > line, std::pair < int, int > plane) {
        gre::Vec3 point1 = (*scene).objects[line.first].get_mesh_center(line.second, 0);
        gre::Vec3 point2 = (*scene).objects[line.first].get_mesh_center(line.second, 1);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Line line_cur(point1, point2);
        gre::Plane plane_cur(coords);

        gre::Vec3 horizont = plane_cur.get_normal();
        if ((horizont ^ line_cur.get_direction()).length() < eps)
            horizont = horizont.horizont();

        update_plane({ point1, point2, point1 + horizont });
    }

    void update_parallel_to_plane(std::pair < int, int > point, std::pair < int, int > plane) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        std::vector <gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);
        gre::Plane plane_cur(coords);
        gre::Vec3 horizont = plane_cur.get_normal().horizont();
        gre::Vec3 vertical = horizont ^ plane_cur.get_normal();

        update_plane({ coord, coord + horizont, coord + vertical });
    }

    void update_parallel_to_lines(std::pair < int, int > point, std::pair < int, int > line1, std::pair < int, int > line2) {
        gre::Vec3 coord = (*scene).objects[point.first].get_center(point.second);
        gre::Vec3 coord1 = (*scene).objects[line1.first].get_mesh_center(line1.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line1.first].get_mesh_center(line1.second, 1);
        gre::Vec3 coord3 = (*scene).objects[line2.first].get_mesh_center(line2.second, 0);
        gre::Vec3 coord4 = (*scene).objects[line2.first].get_mesh_center(line2.second, 1);
        gre::Line line_cur1(coord1, coord2);
        gre::Line line_cur2(coord3, coord4);
        gre::Vec3 direction1 = line_cur1.get_direction();

        gre::Vec3 direction2 = line_cur2.get_direction();
        if ((direction1 ^ direction2).length() < eps)
            direction2 = (line_cur2.start_point - line_cur1.start_point).normalize();

        update_plane({ coord, coord + direction1, coord + direction2 });
    }

    void update_center_two_points(std::pair < int, int > point1, std::pair < int, int > point2) {
        gre::Vec3 coord1 = (*scene).objects[point1.first].get_center(point1.second);
        gre::Vec3 coord2 = (*scene).objects[point2.first].get_center(point2.second);
        gre::Vec3 normal = (coord2 - coord1).normalize();
        gre::Vec3 horizont = normal.horizont();
        gre::Vec3 vertical = normal ^ horizont;
        gre::Vec3 p0 = (coord1 + coord2) / 2;

        update_plane({ p0, p0 + horizont, p0 + vertical });
    }

    void update_center_cut(std::pair < int, int > cut) {
        gre::Vec3 coord1 = (*scene).objects[cut.first].get_mesh_center(cut.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut.first].get_mesh_center(cut.second, 1);
        gre::Vec3 normal = (coord2 - coord1).normalize();
        gre::Vec3 horizont = normal.horizont();
        gre::Vec3 vertical = normal ^ horizont;
        gre::Vec3 p0 = (coord1 + coord2) / 2;

        update_plane({ p0, p0 + horizont, p0 + vertical });
    }

    void update_point_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        gre::Vec3 coord_center = (*scene).objects[center.first].get_center(center.second);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);

        coords.pop_back();
        for (gre::Vec3& el : coords)
            el = el.symmetry(coord_center);

        update_plane(coords);
    }

    void update_line_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        gre::Vec3 coord_center1 = (*scene).objects[center.first].get_mesh_center(center.second, 0);
        gre::Vec3 coord_center2 = (*scene).objects[center.first].get_mesh_center(center.second, 1);
        gre::Line center_line(coord_center1, coord_center2);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);

        coords.pop_back();
        for (gre::Vec3& el : coords)
            el = center_line.symmetry(el);

        update_plane(coords);
    }

    void update_plane_symmetry(std::pair < int, int > plane, std::pair < int, int > center) {
        std::vector < gre::Vec3 > center_coords = (*scene).objects[center.first].get_mesh_positions(center.second, 0);
        gre::Plane center_plane(center_coords);
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);

        coords.pop_back();
        for (gre::Vec3& el : coords)
            el = center_plane.symmetry(el);

        update_plane(coords);
    }

    void update_translate(std::pair < int, int > plane, std::pair < int, int > start, std::pair < int, int > end) {
        gre::Vec3 start_coord = (*scene).objects[start.first].get_center(start.second);
        gre::Vec3 end_coord = (*scene).objects[end.first].get_center(end.second);
        gre::Vec3 translate = end_coord - start_coord;
        std::vector < gre::Vec3 > coords = (*scene).objects[plane.first].get_mesh_positions(plane.second, 0);

        coords.pop_back();
        for (gre::Vec3& el : coords)
            el += translate;

        update_plane(coords);
    }

    void update_bisector(std::pair < int, int > plane1, std::pair < int, int > plane2) {
        std::vector < gre::Vec3 > coords1 = (*scene).objects[plane1.first].get_mesh_positions(plane1.second, 0);
        std::vector < gre::Vec3 > coords2 = (*scene).objects[plane2.first].get_mesh_positions(plane2.second, 0);
        gre::Plane plane_cur1(coords1);
        gre::Plane plane_cur2(coords2);

        std::vector < gre::Vec3 > pos(3);
        for (int i = 0; i < 3; i++)
            pos[i] = (coords1[i] + coords2[i]) / 2;

        if (!plane_cur1.is_intersect(plane_cur2)) {
            update_plane(pos);
            return;
        }

        gre::Line intersection = plane_cur1.intersect(plane_cur2);
        pos[0] = intersection.start_point;
        pos[1] = intersection.start_point + intersection.get_direction();

        gre::Vec3 direction1 = plane_cur1.get_normal() ^ intersection.get_direction();
        gre::Vec3 direction2 = plane_cur2.get_normal() ^ intersection.get_direction();
        if (direction1 * direction2 * special_coefficient < 0)
            direction2 *= -1;
        pos[2] = intersection.start_point + (direction1 + direction2) / 2;

        update_plane(pos);
    }

    RenderObject* intersect_cut(gre::Plane plane_cur, RenderObject* cut) {
        gre::Vec3 coord1 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[cut->scene_id.first].get_mesh_center(cut->scene_id.second, 1);
        gre::Cut cut_ot(coord1, coord2);

        if (!plane_cur.is_intersect(cut_ot))
            return nullptr;

        RenderObject* point = new Point(plane_cur.intersect(cut_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { cut, this };

        return point;
    }

    RenderObject* intersect_line(gre::Plane plane_cur, RenderObject* line) {
        gre::Vec3 coord1 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 0);
        gre::Vec3 coord2 = (*scene).objects[line->scene_id.first].get_mesh_center(line->scene_id.second, 1);
        gre::Line line_ot(coord1, coord2);

        if (!plane_cur.is_intersect(line_ot))
            return nullptr;

        RenderObject* point = new Point(plane_cur.intersect(line_ot), scene, POINT_RADIUS * 0.75);
        point->action = 1;
        point->init_obj = { line, this };

        return point;
    }

    RenderObject* intersect_plane(gre::Plane plane_cur, RenderObject* plane) {
        std::vector < gre::Vec3 > coords = (*scene).objects[plane->scene_id.first].get_mesh_positions(plane->scene_id.second, 0);
        gre::Plane plane_ot(coords);

        if (!plane_cur.is_intersect(plane_ot))
            return nullptr;

        gre::Line intersection = plane_cur.intersect(plane_ot);

        RenderObject* line = new Line(intersection.start_point, intersection.start_point + intersection.get_direction(), scene);
        line->action = 1;
        line->init_obj = { this, plane };

        return line;
    }
    
public:
    Plane(std::pair < int, int > button, std::vector < RenderObject* > init_obj, gre::GraphEngine* scene) {
        type = 3;
        this->scene = scene;
        this->init_obj = init_obj;

        init();
        set_action(button);
        update();
    }

    void switch_hide() {
        gre::Material material = (*scene).objects[scene_id.first].meshes[0].material;
        if (!hide)
            material.set_alpha(0.1);
        else
            material.set_alpha(0.25);
        (*scene).objects[scene_id.first].meshes.apply_func([&](auto& mesh) {
            mesh.material = material;
        });
        hide ^= 1;
    }

    void set_border(bool flag) {
        if (flag) {
            (*scene).objects[scene_id.first].border_mask = 0b10;
        } else {
            (*scene).objects[scene_id.first].border_mask = 0;
        }
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

    RenderObject* intersect(RenderObject* obj) {
        if (obj->get_type() > type)
            return obj->intersect(this);

        std::vector < gre::Vec3 > coords = (*scene).objects[scene_id.first].get_mesh_positions(scene_id.second, 0);
        gre::Plane plane_cur(coords);

        if (obj->get_type() == 1)
            return intersect_cut(plane_cur, obj);
        if (obj->get_type() == 2)
            return intersect_line(plane_cur, obj);
        if (obj->get_type() == 3)
            return intersect_plane(plane_cur, obj);
    }
};
