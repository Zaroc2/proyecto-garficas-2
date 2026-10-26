#pragma once
#include <glm/glm.hpp>

enum class LightType { DIRECTIONAL, POINT, SPOT }; //Direccional = Sol, Point = Bombillo, Spot = Linterna

struct Light {
    LightType type = LightType::POINT;

    glm::vec3 position = { 0.0f, 0.0f, 0.0f };     // solo para point y spot
    glm::vec3 direction = { -1.0f, -1.0f, -1.0f }; // solo para Directional y Spot

    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    // parametros para spot y point
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    // parametros para spot
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f)); //Para que la transición sea suave en cutOff se empieza a apagar y en outerCutOff es 0
};