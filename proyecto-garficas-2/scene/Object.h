#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <render/Mesh.h>
#include <scene/Transform.h>
#include <scene/Material.h>
using namespace glm;

enum MODEL_TYPE { CUBE, SPHERE, PYRAMID, CUSTOM, CYLINDER };

struct Object {
    unsigned int id = 0;          // ID único para color picking
    std::string name;
    Mesh* mesh = nullptr;                   // puntero al mesh (compartido o propio)
    std::vector<SubMesh> subMeshes;
    Transform transform;
    MODEL_TYPE type = MODEL_TYPE::CUSTOM;
    std::string objPath = "";
    std::string mltPath = "";
    bool selected = false;
	Material material;
    
    // Banderas de visualización
    bool wireframe = false;
    bool showNormals = false;
    bool showVertices = false;
    bool showBBox = false;
    bool solid = true; // B1: si false, la camara lo atraviesa (decoracion)

    // Fase 2.1 (fisica): cuerpo dinámico modelado como esfera envolvente.
    bool isDynamic = false;                    // false = estatico (muro)
    glm::vec3 velocity = { 0.0f, 0.0f, 0.0f }; // m/s
    float restitution = 0.6f;                  // e en [0,1]: 1 = rebote perfecto
    float mass = 1.0f;                         // kg (se usa en Fase 2.3)
    float boundRadius = 0.5f;                  // esfera envolvente (la refresca Physics::step)

    int selectedTriangleId = -1;
    int selectedSubMesh = -1;
};