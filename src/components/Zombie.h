#pragma once
#include <cstdint>

enum class ZombieType {
    Base,
    Runner,
    Explosive
};

struct Zombie {
    ZombieType type;
    int movementPoints;
    int goalX = 0;
    int goalY = 0;
};
