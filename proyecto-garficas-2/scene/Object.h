#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <render/Mesh.h>
#include <scene/Transform.h>
using namespace glm;

enum MODEL_TYPE { CUBE, SPHERE, PYRAMID, CUSTOM, CYLINDER };

struct Object {
    unsigned int id = 0;          // ID único para color picking
    std::string name;
    Mesh* mesh = nullptr;                   // puntero al mesh (compartido o propio)
    std::vector<SubMesh> subMeshes;
    Transform transform;
    glm::vec3 diffuseColor = { 0.7f, 0.7f, 0.7f };// Kd del .mtl
    MODEL_TYPE type = MODEL_TYPE::CUSTOM;
    std::string objPath = "";
    std::string mltPath = "";
    float alpha = 1.0f;
    bool selected = false;
    
    // Banderas de visualización
    bool wireframe = false;
    bool showNormals = false;
    bool showVertices = false;
    bool showBBox = false;

    int selectedTriangleId = -1;
    int selectedSubMesh = -1;
};