#include <scene/Physics.h>
#include <scene/Scene.h>
#include <scene/Object.h>
#include <scene/Collision.h>
#include <render/Mesh.h>
#include <cstdio>

float Physics::computeBoundRadius(const Object* obj) {
    if (obj == nullptr || obj->mesh == nullptr) return -1.0f;
    // Tamano del bbox del mesh escalado por el transform (abs por si
    // alguien pone escala negativa desde codigo).
    const glm::vec3 size =
        (obj->mesh->getBBoxMax() - obj->mesh->getBBoxMin()) * glm::abs(obj->transform.scale);
    // Esfera inscrita: el MENOR half-extent. La circunscrita
    // (0.5 * length) haria flotar los cubos sobre el suelo.
    return 0.5f * glm::min(glm::min(size.x, size.y), size.z);
}

void Physics::step(Scene* scene, float dt, const PhysicsParams& params) {
    if (scene == nullptr) return;
    // NaN-safe: "(dt <= 0)" NO atrapa un dt NaN (toda comparacion con NaN
    // da false). Con "!(dt > 0)" un dt NaN TAMBIEN se queda fuera.
    if (!(dt > 0.0f) || !(dt < 1.0f)) return;

    // Umbral de reposo adaptativo: nunca menor que 2*g*dt para que la
    // velocidad que la gravedad inyecta en UN paso no genere micro-rebotes
    // infinitos (evita el "zumbido" en el suelo con dt variable).
    const float restThreshold = glm::max(params.restThreshold, 2.0f * params.gravity * dt);

    for (auto& obj : scene->objects) {
        if (obj == nullptr || !obj->isDynamic || obj->mesh == nullptr) continue;

        // Radio envolvente: se refresca cada paso (cubre cambios de escala
        // hechos desde la UI sin recalcular a mano).
        const float r = computeBoundRadius(obj.get());
        // NaN-safe (antes "r <= 0" dejaba pasar radios NaN/corruptos).
        if (!(r > 0.0f)) continue;
        obj->boundRadius = r;

        // --- WATCHDOG de diagnostico (bug -nan reportado en 2.1) ---
        // La integracion esta probada limpia (repro aislado), asi que un NaN
        // aqui significa que algo de FUERA escribio el objeto. Este chequeo
        // corre ANTES de integrar: la "huella" de que campos estan corruptos
        // identifica al escritor. Se auto-cura para que el NaN no se
        // propague (p.ej. al std::sort del renderer, que con NaN en el
        // comparador es comportamiento indefinido).
        static int nanReports = 0;
        const glm::vec3& p = obj->transform.position;
        const glm::vec3& v = obj->velocity;
        const bool badPos = glm::any(glm::isnan(p));
        const bool badVel = glm::any(glm::isnan(v));
        const bool badE   = !(obj->restitution >= 0.0f && obj->restitution <= 1.0f);
        if (badPos || badVel || badE) {
            if (nanReports < 10) {
                ++nanReports;
                printf("[FISICA][WATCHDOG] NaN ANTES de integrar -> viene de fuera.\n"
                       "    objeto='%s' id=%u pos=(%f,%f,%f) vel=(%f,%f,%f)\n"
                       "    e=%f r=%f dt=%f scale=(%f,%f,%f) rot=(%f,%f,%f)\n"
                       "    bbox=[(%f,%f,%f)..(%f,%f,%f)]\n",
                       obj->name.c_str(), obj->id,
                       p.x, p.y, p.z, v.x, v.y, v.z,
                       obj->restitution, r, dt,
                       obj->transform.scale.x, obj->transform.scale.y, obj->transform.scale.z,
                       obj->transform.rotation.x, obj->transform.rotation.y, obj->transform.rotation.z,
                       obj->mesh->getBBoxMin().x, obj->mesh->getBBoxMin().y, obj->mesh->getBBoxMin().z,
                       obj->mesh->getBBoxMax().x, obj->mesh->getBBoxMax().y, obj->mesh->getBBoxMax().z);
            }
            // Auto-cura: estado valido conocido (apoyado sobre el suelo).
            obj->transform.position = glm::vec3(0.0f, r, 0.0f);
            obj->velocity = glm::vec3(0.0f);
            if (badE) obj->restitution = 0.6f;
            continue;
        }

        // --- 1. Integracion semi-implicita (simplectica) ---
        // Primero v (con la aceleracion), despues p (con la v NUEVA).
        // Con rebotes esto amortigua en cada paso; el Euler explicito
        // (mover p antes de actualizar v) GANARIA energia y explotaria.
        obj->velocity.y -= params.gravity * dt;
        obj->transform.position += obj->velocity * dt;

        // --- 2. Rebote contra ESTATICOS solidos (Fase 2.2) ---
        // La geometria real de la escena primero; el suelo implicito va al
        // final (red de seguridad). Push-out ANTES de reflejar: la
        // reflexion se calcula con la geometria ya corregida.
        for (const auto& s : scene->objects) {
            if (s == nullptr || s->isDynamic || !s->solid || s->mesh == nullptr) continue;

            glm::vec3 wmin, wmax;
            if (!GetWorldAABB(s.get(), wmin, wmax)) continue;

            Contact ct;
            if (!SphereVsAabbContact(obj->transform.position, r, wmin, wmax, ct)) continue;

            // Correccion de posicion (push-out): reponer el cuerpo justo
            // en contacto con el AABB (sin hundirse, sin temblor).
            obj->transform.position += ct.normal * ct.depth;

            // Reflexion de la velocidad, SOLO si se acercan (vn < 0):
            //   v' = v - (1 + e) * vn * n
            // La componente tangencial se conserva (rebote "ping-pong"
            // entre dos paredes: solo cambia la normal).
            const float vn = glm::dot(obj->velocity, ct.normal);
            if (vn < 0.0f) {
                if (glm::abs(vn) < restThreshold) {
                    // Reposo en la normal: quitar SOLO la componente normal
                    // (el cuerpo desliza por la superficie). Evita los
                    // micro-rebotes en contacto sostenido, igual que el
                    // umbral del suelo.
                    obj->velocity -= ct.normal * vn;
                } else {
                    obj->velocity -= (1.0f + obj->restitution) * vn * ct.normal;
                }
            }
        }

        // --- 3. Suelo implicito y = 0 (normal +Y) ---
        if (params.groundEnabled && obj->transform.position.y < r) {
            // Correccion de posicion: sacar el centro del plano (push-out).
            obj->transform.position.y = r;

            // Rebote: invertir y amortiguar SOLO la componente normal (Y).
            // La tangencial (XZ) se conserva: la friccion llega en Fase 3.2.
            if (obj->velocity.y < 0.0f) {
                if (glm::abs(obj->velocity.y) < restThreshold) {
                    obj->velocity.y = 0.0f;                                 // reposo
                } else {
                    obj->velocity.y = -obj->velocity.y * obj->restitution; // rebote
                }
            }
        }
    }
}
