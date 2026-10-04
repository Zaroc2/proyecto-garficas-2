#pragma once

#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace {
    constexpr int WINDOW_WIDTH = 1024;
	constexpr int WINDOW_HEIGHT = 720;
}

GLFWwindow* createWindow();