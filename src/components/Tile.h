#pragma once

#include <glm/glm.hpp>

enum TileType {
    Floor,
    Wall
};

struct Tile {
    int x;
    int y;

    TileType type = TileType::Floor;

    glm::vec3 toWorldPosition(float tileSize) const
    {
        return {x * tileSize, 0.0f, y * tileSize};
    }

    bool operator==(const Tile& other) const {
        return x == other.x && y == other.y;
    }

};

// std::hash for Tile, so we can use it as a key in unordered_map
namespace std {
template<>
struct hash<Tile> {
    std::size_t operator()(const Tile& t) const noexcept {

        std::size_t h1 = std::hash<int>{}(t.x);
        std::size_t h2 = std::hash<int>{}(t.y);
        return h1 ^ (h2 << 1);
    }
};
}