#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Object.h"
#include "Camera.h"
#include "Light.h"
#include "Physics.h"
#include "ParticleSystem.h"

enum class SelectionMode { GLOBAL, LOCAL, TRIANGLE };

// src/scene/Scene.h
class Scene {
public:
	int nextObjectId = 1;
    std::vector<std::unique_ptr<Object>> objects;
    std::unique_ptr<Camera> camera;
    glm::vec3 backgroundColor = { 0.1f, 0.1f, 0.15f };
    bool depthTestEnabled = true;
    bool backFaceCullingEnabled = true;
    bool blendingEnabled = true;
    std::vector<Light> lights;
    int shadingMode = 2; // 0=Flat, 1=Gouraud, 2=Blinn-Phong

    // Fase 2.4: parametros globales de fisica. Viven en la escena (y no en
    // Application) para que se guarden/carguen con ella.
    PhysicsParams physics;

    int selectedObjId = -1;
    SelectionMode selectionMode = SelectionMode::GLOBAL;

    std::vector<ParticleSystem> particleSystems;

    Object* addObject(std::unique_ptr<Object> obj);
    void    removeObject(uint32_t id);
    void    clear();
    Object* findById(uint32_t id);
    Object* getSelectedObject();
    void selectObj(uint32_t id);

    int selectedLightIndex = -1;
};