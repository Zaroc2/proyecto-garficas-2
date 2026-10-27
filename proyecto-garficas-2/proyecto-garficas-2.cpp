
#include "proyecto-garficas-2.h"
#include "core/window.h"

#include "render/Shader.h"
#include "render/MeshManager.h"
#include "render/Renderer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"
#include "scene/Object.h"

#include "io/ModelLoader.h"
#include "core/Application.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

int main()
{
    GLFWwindow* window = createWindow();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    MeshManager meshManager;

    // Crear escena
    std::unique_ptr<Camera> camera = std::make_unique<Camera>();
    Scene* scene = new Scene();
    scene->camera = std::move(camera);
    scene->camera->syncYawPitchFromFront(); // B1: init yaw/pitch desde front (sin salto)

    // Cargar un modelo .obj
    LoadedModel loadedNRT,loadedSubMeshes;

    if (loadOBJ("../../../../assets/models/nrt.obj", "../../../../assets/models/", loadedNRT)) {
        Mesh* loadedMesh = meshManager.getOrLoadModel(100, loadedNRT.vertices, loadedNRT.indices);

        for(int i=0; i < loadedNRT.subMeshes.size(); i++){
            loadedMesh->addSubMesh(loadedNRT.subMeshes.at(i));
        }

        std::unique_ptr<Object> importedObj = std::make_unique<Object>();
        importedObj->mesh = loadedMesh;
        importedObj->subMeshes = loadedMesh->getSubMeshes();
        importedObj->name = "modelo_NRT_cargado";
        importedObj->transform.position = glm::vec3(-3.0f, 0.0f, 0.0f);  // al lado de los otros
		importedObj->transform.scale = glm::vec3(3.0f, 3.0f, 3.0f);
		importedObj->transform.rotation = glm::vec3(270.0f, -10.0f, 60.0f); // rotación inicial
        importedObj->material.diffuseColor = loadedNRT.diffuseColor;
        importedObj->showVertices = false;
        importedObj->wireframe = false;
        importedObj->showBBox = false;
        importedObj->showNormals = false;
        importedObj->type = MODEL_TYPE::CUSTOM;
        importedObj->objPath = "../../../../assets/models/nrt.obj";
        importedObj->mltPath = "../../../../assets/models/";
        scene->addObject(std::move(importedObj));

    }
    else {
        printf("No se pudo cargar el modelo NRT\n");
    }

    if (false && loadOBJ("../../../../assets/models/carritoFiuFiu.obj", "../../../../assets/models/", loadedSubMeshes)) {
        Mesh* loadedMesh = meshManager.getOrLoadModel(101,loadedSubMeshes.vertices, loadedSubMeshes.indices);

        for (int i = 0; i < loadedSubMeshes.subMeshes.size(); i++) {
            loadedMesh->addSubMesh(loadedSubMeshes.subMeshes.at(i));
        }

        std::unique_ptr<Object> importedObj = std::make_unique<Object>();
        importedObj->mesh = loadedMesh;
        importedObj->subMeshes = loadedMesh->getSubMeshes();
        importedObj->name = "modelo_carritoFiuFiu_cargado";
        importedObj->transform.position = glm::vec3(0.0f, 0.0f, 3.0f);  // perpendicular
        importedObj->transform.scale = glm::vec3(1.5f, 1.5f, 1.5f);
        importedObj->material.diffuseColor = loadedSubMeshes.diffuseColor;
        importedObj->showVertices = false;
        importedObj->wireframe = false;
        importedObj->showBBox = false;
        importedObj->showNormals = false;
        importedObj->type = MODEL_TYPE::CUSTOM;
        importedObj->objPath = "../../../../assets/models/carritoFiuFiu.obj";
        importedObj->mltPath = "../../../../assets/models/";
        scene->addObject(std::move(importedObj));

    }
    else {
        printf("No se pudo cargar el modelo carritoFiuFiu\n");
    }

    GLuint texCubo = LoadTexture("../../../../assets/textures/cube.jpeg");
    GLuint texEsfera = LoadTexture("../../../../assets/textures/saul.jpeg");


    // Cubo rojo
    unique_ptr<Object> cube = make_unique<Object>();
    cube->mesh = meshManager.getCube();
    cube->subMeshes = cube->mesh->getSubMeshes();
    cube->transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    cube->material.diffuseColor = glm::vec3(1.0f, 0.4f, 0.4f);
    cube->showVertices = false;
    cube->wireframe = false;
    cube->showBBox = false;
    cube->showNormals = true;
    cube->material.alpha = 0.5f;
    cube->type = MODEL_TYPE::CUBE;
    cube->material.useTexture = true;
    cube->material.textureID = texCubo;
    scene->addObject(std::move(cube));

    // Esfera azul
    unique_ptr<Object> sphere = make_unique<Object>();
    sphere->mesh = meshManager.getSphere();
	sphere->subMeshes = sphere->mesh->getSubMeshes();
    sphere->transform.position = glm::vec3(3.0f, 0.0f, 0.0f);
    sphere->material.diffuseColor = glm::vec3(0.4f, 0.6f, 1.0f);
    sphere->showVertices = false;
    sphere->wireframe = false;
    sphere->showBBox = false;
    sphere->showNormals = true;
    sphere->type = MODEL_TYPE::SPHERE;
	sphere->material.useTexture = true;
	sphere->material.textureID = texEsfera;
    Object* p = scene->addObject(std::move(sphere));

    //luz Direccional (sol)
    Light sun;
    sun.type = LightType::DIRECTIONAL;
    sun.direction = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.color = glm::vec3(1.0f, 0.95f, 0.8f); // Luz cálida
    sun.intensity = 0.8f;
    scene->lights.push_back(sun);

    //luz Point 
    Light bulb;
    bulb.type = LightType::POINT;
    bulb.position = glm::vec3(0.0f, 2.0f, 0.0f);
    bulb.color = glm::vec3(1.0f, 1.0f, 1.0f); // Rojo
    bulb.intensity = 1.0f;
    scene->lights.push_back(bulb);

    //luz Spot (Linterna azul)
    Light flashlight;
    flashlight.type = LightType::SPOT;
    flashlight.position = glm::vec3(3.0f, 3.0f, 3.0f);
    flashlight.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    flashlight.color = glm::vec3(0.2f, 0.5f, 1.0f); // Azul
    flashlight.intensity = 2.0f;
    flashlight.cutOff = glm::cos(glm::radians(12.5f));
    flashlight.outerCutOff = glm::cos(glm::radians(20.0f));
    scene->lights.push_back(flashlight);
    //scene->selectedObjId = p->id;

    // Renderer
    Renderer renderer;
    renderer.init();
    renderer.initPicking(WINDOW_WIDTH, WINDOW_HEIGHT);

	Application* app = new Application();
    app->meshManager = &meshManager;

    float aspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    do {
        glfwPollEvents();
		app->run(window, scene, &meshManager);

        // Calcular aspect ratio actual por si hubo resize
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        float aspectRatio = fbW / (float)fbH;

        static bool mouseWasPressed = false;
        bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;


        if (mousePressed && !mouseWasPressed && app->selectionMode == SELECT && !ImGui::GetIO().WantCaptureMouse) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            renderer.renderForPicking(scene);
            PickingResult r = renderer.readPixel((int)mx, (int)my);

            //Limpiamos la anterior selección si había
            for (int i = 0; i < scene->objects.size();i++) {
                scene->objects[i]->selectedSubMesh = -1;
                scene->objects[i]->selectedTriangleId = -1;
            }

            scene->selectObj(r.objectId);

            // Guardamos el triángulo en el objeto
            if (r.objectId != 0) {
                Object* selectedObj = scene->findById(r.objectId);
                // buscamos en qué submesh cae para asig
                uint32_t triStart = r.triangleId * 3;
                int subMeshIdx = -1;
                const std::vector<SubMesh>& subs = selectedObj->subMeshes;
                for (size_t i = 0; i < subs.size(); ++i) {
                    if (triStart >= subs[i].indexOffset && triStart < subs[i].indexOffset + subs[i].indexCount) {
                        subMeshIdx = (int)i;
                        break;
                    }
                }

                // Aplicar según el modo
                switch (scene->selectionMode) {
                case SelectionMode::GLOBAL:
                    // nada extra: solo el objeto entero
                    break;
                case SelectionMode::LOCAL:
                    selectedObj->selectedSubMesh = subMeshIdx;
                    break;
                case SelectionMode::TRIANGLE:
                    selectedObj->selectedTriangleId = (int)r.triangleId;
                    break;
                }
            }
        }
        mouseWasPressed = mousePressed;


        // Render
        renderer.renderScene(scene, aspectRatio);

        // Rendering
		// (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls glfwSwapBuffers() etc.)glfwPollEvents();
        glfwSwapBuffers(window);
    } while (glfwGetKey(window, GLFW_KEY_DELETE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
