#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Object.h"

// src/scene/Scene.h
class Scene {
public:
    std::vector<std::unique_ptr<Object>> objects;
    // Camera camera;
    glm::vec3 backgroundColor = { 0.1f, 0.1f, 0.15f };
    bool depthTestEnabled = true;
    bool backFaceCullingEnabled = true;

    Object* addObject(std::unique_ptr<Object> obj);
    void    removeObject(uint32_t id);
    void    clear();
    Object* findById(uint32_t id);
};