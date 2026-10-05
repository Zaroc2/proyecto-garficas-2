#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Object.h"
#include "Camera.h"

enum class SelectionMode { GLOBAL, LOCAL, TRIANGLE }

// src/scene/Scene.h
class Scene {
public:
	int nextObjectId = 1;
    std::vector<std::unique_ptr<Object>> objects;
    std::unique_ptr<Camera> camera;
    glm::vec3 backgroundColor = { 0.1f, 0.1f, 0.15f };
    bool depthTestEnabled = true;
    bool backFaceCullingEnabled = true;

    int selectedObjId = -1;
    SelectionMode selectionMode = SelectionMode::GLOBAL;

    Object* addObject(std::unique_ptr<Object> obj);
    void    removeObject(uint32_t id);
    void    clear();
    Object* findById(uint32_t id);
    Object* getSelectedObject();
    void selectObj(uint32_t id);
};