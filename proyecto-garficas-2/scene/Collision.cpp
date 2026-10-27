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
