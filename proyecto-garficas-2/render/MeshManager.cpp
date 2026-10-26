#include "MeshManager.h"
#include <glm/gtc/constants.hpp>


// añadir un triángulo calculando su normal con producto cruz
void addTriangle(std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
    glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2)
{
    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    uint32_t base = (uint32_t)vertices.size();

    Vertex v0; v0.position = p0; v0.normal = normal; v0.texCoord = uv0;
    Vertex v1; v1.position = p1; v1.normal = normal; v1.texCoord = uv1;
    Vertex v2; v2.position = p2; v2.normal = normal; v2.texCoord = uv2;

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
    v.position = glm::vec3(-s, -s, s); v.normal = glm::vec3(0, 0, 1); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, -s, s);  v.normal = glm::vec3(0, 0, 1); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, s);   v.normal = glm::vec3(0, 0, 1); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, s, s);  v.normal = glm::vec3(0, 0, 1); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

    // Cara trasera (-Z)
    v.position = glm::vec3(s, -s, -s);  v.normal = glm::vec3(0, 0, -1); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(0, 0, -1); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s);  v.normal = glm::vec3(0, 0, -1); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s);   v.normal = glm::vec3(0, 0, -1); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

    // Cara derecha (+X)
    v.position = glm::vec3(s, -s, s);   v.normal = glm::vec3(1, 0, 0); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, -s, -s);  v.normal = glm::vec3(1, 0, 0); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s);   v.normal = glm::vec3(1, 0, 0); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, s);    v.normal = glm::vec3(1, 0, 0); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

    // Cara izquierda (-X)
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(-1, 0, 0); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, s);  v.normal = glm::vec3(-1, 0, 0); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, s, s);   v.normal = glm::vec3(-1, 0, 0); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s);  v.normal = glm::vec3(-1, 0, 0); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

    // Cara superior (+Y)
    v.position = glm::vec3(-s, s, s);   v.normal = glm::vec3(0, 1, 0); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, s);    v.normal = glm::vec3(0, 1, 0); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, s, -s);   v.normal = glm::vec3(0, 1, 0); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, s, -s);  v.normal = glm::vec3(0, 1, 0); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

    // Cara inferior (-Y)
    v.position = glm::vec3(-s, -s, -s); v.normal = glm::vec3(0, -1, 0); v.texCoord = glm::vec2(0.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, -s, -s);  v.normal = glm::vec3(0, -1, 0); v.texCoord = glm::vec2(1.0f, 0.0f); vertices.push_back(v);
    v.position = glm::vec3(s, -s, s);   v.normal = glm::vec3(0, -1, 0); v.texCoord = glm::vec2(1.0f, 1.0f); vertices.push_back(v);
    v.position = glm::vec3(-s, -s, s);  v.normal = glm::vec3(0, -1, 0); v.texCoord = glm::vec2(0.0f, 1.0f); vertices.push_back(v);

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
            vert.texCoord = glm::vec2(u, v);
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
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i1);
            indices.push_back(i3);
            indices.push_back(i2);
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

    // UVs para las caras laterales (Triángulo apuntando hacia arriba)
    glm::vec2 uvL(0.0f, 0.0f); // Abajo Izq
    glm::vec2 uvR(1.0f, 0.0f); // Abajo Der
    glm::vec2 uvTop(0.5f, 1.0f); // Arriba Centro

    // UVs para la base (Cuadrado completo)
    glm::vec2 uv00(0.0f, 0.0f); glm::vec2 uv10(1.0f, 0.0f);
    glm::vec2 uv11(1.0f, 1.0f); glm::vec2 uv01(0.0f, 1.0f);

    // 4 caras laterales
    addTriangle(vertices, indices, A, apex, B, uvL, uvTop, uvR);
    addTriangle(vertices, indices, B, apex, C, uvL, uvTop, uvR);
    addTriangle(vertices, indices, C, apex, D, uvL, uvTop, uvR);
    addTriangle(vertices, indices, D, apex, A, uvL, uvTop, uvR);

    // Base (2 triángulos)
    addTriangle(vertices, indices, A, B, C, uv00, uv10, uv11);
    addTriangle(vertices, indices, A, C, D, uv00, uv11, uv01);
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
        float u = (float)i / segments;
        float theta = u * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        glm::vec3 normal(cx / radius, 0.0f, cz / radius);

        Vertex top;    top.position = glm::vec3(cx, h, cz);     top.normal = normal;    top.texCoord = glm::vec2(u, 1.0f);
        Vertex bottom; bottom.position = glm::vec3(cx, -h, cz); bottom.normal = normal; bottom.texCoord = glm::vec2(u, 0.0f);

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
    Vertex centerTop; centerTop.position = glm::vec3(0, h, 0); centerTop.normal = glm::vec3(0, 1, 0); centerTop.texCoord = glm::vec2(0.5f, 0.5f);
    vertices.push_back(centerTop);

    uint32_t topRingStart = (uint32_t)vertices.size();
    for (int i = 0; i <= segments; ++i) {
        float u = (float)i / segments;
        float theta = u * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        Vertex v; v.position = glm::vec3(cx, h, cz); v.normal = glm::vec3(0, 1, 0);
        v.texCoord = glm::vec2(0.5f + cosf(theta) * 0.5f, 0.5f + sinf(theta) * 0.5f);
        vertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenter);
        indices.push_back(topRingStart + i + 1);
        indices.push_back(topRingStart + i);
    }

    // --- TAPA INFERIOR ---
    uint32_t botCenter = (uint32_t)vertices.size();
    Vertex centerBot; centerBot.position = glm::vec3(0, -h, 0); centerBot.normal = glm::vec3(0, -1, 0); centerBot.texCoord = glm::vec2(0.5f, 0.5f);
    vertices.push_back(centerBot);

    uint32_t botRingStart = (uint32_t)vertices.size();
    for (int i = 0; i <= segments; ++i) {
        float u = (float)i / segments;
        float theta = u * 2.0f * glm::pi<float>();
        float cx = cosf(theta) * radius;
        float cz = sinf(theta) * radius;
        Vertex v; v.position = glm::vec3(cx, -h, cz); v.normal = glm::vec3(0, -1, 0);
        v.texCoord = glm::vec2(0.5f + cosf(theta) * 0.5f, 0.5f + sinf(theta) * 0.5f);
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

        //Submesh de 2 en 2
        const char* names[6] = { "front", "back", "right", "left", "top", "bottom" };
        for (int i = 0; i < 6; ++i) {
            cube->addSubMesh({ (uint32_t)(i * 6), 6, names[i] });
        }
    }
    return cube.get();
}

Mesh* MeshManager::getSphere() {
    if (!sphere) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildSphere(vertices, indices);
        sphere = std::unique_ptr<Mesh>(new Mesh(vertices, indices));

        //submesh es toda la esfera
        sphere->addSubMesh({ 0, (uint32_t)indices.size(), "esfera" });
    }
    return sphere.get();
}

Mesh* MeshManager::getPyramid() {
    if (!pyramid) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildPyramid(vertices, indices);
        pyramid = std::unique_ptr<Mesh>(new Mesh(vertices, indices));

        //submeshes de 3 en 3
        for (int i = 0; i < 6; ++i) {
            pyramid->addSubMesh({ (uint32_t)(i * 3), 3, "tri_" + std::to_string(i) });
        }
    }
    return pyramid.get();
}

Mesh* MeshManager::getCylinder() {
    if (!cylinder) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        buildCylinder(vertices, indices);
        cylinder = std::unique_ptr<Mesh>(new Mesh(vertices, indices));

        const int segments = 24;

        // paredes 6 segmentos por cara
        uint32_t lateralCount = segments * 6;
        cylinder->addSubMesh({ 0, lateralCount, "lateral" });

        // tapa superior  3 triangulos por segmento
        uint32_t topStart = lateralCount;
        uint32_t topCount = segments * 3;
        cylinder->addSubMesh({ topStart, topCount, "tapa_superior" });

        // lo mismo para la tapa inferior
        uint32_t botStart = topStart + topCount;
        uint32_t botCount = segments * 3;
        cylinder->addSubMesh({ botStart, botCount, "tapa_inferior" });
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