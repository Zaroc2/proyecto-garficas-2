#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "Mesh.h"

class MeshManager {
public:
    // Primitivas a tamaño base, se escalan con transform.scale y son punteros para así poder compartir meshes entre varios objetos   
    Mesh* getCube();//Cubo: lado 1 (de -0.5 a 0.5)
    Mesh* getSphere();//Esfera radio 0.5
    Mesh* getPyramid();//Pirámide base 1, altura 1
    Mesh* getCylinder();//Cilindro radio 0.5, altura 1

    // Para modelos cargados desde .obj. El que llama decide el id.
    // Si vuelve a pedir el mismo id, devuelve el mismo Mesh.
    Mesh* getOrLoadModel(int id,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices);

private:
    std::unique_ptr<Mesh> cube;
    std::unique_ptr<Mesh> sphere;
    std::unique_ptr<Mesh> pyramid;
    std::unique_ptr<Mesh> cylinder;

    // loadedModels[i] contiene el mesh con id i (o nullptr si no existe)
    std::vector<std::unique_ptr<Mesh>> loadedModels;
};