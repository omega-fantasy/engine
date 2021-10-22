#ifndef SCENE_H
#define SCENE_H

#include "input.h"
#include "engine.h"

class Scene {
    public:
    virtual bool advance(int current_frame) = 0;
};

class ScenePlayer {
    public:
    void handle_scenes() {
        if (!current_scene) {
            return;
        }
        if (current_frame == 0) {
            Engine.input()->disable();
        }
        if (current_scene->advance(current_frame)) {
            Engine.input()->enable();
            current_frame = 0;
            current_scene = nullptr;
            return;
        }
        current_frame++;
    }

    void play(Scene* s) {
        if (!current_scene) {
            current_scene = s;
        }
    }

    int current_frame = 0;
    Scene* current_scene = nullptr;
};

#endif
