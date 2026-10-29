#include <scene/Physics.h>
#include <scene/Scene.h>
#include <scene/Object.h>
#include <scene/Collision.h>
#include <render/Mesh.h>
#include <cstdio>
#include <vector>

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

namespace {

// Par detectado en el paso (Fase 2.3). La normal apunta de a HACIA b:
// convencion UNICA para todo el solver, sea cual sea la combinacion
// de tipos de cuerpo (din-est, est-din, din-din).
struct CollisionPair {
    Object* a = nullptr;
    Object* b = nullptr;
    Contact contact;
};

} // namespace

void Physics::step(Scene* scene, float dt, const PhysicsParams& params) {
    if (scene == nullptr) return;
    // NaN-safe: "(dt <= 0)" NO atrapa un dt NaN (toda comparacion con NaN
    // da false). Con "!(dt > 0)" un dt NaN TAMBIEN se queda fuera.
    if (!(dt > 0.0f) || !(dt < 1.0f)) return;

    // Umbral de reposo adaptativo: nunca menor que 2*g*dt para que la
    // velocidad que la gravedad inyecta en UN paso no genere micro-rebotes
    // infinitos (evita el "zumbido" en el suelo con dt variable).
    const float restThreshold = glm::max(params.restThreshold, 2.0f * params.gravity * dt);

    // Clasificacion de cuerpos (tabla Fase 2.2): los decorativos (!solid)
    // quedan fuera de la fisica desde el principio.
    std::vector<Object*> dyns, stats;
    for (const auto& o : scene->objects) {
        if (o == nullptr || o->mesh == nullptr) continue;
        if (o->isDynamic) dyns.push_back(o.get());
        else if (o->solid) stats.push_back(o.get());
    }

    // --- 1. Integrar TODOS los dinamicos (Euler semi-implicito) ---
    // Primero v (con la aceleracion), despues p (con la v NUEVA).
    // Con rebotes esto amortigua en cada paso; el Euler explicito
    // (mover p antes de actualizar v) GANARIA energia y explotaria.
    for (Object* obj : dyns) {
        const float r = computeBoundRadius(obj);
        obj->boundRadius = r; // refresco (cubre cambios de escala en la UI)
        if (!(r > 0.0f)) continue;

        // --- WATCHDOG de diagnostico (bug -nan de la Fase 2.1) ---
        // La integracion esta probada limpia (repro aislado), asi que un NaN
        // aqui significa que algo de FUERA escribio el objeto. Se corre
        // ANTES de integrar: la "huella" de campos corruptos identifica al
        // escritor. Auto-cura para que no se propague (p.ej. al std::sort
        // del renderer, que con NaN en el comparador es UB).
        static int nanReports = 0;
        const glm::vec3& p = obj->transform.position;
        const glm::vec3& v = obj->velocity;
        const bool badPos  = glm::any(glm::isnan(p));
        const bool badVel  = glm::any(glm::isnan(v));
        const bool badE    = !(obj->restitution >= 0.0f && obj->restitution <= 1.0f);
        const bool badMass = !(obj->mass > 0.0f);
        if (badPos || badVel || badE || badMass) {
            if (nanReports < 10) {
                ++nanReports;
                printf("[FISICA][WATCHDOG] NaN ANTES de integrar -> viene de fuera.\n"
                       "    objeto='%s' id=%u pos=(%f,%f,%f) vel=(%f,%f,%f)\n"
                       "    e=%f masa=%f r=%f dt=%f scale=(%f,%f,%f)\n",
                       obj->name.c_str(), obj->id,
                       p.x, p.y, p.z, v.x, v.y, v.z,
                       obj->restitution, obj->mass, r, dt,
                       obj->transform.scale.x, obj->transform.scale.y, obj->transform.scale.z);
            }
            // Auto-cura: estado valido conocido (apoyado sobre el suelo).
            obj->transform.position = glm::vec3(0.0f, r, 0.0f);
            obj->velocity = glm::vec3(0.0f);
            if (badE) obj->restitution = 0.6f;
            if (badMass) obj->mass = 1.0f;
            continue;
        }

        obj->velocity.y -= params.gravity * dt;
        obj->transform.position += obj->velocity * dt;
    }

    // --- 2. Detectar TODOS los contactos (aqui NO se resuelve nada) ---
    // Recopilar pares primero y resolver despues: una colision resuelta
    // no puede "esconder" otra del mismo paso (orden de la arquitectura).
    std::vector<CollisionPair> pairs;

    // 2a. dinamico vs estatico: esfera envolvente vs AABB mundial.
    //     Par (a = estatico, b = dinamico): la normal de
    //     SphereVsAabbContact ya apunta del AABB hacia la esfera (a -> b).
    for (Object* d : dyns) {
        if (!(d->boundRadius > 0.0f)) continue;
        for (Object* s : stats) {
            glm::vec3 wmin, wmax;
            if (!GetWorldAABB(s, wmin, wmax)) continue;
            Contact ct;
            if (!SphereVsAabbContact(d->transform.position, d->boundRadius, wmin, wmax, ct))
                continue;
            pairs.push_back({ s, d, ct });
        }
    }

    // 2b. dinamico vs dinamico: esfera-esfera (i < j, cada par una vez).
    for (size_t i = 0; i < dyns.size(); ++i) {
        if (!(dyns[i]->boundRadius > 0.0f)) continue;
        for (size_t j = i + 1; j < dyns.size(); ++j) {
            if (!(dyns[j]->boundRadius > 0.0f)) continue;
            Contact ct;
            if (!SphereVsSphereContact(dyns[i]->transform.position, dyns[i]->boundRadius,
                                       dyns[j]->transform.position, dyns[j]->boundRadius, ct))
                continue;
            pairs.push_back({ dyns[i], dyns[j], ct });
        }
    }

    // --- 3. Resolver TODOS los pares: impulso unificado + Baumgarte ---
    // UNA sola formula para todos: el estatico aporta invM = 0 (masa
    // infinita) y el resultado es EXACTAMENTE la reflexion de la Fase 2.2
    // — esa es la razon de escribir la fisica contra invM (teoria 2).
    //   v_rel = vB - vA ; vn < 0 ⟹ se acercan
    //   j = -(1 + e) · vn / (invMa + invMb)          [N·s]
    //   vA -= (j · invMa) · n ; vB += (j · invMb) · n
    const float slop = 0.01f; // tolerancia de penetracion que NO se corrige (anti-jitter)
    const float pct  = 0.6f;  // fraccion de correccion por paso (Baumgarte ligero)
    for (const CollisionPair& pr : pairs) {
        const float invMa = pr.a->getInvMass();
        const float invMb = pr.b->getInvMass();
        const float den = invMa + invMb;
        if (!(den > 0.0f)) continue; // dos estaticos (no ocurre, guard igual)

        const glm::vec3 n = pr.contact.normal;

        // Velocidades efectivas: un estatico se comporta como v = 0.
        const glm::vec3 va = pr.a->isDynamic ? pr.a->velocity : glm::vec3(0.0f);
        const glm::vec3 vb = pr.b->isDynamic ? pr.b->velocity : glm::vec3(0.0f);
        const glm::vec3 vrel = vb - va;
        const float vn = glm::dot(vrel, n);

        if (vn < 0.0f) { // se acercan (con NaN no entra: comparacion falsa)
            // e del par: media simple (defendible; motores reales usan max
            // o sqrt del producto). Bajo el umbral de reposo, e = 0: la
            // componente normal se CANCELA (reposo/deslizar) en vez de
            // micro-rebotar — mismo truco del umbral de la Fase 2.1.
            const float eAvg = 0.5f * (pr.a->restitution + pr.b->restitution);
            const float e = (glm::abs(vn) < restThreshold) ? 0.0f : eAvg;
            const float j = -(1.0f + e) * vn / den;
            if (pr.a->isDynamic) pr.a->velocity -= n * (j * invMa);
            if (pr.b->isDynamic) pr.b->velocity += n * (j * invMb);
        }

        // Correccion de posicion proporcional a invM (Baumgarte ligero):
        // quien tiene menos masa se corrige mas; solo el 60% por paso y
        // solo lo que exceda el slop (1 cm). El resto se absorbe en los
        // pasos siguientes — evita el "pop" de corregir el 100% a la vez.
        const float corr = glm::max(pr.contact.depth - slop, 0.0f) * pct / den;
        if (corr > 0.0f) {
            if (pr.a->isDynamic) pr.a->transform.position -= n * (corr * invMa);
            if (pr.b->isDynamic) pr.b->transform.position += n * (corr * invMb);
        }
    }

    // --- 4. Suelo implicito y = 0 (red de seguridad, push-out 100%) ---
    for (Object* obj : dyns) {
        if (!(obj->boundRadius > 0.0f)) continue;
        if (params.groundEnabled && obj->transform.position.y < obj->boundRadius) {
            // Correccion de posicion: sacar el centro del plano (push-out).
            obj->transform.position.y = obj->boundRadius;

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
