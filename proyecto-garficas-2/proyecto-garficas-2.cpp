
#include "proyecto-garficas-2.h"
#include "core/window.h"

#include "render/Shader.h"
#include "render/MeshManager.h"
#include "render/Renderer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"
#include "scene/Object.h"


using namespace std;

int main()
{
    GLFWwindow* window = createWindow();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glEnable(GL_DEPTH_TEST);

    MeshManager meshManager;

    // Crear escena
    Scene scene;

    // Cubo rojo
    unique_ptr<Object> cube = make_unique<Object>();
    cube->mesh = meshManager.getCube();
    cube->transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    cube->id = 1;
    cube->diffuseColor = glm::vec3(1.0f, 0.4f, 0.4f);
    scene.addObject(std::move(cube));

    // Pirámide azul
    unique_ptr<Object> piramid = make_unique<Object>();
    piramid->mesh = meshManager.getPyramid();
    piramid->transform.position = glm::vec3(3.0f, 0.0f, 0.0f);
    piramid->id = 2;
    piramid->diffuseColor = glm::vec3(0.4f, 0.6f, 1.0f);
    scene.addObject(std::move(piramid));

    // Renderer
    Renderer renderer;
    renderer.init();

    float aspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    do {
        // Animación
        scene.findById(1)->transform.rotation.y = 100.0f * (float)glfwGetTime();
        scene.findById(2)->transform.rotation.z = 100.0f * (float)glfwGetTime();

        // Render
        renderer.renderScene(scene, aspectRatio);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    return 0;
}