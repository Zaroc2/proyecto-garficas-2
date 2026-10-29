#pragma once
// B1 (Companero B): colisiones camara-vs-escena con AABB, solo CPU.
// No toca Renderer/Shader (compa A). Sin dependencias OpenGL.

#include <glm/glm.hpp>

struct Object;
class Scene;

// AABB del objeto en espacio mundo: aplica su model matrix a las 8
// esquinas del AABB local del mesh. Devuelve false si no hay mesh.
bool GetWorldAABB(const Object* obj, glm::vec3& outMin, glm::vec3& outMax);

bool AabbVsAabb(const glm::vec3& minA, const glm::vec3& maxA,
                const glm::vec3& minB, const glm::vec3& maxB);

// Esfera (jugador/camara) contra AABB. radius = radio del jugador.
bool SphereVsAabb(const glm::vec3& center, float radius,
                  const glm::vec3& boxMin, const glm::vec3& boxMax);

// ----- Fase 2.2: contacto esfera-AABB con informacion completa -----
// La version de arriba solo dice "si/no" (suficiente para la camara, B1).
// Esta devuelve ademas normal/depth/punto, que es lo que la fisica necesita
// para reflejar la velocidad y corregir la posicion.

struct Contact {
    glm::vec3 normal = { 0.0f, 0.0f, 0.0f }; // del AABB HACIA el centro de la esfera
    float depth = 0.0f;                      // magnitud del push-out (>= 0)
    glm::vec3 point = { 0.0f, 0.0f, 0.0f };  // punto de contacto (q), para eventos/debug (Ambito 3)
};

bool SphereVsAabbContact(const glm::vec3& center, float radius,
                         const glm::vec3& boxMin, const glm::vec3& boxMax,
                         Contact& out);

// ----- Fase 2.3: contacto esfera-esfera (dinamico vs dinamico) -----
// Mismo struct Contact. normal: de A HACIA B. depth = (rA + rB) - dist.
// Caso centros coincidentes: normal (0,1,0) fija (desempate determinista).
bool SphereVsSphereContact(const glm::vec3& cA, float rA,
                           const glm::vec3& cB, float rB,
                           Contact& out);

// true si la esfera en pos choca con algun objeto solido de la escena.
bool CollidesWithScene(Scene* scene, const glm::vec3& pos, float radius);
