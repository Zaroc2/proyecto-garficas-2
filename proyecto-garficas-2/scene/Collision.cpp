#include <scene/Collision.h>
#include <scene/Scene.h>
#include <scene/Object.h>
#include <render/Mesh.h>
#include <limits>

bool GetWorldAABB(const Object* obj, glm::vec3& outMin, glm::vec3& outMax) {
    if (obj == nullptr || obj->mesh == nullptr) return false;
    const glm::vec3 localMin = obj->mesh->getBBoxMin();
    const glm::vec3 localMax = obj->mesh->getBBoxMax();
    const glm::mat4 m = obj->transform.getModelMatrix();
    const float inf = std::numeric_limits<float>::max();
    outMin = glm::vec3(inf);
    outMax = glm::vec3(-inf);
    for (int i = 0; i < 8; ++i) {
        glm::vec3 corner(
            (i & 1) ? localMax.x : localMin.x,
            (i & 2) ? localMax.y : localMin.y,
            (i & 4) ? localMax.z : localMin.z);
        glm::vec3 w = glm::vec3(m * glm::vec4(corner, 1.0f));
        outMin = glm::min(outMin, w);
        outMax = glm::max(outMax, w);
    }
    return true;
}

bool AabbVsAabb(const glm::vec3& minA, const glm::vec3& maxA,
                const glm::vec3& minB, const glm::vec3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
           (minA.y <= maxB.y && maxA.y >= minB.y) &&
           (minA.z <= maxB.z && maxA.z >= minB.z);
}

bool SphereVsAabb(const glm::vec3& center, float radius,
                  const glm::vec3& boxMin, const glm::vec3& boxMax) {
    glm::vec3 closest = glm::clamp(center, boxMin, boxMax);
    glm::vec3 d = center - closest;
    return glm::dot(d, d) < radius * radius;
}

bool CollidesWithScene(Scene* scene, const glm::vec3& pos, float radius) {
    if (scene == nullptr) return false;
    for (const auto& obj : scene->objects) {
        if (obj == nullptr || !obj->solid || obj->mesh == nullptr) continue;
        glm::vec3 wmin, wmax;
        if (!GetWorldAABB(obj.get(), wmin, wmax)) continue;
        if (SphereVsAabb(pos, radius, wmin, wmax)) return true;
    }
    return false;
}

// ----- Fase 2.2: contacto esfera-AABB con normal/depth/punto -----
bool SphereVsAabbContact(const glm::vec3& center, float radius,
                         const glm::vec3& boxMin, const glm::vec3& boxMax,
                         Contact& out) {
    // 1) Punto mas cercano del AABB al centro: saturar cada componente.
    //    (La version bool de arriba ya hacia el clamp; esta lo APROVECHA
    //    para calcular la normal y la profundidad.)
    const glm::vec3 q = glm::clamp(center, boxMin, boxMax);
    const glm::vec3 d = center - q;              // del AABB hacia la esfera
    const float dist2 = glm::dot(d, d);
    if (dist2 >= radius * radius) return false; // sin contacto

    if (dist2 > 1e-12f) {
        // 2) Caso normal: centro FUERA del AABB.
        //    normal = d normalizado; depth = lo que falta para quedar
        //    exactamente en contacto (distancia r del punto mas cercano).
        const float dist = glm::sqrt(dist2);
        out.normal = d / dist;
        out.depth = radius - dist;
        out.point = q;
    } else {
        // 3) Centro DENTRO del AABB (dt largo, solape al colocar, o un
        //    push-out de otro estatico lo metio): empujar por la cara de
        //    MENOR penetracion (minimum translation axis). Probar las 6
        //    caras y elegir la mas cercana al centro.
        struct Face { float dist; glm::vec3 n; };
        const Face faces[6] = {
            { center.x - boxMin.x, glm::vec3(-1.0f, 0.0f, 0.0f) }, // cara -X
            { boxMax.x - center.x, glm::vec3( 1.0f, 0.0f, 0.0f) }, // cara +X
            { center.y - boxMin.y, glm::vec3(0.0f, -1.0f, 0.0f) }, // cara -Y
            { boxMax.y - center.y, glm::vec3(0.0f,  1.0f, 0.0f) }, // cara +Y
            { center.z - boxMin.z, glm::vec3(0.0f, 0.0f, -1.0f) }, // cara -Z
            { boxMax.z - center.z, glm::vec3(0.0f, 0.0f,  1.0f) }, // cara +Z
        };
        const Face* best = &faces[0];
        for (int i = 1; i < 6; ++i)
            if (faces[i].dist < best->dist) best = &faces[i];
        out.normal = best->n;
        // Hay que sacar el centro hasta la cara elegida y luego r mas alla.
        out.depth = best->dist + radius;
        out.point = q; // == center
    }
    return true;
}

// ----- Fase 2.3: contacto esfera-esfera (dinamico vs dinamico) -----
bool SphereVsSphereContact(const glm::vec3& cA, float rA,
                           const glm::vec3& cB, float rB,
                           Contact& out) {
    const glm::vec3 d = cB - cA;              // de A hacia B
    const float rSum = rA + rB;
    const float dist2 = glm::dot(d, d);
    if (dist2 >= rSum * rSum) return false;   // sin contacto

    if (dist2 > 1e-12f) {
        const float dist = glm::sqrt(dist2);
        out.normal = d / dist;
        out.depth = rSum - dist;
        // Punto de contacto: mitad del solape entre las dos superficies
        // (para eventos/debug del Ambito 3).
        out.point = cA + out.normal * (rA - 0.5f * out.depth);
    } else {
        // Centros coincidentes (rarisimo, pero determinista):
        // normal fija (0,1,0) y penetracion total.
        out.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        out.depth = rSum;
        out.point = cA;
    }
    return true;
}
