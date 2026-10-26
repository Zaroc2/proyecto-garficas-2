#pragma once
#include <glm/glm.hpp>
#include <glad/gl.h>

struct Material {
    glm::vec3 diffuseColor = { 0.7f, 0.7f, 0.7f };
    float alpha = 1.0f;
    GLuint textureID = 0;       // ID de la textura en OpenGL, 0 por defecto
    bool useTexture = false;
};