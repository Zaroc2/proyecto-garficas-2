#pragma once

#include "window.h"

GLFWwindow* createWindow() {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// Configuramos GLFW
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Queremos OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Para hacer feliz a MacOS ; Aunque no debería ser necesaria
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //No queremos el viejo OpenGL 

	// Creamos la ventana y el contexto OpenGL
	GLFWwindow* window;
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Proyecto Gráficas 2", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	//Función callback que se llama cada que se hace resize a la ventana, simplemente para redimensionar el viewport y que se vea bien
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {glViewport(0, 0, width, height);});

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	if (!gladLoadGL(glfwGetProcAddress)){
		fprintf(stderr, "Failed to initialize OpenGL context\n");
		glfwTerminate();
		return NULL;
	}

	return window;
}