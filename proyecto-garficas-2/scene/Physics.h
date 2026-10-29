#pragma once
// Fase 2.1 (fisica): cuerpos dinamicos - integracion semi-implicita + suelo.
// Mismo espiritu que Collision.h: solo CPU, sin dependencias OpenGL.
// No toca Renderer/Shader.

#include <glm/glm.hpp>

struct Object; // fwd decl (estilo Collision.h)
class Scene;

// Parametros globales del sistema de fisica (UI completa en Fase 2.4).
struct PhysicsParams {
    float gravity = 9.81f;       // m/s^2
    float restThreshold = 0.35f; // umbral de reposo: |vn| menor -> se detiene
    bool groundEnabled = true;   // suelo implicito y = 0
};

namespace Physics {

// Radio de la esfera envolvente del objeto. Convencion: esfera INSCRITA
// (0.5 * min half-extent del bbox escalado): garantiza que el cuerpo quepa
// dentro de su esfera -> un cubo apoya plano sobre el suelo (no flota).
// Nota: contra paredes (Fase 2.2) un cubo penetrara visualmente un poco
// antes de rebotar; para esas demos usar esferas (esfera exacta).
// Devuelve -1.0f si el objeto no tiene mesh.
float computeBoundRadius(const Object* obj);

// Un paso de fisica:
//   1. Integrar dinamicos (gravedad, Euler semi-implicito).
//   2. Rebote contra el suelo y = 0 con restitucion + umbral de reposo.
void step(Scene* scene, float dt, const PhysicsParams& params);

} // namespace Physics
