
#include "proyecto-garficas-2.h"
#include "core/window.h"

#include "render/Shader.h"
#include "render/MeshManager.h"
#include "render/Renderer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"
#include "scene/Object.h"

#include "io/ModelLoader.h"


enum SELECTION_STATE { MOVE, SELECT };

using namespace std;

int main()
{
    GLFWwindow* window = createWindow();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    MeshManager meshManager;

    // Crear escena
    std::unique_ptr<Camera> camera = std::make_unique<Camera>();
    Scene scene;
    scene.camera = std::move(camera);
    scene.camera->updateFromMouse(0, 0);

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
        scene.addObject(std::move(importedObj));
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
    scene.addObject(std::move(cube));

    // Pirámide azul
    unique_ptr<Object> piramid = make_unique<Object>();
    piramid->mesh = meshManager.getPyramid();
    piramid->transform.position = glm::vec3(3.0f, 0.0f, 0.0f);
    piramid->diffuseColor = glm::vec3(0.4f, 0.6f, 1.0f);
    piramid->showVertices = false;
    piramid->wireframe = false;
    piramid->showBBox = false;
    piramid->showNormals = true;
    scene.addObject(std::move(piramid));

    // Renderer
    Renderer renderer;
    renderer.init();
    renderer.initPicking(WINDOW_WIDTH, WINDOW_HEIGHT);

    float aspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;

	SELECTION_STATE selectionMode = SELECT;
    bool selectionModeChange = false;

    double lastxpos = 0, lastypos = 0;
    double lastTime = glfwGetTime();
    do {
        double currentTime = glfwGetTime();
        double delta = currentTime - lastTime;

        // obtener imputs del mouse
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		double dx = xpos - lastxpos;
		double dy = lastypos - ypos;
		lastxpos = xpos;
		lastypos = ypos;

		if (selectionMode == MOVE) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) scene.camera->moveFoward(delta);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) scene.camera->moveBackward(delta);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) scene.camera->moveRight(delta);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) scene.camera->moveLeft(delta);
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) scene.camera->moveUp(delta);
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) scene.camera->moveDown(delta);
		}
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if(!selectionModeChange) {
				if(selectionMode == SELECT) {
					selectionMode = MOVE;
				} else {
					selectionMode = SELECT;
				}

				if (selectionMode == SELECT) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
            }
            selectionModeChange = true;
        }
        else {
            selectionModeChange = false;
        }
    
        if (selectionMode == MOVE) {
    	    scene.camera->updateFromMouse(dx, dy);
        }
        // Animación
        scene.findById(1)->transform.rotation.y = 100.0f * (float)glfwGetTime();
        scene.findById(2)->transform.rotation.z = 100.0f * (float)glfwGetTime();

        float b = 0.5f + 0.5f * sinf((float)glfwGetTime());   // oscila entre 0 y 1
        scene.findById(1)->diffuseColor = glm::vec3(1.0f, 0.4f, b);
        scene.findById(1)->diffuseColor = glm::vec3(1.0f, b, 0.4f);
        scene.findById(2)->diffuseColor = glm::vec3(b, 0.4f, 0.4f);
        scene.findById(2)->diffuseColor = glm::vec3(1.0f, b, 0.4f);
        scene.findById(3)->diffuseColor = glm::vec3(1.0f, 0.4f, b);
        scene.findById(3)->diffuseColor = glm::vec3(b, 0.1f, 0.4f);


        // Render
        renderer.renderScene(scene, aspectRatio);

        // PRUEBA TEMPORAL: con la tecla P, imprimir el ID del centro de la pantalla
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            renderer.renderForPicking(scene);
            PickingResult r = renderer.readPixel(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            printf("Picking centro: objectId=%u, triangleId=%u\n", r.objectId, r.triangleId);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    return 0;
}
