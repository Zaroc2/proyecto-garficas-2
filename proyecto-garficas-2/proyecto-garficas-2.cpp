
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
    scene->camera->updateFromMouse(0, 0);

    // Cargar un modelo .obj
    LoadedModel loaded;

    if (loadOBJ("../../../../assets/models/nrt.obj", "../../../../assets/models/", loaded)) {
        Mesh* loadedMesh = meshManager.getOrLoadModel(100, loaded.vertices, loaded.indices);

        std::unique_ptr<Object> importedObj = std::make_unique<Object>();
        importedObj->mesh = loadedMesh;
        importedObj->name = "modelo_cargado";
        importedObj->transform.position = glm::vec3(-3.0f, 0.0f, 0.0f);  // al lado de los otros
		importedObj->transform.scale = glm::vec3(3.0f, 3.0f, 3.0f);
		importedObj->transform.rotation = glm::vec3(270.0f, -10.0f, 60.0f); // rotación inicial
        importedObj->diffuseColor = loaded.diffuseColor;
        importedObj->showVertices = false;
        importedObj->wireframe = false;
        importedObj->showBBox = false;
        importedObj->showNormals = false;
        scene->addObject(std::move(importedObj));
    }
    else {
        printf("No se pudo cargar el modelo\n");
    }


    // Cubo rojo
    unique_ptr<Object> cube = make_unique<Object>();
    cube->mesh = meshManager.getCube();
    cube->transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    cube->diffuseColor = glm::vec3(1.0f, 0.4f, 0.4f);
    cube->showVertices = false;
    cube->wireframe = false;
    cube->showBBox = false;
    cube->showNormals = true;
    scene->addObject(std::move(cube));

    // Pirámide azul
    unique_ptr<Object> piramid = make_unique<Object>();
    piramid->mesh = meshManager.getPyramid();
    piramid->transform.position = glm::vec3(3.0f, 0.0f, 0.0f);
    piramid->diffuseColor = glm::vec3(0.4f, 0.6f, 1.0f);
    piramid->showVertices = false;
    piramid->wireframe = false;
    piramid->showBBox = false;
    piramid->showNormals = true;
    Object* p = scene->addObject(std::move(piramid));

    scene->selectedObjId = p->id;

    // Renderer
    Renderer renderer;
    renderer.init();
    renderer.initPicking(WINDOW_WIDTH, WINDOW_HEIGHT);

	Application* app = new Application();

    float aspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    do {
        glfwPollEvents();
		app->run(window, scene);
        // Animación
        scene->findById(1)->transform.rotation.y = 100.0f * (float)glfwGetTime();
        scene->findById(2)->transform.rotation.z = 100.0f * (float)glfwGetTime();

        float b = 0.5f + 0.5f * sinf((float)glfwGetTime());   // oscila entre 0 y 1
        scene->findById(1)->diffuseColor = glm::vec3(1.0f, 0.4f, b);
        scene->findById(1)->diffuseColor = glm::vec3(1.0f, b, 0.4f);
        scene->findById(2)->diffuseColor = glm::vec3(b, 0.4f, 0.4f);
        scene->findById(2)->diffuseColor = glm::vec3(1.0f, b, 0.4f);
        scene->findById(3)->diffuseColor = glm::vec3(1.0f, 0.4f, b);
        scene->findById(3)->diffuseColor = glm::vec3(b, 0.1f, 0.4f);


        // Render
        renderer.renderScene(scene, aspectRatio);

        // PRUEBA TEMPORAL: con la tecla P, imprimir el ID del centro de la pantalla
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            renderer.renderForPicking(scene);
            PickingResult r = renderer.readPixel(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            printf("Picking centro: objectId=%u, triangleId=%u\n", r.objectId, r.triangleId);
        }

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
