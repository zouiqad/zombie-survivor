#pragma once

#include <vector>
#include "components/Tile.h"
#include "core/Event.h"
#include "core/System.h"


class MovementSystem : public System {
public:
    MovementSystem() = default;
    ~MovementSystem() =default;

    void Init() override;

    void Update(float dt) override;

private:
    int Heuristic(const Tile& a, const Tile& b) const;
    std::vector<Tile> AStarPathfinding(const Tile& start, const Tile& goal) const;

    // Listeners
    void GameNextTurnListener(Event& event);
};
