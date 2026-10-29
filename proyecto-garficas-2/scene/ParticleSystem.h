#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;        // Tiempo restante de vida
    float maxLife;     // Tiempo total de vida
    float size;        // Tamaño del punto
};

class ParticleSystem {
public:
    std::string name = "Sistema"; // Para mostrarlo en ImGui
    bool enabled = true;
    std::vector<Particle> particles;

    // Tasa de emisión (partículas por segundo)
    float emissionRate = 50.0f;
    float emissionTimer = 0.0f;

    // Parámetros para el emisor
    glm::vec3 emitterPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec4 startColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Naranja
    glm::vec4 endColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);   // Rojo transparente

    void Update(float dt);
    void Emit();
};