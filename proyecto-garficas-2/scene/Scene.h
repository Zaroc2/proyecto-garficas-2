#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Object.h"
#include "Camera.h"

// src/scene/Scene.h
class Scene {
private:
	GLuint MatrixID = 0;
	GLuint ModelMatrixID = 0;
	GLuint LightDirID = 0;
	GLuint ObjectColorID = 0;
	GLuint AlphaID = 0;

	int nextObjectId = 1; // Para asignar IDs únicos a los objetos
    
public:
    std::vector<std::unique_ptr<Object>> objects;
    std::unique_ptr<Camera> camera;
    glm::vec3 backgroundColor = { 0.1f, 0.1f, 0.15f };
    bool depthTestEnabled = true;
    bool backFaceCullingEnabled = true;
	GLuint shaderProgram = 0;

    Object* addObject(std::unique_ptr<Object> obj);
    void    removeObject(uint32_t id);
    void    clear();
    Object* findById(uint32_t id);

    void draw(float ratio);
    void setup(GLuint shaderProgram);
};