// proyecto-garficas-2.cpp: define el punto de entrada de la aplicación.
//

#include "proyecto-garficas-2.h"
#include "core/window.h"
#include "render/Shader.h"

using namespace std;

static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

int main()
{
	GLFWwindow* window = createWindow();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	char vertexShaderPath[] = "C:\\Users\\logis\\source\\repos\\proyecto-garficas-2\\assets\\shaders\\SimpleShader.vertexshader";
	char fragmentShaderPath[] = "C:\\Users\\logis\\source\\repos\\proyecto-garficas-2\\assets\\shaders\\SimpleShader.fragmentshader";

	GLuint programID = LoadShaders(vertexShaderPath, fragmentShaderPath);

	if (programID == 0) {
		printf("Cannot compile the shaders.");
		return -1;
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		glDrawArrays(GL_TRIANGLES, 0, 3); // Dibujamos el triángulo
		glDisableVertexAttribArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	return 0;
}
