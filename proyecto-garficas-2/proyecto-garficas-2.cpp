
#include "proyecto-garficas-2.h"
#include "core/window.h"
#include "render/Shader.h"
#include "render/MeshManager.h"

#include <glm/gtc/matrix_transform.hpp> //SOLO PARA PROBAR SHADERS

#include "scene/Camera.h"
#include "scene/Scene.h"
#include "scene/Object.h"


using namespace std;

int main()
{
	GLFWwindow* window = createWindow();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	char vertexShaderPath[] = "../../../../assets/shaders/Basic.vertexshader";
	char fragmentShaderPath[] = "../../../../assets/shaders/Basic.fragmentshader";

	GLuint programID = LoadShaders(vertexShaderPath, fragmentShaderPath);

	if (programID == 0) {
		printf("Cannot compile the shaders.");
		return -1;
	}

	Camera camera;

	MeshManager meshManager;

	// creamos un objeto
	unique_ptr<Object> cube = make_unique<Object>();
	cube->mesh = meshManager.getCube();
	cube->transform.scale = glm::vec3(2, 2, 2);

	// creamos otro objeto
	unique_ptr<Object> piramid = make_unique<Object>();
	piramid->mesh = meshManager.getPyramid();
	piramid->transform.position = glm::vec3(3, 0, 0);

	Scene scene;
	scene.camera = camera;
	scene.addObject(std::move(cube));
	scene.addObject(std::move(piramid));
	scene.setup(programID);
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.findById(1)->transform.rotation.y = 10.0f * (float)glfwGetTime();
		scene.findById(2)->transform.rotation.z = 10.0f * (float)glfwGetTime();

		scene.draw(WINDOW_WIDTH / (float)WINDOW_HEIGHT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	return 0;
}
