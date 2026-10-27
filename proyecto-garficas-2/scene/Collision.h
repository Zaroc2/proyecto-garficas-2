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

// true si la esfera en pos choca con algun objeto solido de la escena.
bool CollidesWithScene(Scene* scene, const glm::vec3& pos, float radius);
