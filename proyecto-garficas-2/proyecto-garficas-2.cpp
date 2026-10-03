// proyecto-garficas-2.cpp: define el punto de entrada de la aplicación.
//

#include "proyecto-garficas-2.h"
#include "core/window.h"

using namespace std;

int main()
{
	GLFWwindow* window = createWindow();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	do {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	return 0;
}
