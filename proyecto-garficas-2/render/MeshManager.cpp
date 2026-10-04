#include "MeshManager.h"
#include <glm/gtc/constants.hpp>


// añadir un triángulo calculando su normal con producto cruz
void addTriangle(std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    uint32_t base = (uint32_t)vertices.size();

    Vertex v0; v0.position = p0; v0.normal = normal;
    Vertex v1; v1.position = p1; v1.normal = normal;
    Vertex v2; v2.position = p2; v2.normal = normal;

    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);

    indices.push_back(base);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
}

// crear un cubo
void buildCube(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    const float s = 0.5f;
    Vertex v;

    // Cara frontal (+Z)
    v.position = glm::vec3(-s, -s, s); v.normal = glm::vec3(0, 0, 1); vertices.push_back(v);
    v.position = glm::vec3(s, -s, s); v.normal = glm::vec3(0, 0, 1); vertices.push_back(v);
    v.position = glm::vec3(s, s, s); v.normal = glm::vec3(0, 0, 1); vertices.push_back(v);
    v.position = glm::vec3(-s, s, s); v.normal = glm::vec3(0, 0, 1); vertices.push_back(v);

    // Cara trasera (-Z)
    v.position = glm::vec3(s, -s, -s); v.normal = glm::vec3(0, 0, -1); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(0, 0, -1); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s); v.normal = glm::vec3(0, 0, -1); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s); v.normal = glm::vec3(0, 0, -1); vertices.push_back(v);

    // Cara derecha (+X)
    v.position = glm::vec3(s, -s, s); v.normal = glm::vec3(1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(s, -s, -s); v.normal = glm::vec3(1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s); v.normal = glm::vec3(1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(s, s, s); v.normal = glm::vec3(1, 0, 0); vertices.push_back(v);

    // Cara izquierda (-X)
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(-1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, s); v.normal = glm::vec3(-1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(-s, s, s); v.normal = glm::vec3(-1, 0, 0); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s); v.normal = glm::vec3(-1, 0, 0); vertices.push_back(v);

    // Cara superior (+Y)
    v.position = glm::vec3(-s, s, s); v.normal = glm::vec3(0, 1, 0); vertices.push_back(v);
    v.position = glm::vec3(s, s, s); v.normal = glm::vec3(0, 1, 0); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s); v.normal = glm::vec3(0, 1, 0); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s); v.normal = glm::vec3(0, 1, 0); vertices.push_back(v);

    // Cara inferior (-Y)
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(0, -1, 0); vertices.push_back(v);
    v.position = glm::vec3(s, -s, -s); v.normal = glm::vec3(0, -1, 0); vertices.push_back(v);
    v.position = glm::vec3(s, -s, s); v.normal = glm::vec3(0, -1, 0); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, s); v.normal = glm::vec3(0, -1, 0); vertices.push_back(v);

    for (uint32_t face = 0; face < 6; ++face) {
        uint32_t b = face * 4;
        indices.push_back(b);
        indices.push_back(b + 1);
        indices.push_back(b + 2);
        indices.push_back(b);
        indices.push_back(b + 2);
        indices.push_back(b + 3);
    }
}

// crear una esfera
void buildSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    const float radius = 0.5f;
    const int segments = 24;

    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float u = (float)x / segments;
            float v = (float)y / segments;

            float theta = u * 2.0f * glm::pi<float>();
            float phi = v * glm::pi<float>();

            float px = radius * sinf(phi) * cosf(theta);
            float py = radius * cosf(phi);
            float pz = radius * sinf(phi) * sinf(theta);

            glm::vec3 pos(px, py, pz);
            glm::vec3 normal = glm::normalize(pos);

            Vertex vert;
            vert.position = pos;
            vert.normal = normal;
            vertices.push_back(vert);
        }
    }

    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            uint32_t i0 = y * (segments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (segments + 1);
            uint32_t i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }
}

// crear una pirámide
void buildPyramid(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    const float b = 0.5f;  // media base
    const float h = 0.5f;  // media altura

    glm::vec3 A(-b, -h, -b);
    glm::vec3 B(b, -h, -b);
    glm::vec3 C(b, -h, b);
    glm::vec3 D(-b, -h, b);
    glm::vec3 apex(0.0f, h, 0.0f);

    // 4 caras laterales (orden: vértice, ápice, siguiente vértice)
    addTriangle(vertices, indices, A, apex, B);  // trasera
    addTriangle(vertices, indices, B, apex, C);  // derecha
    addTriangle(vertices, indices, C, apex, D);  // frontal
    addTriangle(vertices, indices, D, apex, A);  // izquierda

    // Base 2 triángulos
    addTriangle(vertices, indices, A, B, C);
    addTriangle(vertices, indices, A, C, D);
}

// crear un cilindro --- CILINDRO ---
void buildCylinder(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    const float radius = 0.5f;
    const float h = 0.5f;
    const int segments = 24;

    // --- LATERAL ---
    uint32_t lateralStart = (uint32_t)vertices.size();

    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        glm::vec3 normal(cx / radius, 0.0f, cz / radius);

        Vertex top;    top.position = glm::vec3(cx, h, cz); top.normal = normal;
        Vertex bottom; bottom.position = glm::vec3(cx, -h, cz); bottom.normal = normal;

        vertices.push_back(top);
        vertices.push_back(bottom);
    }

    for (int i = 0; i < segments; ++i) {
        uint32_t topA = lateralStart + i * 2 + 0;
        uint32_t bottomA = lateralStart + i * 2 + 1;
        uint32_t topB = lateralStart + (i + 1) * 2 + 0;
        uint32_t bottomB = lateralStart + (i + 1) * 2 + 1;

        indices.push_back(topA);
        indices.push_back(bottomA);
        indices.push_back(bottomB);

        indices.push_back(topA);
        indices.push_back(bottomB);
        indices.push_back(topB);
    }

    // --- TAPA SUPERIOR ---
    uint32_t topCenter = (uint32_t)vertices.size();
    Vertex centerTop; centerTop.position = glm::vec3(0, h, 0); centerTop.normal = glm::vec3(0, 1, 0);
    vertices.push_back(centerTop);

    uint32_t topRingStart = (uint32_t)vertices.size();
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        Vertex v; v.position = glm::vec3(cx, h, cz); v.normal = glm::vec3(0, 1, 0);
        vertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenter);
        indices.push_back(topRingStart + i + 1);
        indices.push_back(topRingStart + i);
    }

    // --- TAPA INFERIOR ---
    uint32_t botCenter = (uint32_t)vertices.size();
    Vertex centerBot; centerBot.position = glm::vec3(0, -h, 0); centerBot.normal = glm::vec3(0, -1, 0);
    vertices.push_back(centerBot);

    uint32_t botRingStart = (uint32_t)vertices.size();
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        Vertex v; v.position = glm::vec3(cx, -h, cz); v.normal = glm::vec3(0, -1, 0);
        vertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        indices.push_back(botCenter);
        indices.push_back(botRingStart + i);
        indices.push_back(botRingStart + i + 1);
    }
}

//Funciones para crear las primitivas

Mesh* MeshManager::getCube() {
    if (!cube) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildCube(vertices, indices);
        cube = std::unique_ptr<Mesh>(new Mesh(vertices, indices));
    }
    return cube.get();
}

Mesh* MeshManager::getSphere() {
    if (!sphere) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildSphere(vertices, indices);
        sphere = std::unique_ptr<Mesh>(new Mesh(vertices, indices));
    }
    return sphere.get();
}

Mesh* MeshManager::getPyramid() {
    if (!pyramid) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildPyramid(vertices, indices);
        pyramid = std::unique_ptr<Mesh>(new Mesh(vertices, indices));
    }
    return pyramid.get();
}

Mesh* MeshManager::getCylinder() {
    if (!cylinder) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildCylinder(vertices, indices);
        cylinder = std::unique_ptr<Mesh>(new Mesh(vertices, indices));
    }
    return cylinder.get();
}

Mesh* MeshManager::getOrLoadModel(int id,
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices)
{
    if (id < 0) return nullptr;

    // ¿Ya existe?
    if (id < (int)loadedModels.size() && loadedModels[id].get() != nullptr) {
        return loadedModels[id].get();
    }

    // Asegurar que el vector tiene espacio hasta el índice id
    if (id >= (int)loadedModels.size()) {
        loadedModels.resize(id + 1);
    }

    loadedModels[id] = std::unique_ptr<Mesh>(new Mesh(vertices, indices));
    return loadedModels[id].get();
}