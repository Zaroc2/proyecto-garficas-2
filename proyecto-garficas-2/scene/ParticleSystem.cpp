#include "ParticleSystem.h"
#include <cstdlib>

void ParticleSystem::Emit() {
    Particle p;
    p.position = emitterPosition;

    // velocidad random (hacia arriba con dispersión)
    float rx = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
    float ry = ((rand() % 100) / 100.0f) * 1.0f + 0.5f;
    float rz = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
    p.velocity = glm::vec3(rx, ry, rz);

    p.maxLife = 1.0f + ((rand() % 100) / 100.0f); // Entre 1 y 2 segs
    p.life = p.maxLife;
    p.color = startColor;
    p.size = 8.0f + ((rand() % 100) / 100.0f) * 4.0f; // Entre 8 y 12 px

    particles.push_back(p);
}

void ParticleSystem::Update(float dt) {
    //Emitimos nuevas particulas si superamos el intervalo de emisión
    emissionTimer += dt;
    float emitInterval = 1.0f / emissionRate;

    int maxPerFrame = 500;
    while (emissionTimer >= emitInterval && maxPerFrame > 0) {
        Emit();
        emissionTimer -= emitInterval;
        maxPerFrame--;
    }

    // actualizamos las existentes
    for (int i = 0; i < particles.size(); i++) {
        particles[i].life -= dt;
        particles[i].position += particles[i].velocity * dt;

        // Gravedad opcional
        particles[i].velocity.y -= 0.5f * dt;

        // Interpolar color (de naranja a transparente)
        particles[i].color = glm::mix(endColor, startColor, particles[i].life / particles[i].maxLife);
        particles[i].color.a = particles[i].life / particles[i].maxLife; // Se desvanece
    }

    // si el tiempo acabo, las eliminamos
    for (int i = (int)particles.size() - 1; i >= 0; i--) {
        if (particles[i].life <= 0.0f) {
            particles.erase(particles.begin() + i);
        }
    }
}