#include <io/SceneSerializer.h>
#include <io/ModelLoader.h>
#include <render/MeshManager.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>

// ---------- helpers de escritura ----------
static json vec3ToJson(const glm::vec3& v) {
    return json::array({ v.x, v.y, v.z });
}

// ---------- helpers de lectura defensiva ----------
static bool jsonToVec3(const json& j, glm::vec3& out) {
    if (!j.is_array() || j.size() != 3) return false;
    try {
        out.x = j[0].get<float>();
        out.y = j[1].get<float>();
        out.z = j[2].get<float>();
        return true;
    } catch (...) {
        return false;
    }
}

template <typename T>
static bool jsonGet(const json& j, const char* key, T& out) {
    auto it = j.find(key);
    if (it == j.end()) return false;
    try {
        out = it->get<T>();
        return true;
    } catch (...) {
        return false;
    }
}

// Reconecta el puntero mesh de un objeto recien cargado.
// Para CUSTOM recarga el .obj de disco (objPath + mtlPath como dir base).
static void linkMesh(Object* o, MeshManager* manager) {
    if (o == nullptr) return;
    o->mesh = nullptr;
    if (manager == nullptr) return;

    switch (o->type) {
    case CUBE:     o->mesh = manager->getCube();     break;
    case SPHERE:   o->mesh = manager->getSphere();   break;
    case PYRAMID:  o->mesh = manager->getPyramid();  break;
    case CYLINDER: o->mesh = manager->getCylinder(); break;
    case CUSTOM: {
        if (o->objPath.empty()) {
            std::cerr << "[loadScene] objeto CUSTOM id=" << o->id
                      << " sin objPath, queda sin mesh." << std::endl;
            return;
        }
        LoadedModel loaded;
        if (!loadOBJ(o->objPath, o->mltPath, loaded)) {
            std::cerr << "[loadScene] no se pudo recargar " << o->objPath
                      << " (id=" << o->id << "), queda sin mesh." << std::endl;
            return;
        }
        Mesh* m = manager->getOrLoadModel(static_cast<int>(o->id),
                                          loaded.vertices, loaded.indices);
        if (m == nullptr) return;
        // getOrLoadModel devuelve el mesh cacheado si el id ya existia:
        // solo agregamos submeshes la primera vez.
        if (m->getSubMeshes().empty()) {
            for (const auto& s : loaded.subMeshes)
                m->addSubMesh(s);
        }
        o->mesh = m;
        // Si el .mtl no traia color y el json tampoco, usa el del modelo.
        break;
    }
    default:
        std::cerr << "[loadScene] tipo desconocido (" << static_cast<int>(o->type)
                  << ") id=" << o->id << ", queda sin mesh." << std::endl;
        break;
    }
    if (o->mesh) {
        o->subMeshes = o->mesh->getSubMeshes();
    }
}

// Serializa un Object a json (solo datos; el mesh se reconecta al cargar).
static json objectToJson(const Object* o) {
    json m;
    m["id"]   = o->id;
    m["name"] = o->name;
    m["type"] = static_cast<int>(o->type);
    m["diffuseColor"] = vec3ToJson(o->material.diffuseColor);
    m["transform"] = {
        { "position", vec3ToJson(o->transform.position) },
        { "rotation", vec3ToJson(o->transform.rotation) },
        { "scale",    vec3ToJson(o->transform.scale) }
    };
    m["alpha"]        = o->material.alpha;
    m["wireframe"]    = o->wireframe;
    m["showNormals"]  = o->showNormals;
    m["showVertices"] = o->showVertices;
    m["showBBox"]     = o->showBBox;
    m["solid"]        = o->solid;

    // Fase 2.4: estado fisico del cuerpo. boundRadius NO se guarda: es
    // derivable (bbox + escala) y Physics::step lo refresca cada paso —
    // nunca persistir lo derivable: se desincroniza.
    m["isDynamic"]   = o->isDynamic;
    m["velocity"]    = vec3ToJson(o->velocity);
    m["restitution"] = o->restitution;
    m["mass"]        = o->mass;

    m["objPath"] = o->objPath;
    m["mltPath"] = o->mltPath;
    return m;
}

// Rellena un Object desde json. Devuelve false si el bloque no es un objeto valido.
static bool objectFromJson(const json& m, Object& o) {
    if (!m.is_object()) return false;

    int id = 0, type = static_cast<int>(CUSTOM);
    jsonGet(m, "id", id);
    o.id = static_cast<unsigned int>(id > 0 ? id : 0);
    jsonGet(m, "name", o.name);
    if (jsonGet(m, "type", type)) {
        if (type < static_cast<int>(CUBE) || type > static_cast<int>(CYLINDER))
            type = static_cast<int>(CUSTOM);
        o.type = static_cast<MODEL_TYPE>(type);
    }

    glm::vec3 c = o.material.diffuseColor;
    auto itc = m.find("diffuseColor");
    if (itc != m.end() && jsonToVec3(*itc, c))
        o.material.diffuseColor = c;

    auto itt = m.find("transform");
    if (itt != m.end() && itt->is_object()) {
        glm::vec3 v;
        auto itp = itt->find("position");
        if (itp != itt->end() && jsonToVec3(*itp, v)) o.transform.position = v;
        auto itr = itt->find("rotation");
        if (itr != itt->end() && jsonToVec3(*itr, v)) o.transform.rotation = v;
        auto its = itt->find("scale");
        if (its != itt->end() && jsonToVec3(*its, v)) o.transform.scale = v;
    }

    jsonGet(m, "alpha", o.material.alpha);
    jsonGet(m, "wireframe", o.wireframe);
    jsonGet(m, "showNormals", o.showNormals);
    jsonGet(m, "showVertices", o.showVertices);
    jsonGet(m, "showBBox", o.showBBox);
    jsonGet(m, "solid", o.solid);

    // Fase 2.4: estado fisico. Lectura defensiva + saneo por si el JSON se
    // edito a mano (restitution fuera de [0,1], masa <= 0).
    jsonGet(m, "isDynamic", o.isDynamic);
    glm::vec3 vel = o.velocity;
    auto itv = m.find("velocity");
    if (itv != m.end() && jsonToVec3(*itv, vel))
        o.velocity = vel;
    if (jsonGet(m, "restitution", o.restitution))
        o.restitution = glm::clamp(o.restitution, 0.0f, 1.0f);
    if (jsonGet(m, "mass", o.mass) && !(o.mass > 0.0f))
        o.mass = 1.0f;

    jsonGet(m, "objPath", o.objPath);
    jsonGet(m, "mltPath", o.mltPath);

    // Estado de seleccion por triangulo/submesh es transitorio: se resetea.
    o.selected = false;
    o.selectedTriangleId = -1;
    o.selectedSubMesh = -1;
    return true;
}

static json cameraToJson(const Camera* cam) {
    json c;
    c["position"] = vec3ToJson(cam->position);
    c["front"]    = vec3ToJson(cam->front);
    c["up"]       = vec3ToJson(cam->up);
    c["yaw"]   = cam->yaw;
    c["pitch"] = cam->pitch;
    c["fov"]   = cam->fov;
    c["nearPlane"] = cam->nearPlane;
    c["farPlane"]  = cam->farPlane;
    c["speed"] = cam->speed;
    c["mouseSensitivity"] = cam->mouseSensitivity;
    return c;
}

static void cameraFromJson(const json& c, Camera& cam) {
    glm::vec3 v;
    if (jsonToVec3(c.value("position", json::array()), v)) cam.position = v;
    if (jsonToVec3(c.value("front", json::array()), v))    cam.front = v;
    if (jsonToVec3(c.value("up", json::array()), v))       cam.up = v;
    jsonGet(c, "yaw", cam.yaw);
    jsonGet(c, "pitch", cam.pitch);
    jsonGet(c, "fov", cam.fov);
    jsonGet(c, "nearPlane", cam.nearPlane);
    jsonGet(c, "farPlane", cam.farPlane);
    jsonGet(c, "speed", cam.speed);
    jsonGet(c, "mouseSensitivity", cam.mouseSensitivity);
    // Re-normaliza por si el json trae vectores degradados.
    if (glm::length(cam.front) > 1e-6f) cam.front = glm::normalize(cam.front);
    if (glm::length(cam.up) > 1e-6f)    cam.up = glm::normalize(cam.up);
}

bool saveScene(const Scene* scene, const std::string& path) {
    if (scene == nullptr) {
        std::cerr << "[saveScene] escena nula." << std::endl;
        return false;
    }

    json obj;
    obj["nextObjectId"] = scene->nextObjectId;
    obj["backgroundColor"] = vec3ToJson(scene->backgroundColor);
    obj["depthTestEnabled"] = scene->depthTestEnabled;
    obj["backFaceCullingEnabled"] = scene->backFaceCullingEnabled;
    obj["blendingEnabled"] = scene->blendingEnabled;
    obj["selectionMode"] = static_cast<int>(scene->selectionMode);
    obj["selectedObjId"] = scene->selectedObjId;

    // Fase 2.4: parametros globales de fisica de la escena.
    obj["physics"] = {
        { "gravity", scene->physics.gravity },
        { "groundEnabled", scene->physics.groundEnabled }
    };

    if (scene->camera != nullptr)
        obj["camera"] = cameraToJson(scene->camera.get());
    else
        obj["camera"] = nullptr;

    obj["objects"] = json::array();
    for (const auto& ptr : scene->objects) {
        if (ptr == nullptr) continue;
        obj["objects"].push_back(objectToJson(ptr.get()));
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[saveScene] no se pudo abrir para escribir: " << path << std::endl;
        return false;
    }
    file << obj.dump(4);
    file.close();
    return true;
}

// Nucleo comun de carga. manager == nullptr -> solo datos, mesh = nullptr.
static bool loadSceneImpl(Scene* scene, MeshManager* manager, const std::string& path) {
    if (scene == nullptr) {
        std::cerr << "[loadScene] escena nula." << std::endl;
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[loadScene] no se pudo abrir para leer: " << path << std::endl;
        return false;
    }

    json obj;
    try {
        file >> obj;
    } catch (const json::parse_error& e) {
        std::cerr << "[loadScene] error de parse JSON: " << e.what() << std::endl;
        return false;
    }
    file.close();

    if (!obj.is_object()) {
        std::cerr << "[loadScene] el archivo no contiene un objeto JSON." << std::endl;
        return false;
    }

    scene->clear();

    jsonGet(obj, "nextObjectId", scene->nextObjectId);

    glm::vec3 bg = scene->backgroundColor;
    auto itbg = obj.find("backgroundColor");
    if (itbg != obj.end() && jsonToVec3(*itbg, bg))
        scene->backgroundColor = bg;

    // "dephTestEnabled" era el nombre con typo de la version anterior: se acepta
    // por compatibilidad, pero se guarda siempre como "depthTestEnabled".
    if (!jsonGet(obj, "depthTestEnabled", scene->depthTestEnabled))
        jsonGet(obj, "dephTestEnabled", scene->depthTestEnabled);
    jsonGet(obj, "backFaceCullingEnabled", scene->backFaceCullingEnabled);
    jsonGet(obj, "blendingEnabled", scene->blendingEnabled);

    int mode = static_cast<int>(scene->selectionMode);
    if (jsonGet(obj, "selectionMode", mode)) {
        if (mode < static_cast<int>(SelectionMode::GLOBAL) ||
            mode > static_cast<int>(SelectionMode::TRIANGLE))
            mode = static_cast<int>(SelectionMode::GLOBAL);
        scene->selectionMode = static_cast<SelectionMode>(mode);
    }
    jsonGet(obj, "selectedObjId", scene->selectedObjId);

    // Fase 2.4: parametros globales de fisica. Escenas viejas no traen el
    // bloque: se conservan los valores por defecto.
    auto itph = obj.find("physics");
    if (itph != obj.end() && itph->is_object()) {
        jsonGet(*itph, "gravity", scene->physics.gravity);
        jsonGet(*itph, "groundEnabled", scene->physics.groundEnabled);
    }

    // Camara (opcional: escenas viejas pueden no traerla).
    auto itcam = obj.find("camera");
    if (itcam != obj.end() && itcam->is_object()) {
        if (scene->camera == nullptr)
            scene->camera = std::make_unique<Camera>();
        cameraFromJson(*itcam, *scene->camera);
    }

    auto itobjs = obj.find("objects");
    if (itobjs != obj.end() && itobjs->is_array()) {
        int maxId = 0;
        for (const auto& m : *itobjs) {
            auto o = std::make_unique<Object>();
            o->mesh = nullptr;
            if (!objectFromJson(m, *o)) {
                std::cerr << "[loadScene] bloque de objeto invalido, se omite." << std::endl;
                continue;
            }
            if (o->id == 0) {
                // JSON sin id valido (p. ej. editado a mano): asigna uno fresco.
                o->id = static_cast<unsigned int>(scene->nextObjectId++);
            }
            linkMesh(o.get(), manager);
            maxId = std::max(maxId, static_cast<int>(o->id));
            scene->objects.push_back(std::move(o));
        }
        // Si el archivo no traia nextObjectId (o quedo desfasado), lo recalcula.
        if (!obj.contains("nextObjectId") || scene->nextObjectId <= maxId)
            scene->nextObjectId = maxId + 1;
    }

    // La seleccion guardada puede apuntar a un id inexistente: se valida.
    if (scene->selectedObjId >= 0 && scene->findById(static_cast<uint32_t>(scene->selectedObjId)) == nullptr)
        scene->selectedObjId = -1;

    return true;
}

bool loadScene(Scene* scene, MeshManager& meshManager, const std::string& path) {
    return loadSceneImpl(scene, &meshManager, path);
}

bool loadScene(Scene* scene, const std::string& path) {
    return loadSceneImpl(scene, nullptr, path);
}
