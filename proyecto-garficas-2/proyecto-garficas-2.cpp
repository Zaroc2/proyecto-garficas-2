
#include "proyecto-garficas-2.h"
#include "core/window.h"
#include "render/Shader.h"
#include "render/MeshManager.h"

#include <glm/gtc/matrix_transform.hpp> //SOLO PARA PROBAR SHADERS


using namespace std;

int main()
{
	GLFWwindow* window = createWindow();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//char vertexShaderPath[] = "C:\\Users\\logis\\source\\repos\\proyecto-garficas-2\\assets\\shaders\\SimpleShader.vertexshader";
	//char fragmentShaderPath[] = "C:\\Users\\logis\\source\\repos\\proyecto-garficas-2\\assets\\shaders\\SimpleShader.fragmentshader";

	char vertexShaderPath[] = "../../../../assets/shaders/Basic.vertexshader";
	char fragmentShaderPath[] = "../../../../assets/shaders/Basic.fragmentshader";

	GLuint programID = LoadShaders(vertexShaderPath, fragmentShaderPath);

	if (programID == 0) {
		printf("Cannot compile the shaders.");
		return -1;
	}

	//---PRUEBA
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "modelMatrix");
	GLuint LightDirID = glGetUniformLocation(programID, "lightDir");
	GLuint ObjectColorID = glGetUniformLocation(programID, "objectColor");
	GLuint AlphaID = glGetUniformLocation(programID, "alpha");

	// Activar depth test (crítico para 3D)
	glEnable(GL_DEPTH_TEST);
	//---PRUEBA

	std::vector<Vertex> vertices = {
	{{-1.0f, -1.0f, 0.0f}, {0, 0, 1}},
	{{ 1.0f, -1.0f, 0.0f}, {0, 0, 1}},
	{{ 0.0f,  1.0f, 0.0f}, {0, 0, 1}},
	};
	std::vector<uint32_t> indices = { 0, 1, 2 };

	MeshManager meshManager;

	Mesh* mesh = meshManager.getCube();

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 720.0f, 0.1f, 100.0f);
		glm::mat4 View = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		Model = glm::rotate(Model, (float)glfwGetTime(), glm::vec3(0, 1, 0));
		glm::mat4 MVP = Projection * View * Model;

		glUseProgram(programID);

		//---prueba
		// Enviar uniforms
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
		glUniform3f(LightDirID, -0.5f, -1.0f, -0.3f);
		glUniform3f(ObjectColorID, 1.0f, 0.4f, 0.4f);
		glUniform1f(AlphaID, 1.0f);
		//---prueba

		mesh->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	return 0;
}
