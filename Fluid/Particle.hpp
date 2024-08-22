#pragma once
#include "GLTools.hpp"

struct Particle {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 acceleration = glm::vec3(0);
    glm::vec3 force = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    float color = 0;
    float density = 0;
    float pressure = 0;
    uint32_t hash = 0;
    bool isClose = false;
    bool isBoundary = false;

};

