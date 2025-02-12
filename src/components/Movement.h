#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Movement {
    std::vector<glm::vec3> waypoints;
    size_t currentIndex = 0;
};
