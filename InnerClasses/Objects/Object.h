#pragma once


class Object : public RenderObject {
public:
    Object(std::pair<int, int> scene_id, eng::GraphEngine* scene) {
        type = 0;
        action = -1;
        this->scene = scene;
        this->scene_id = scene_id;
    }

    void switch_hide() {
        hide ^= 1;
    }

    void set_border(bool flag) {
        if (flag) {
            (*scene)[scene_id.first].border_mask = 0b1000;
        } else {
            (*scene)[scene_id.first].border_mask = 0;
        }
    }

    void update() {
        return;
    }

    RenderObject* intersect(RenderObject* obj) {
        return nullptr;
    }
};
