#pragma once
#include <cstdint>

enum class SurvivorType {
    Base,
    Doctor,
    Military
};

struct Survivor {
    SurvivorType type;
    int movementPoints;
    int goalX = 0;
    int goalY = 0;
    bool isInfected = false;
    int infectionCountDown = 0;
};
