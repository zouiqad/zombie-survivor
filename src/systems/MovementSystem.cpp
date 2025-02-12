#include "MovementSystem.h"

#include <iostream>
#include <queue>
#include <unordered_map>

#include "components/Movement.h"
#include "components/Survivor.h"
#include "components/Transform.h"
#include "components/Zombie.h"
#include "core/Mediator.h"

extern Mediator gMediator;

void MovementSystem::Init() {
    gMediator.AddEventListener(METHOD_LISTENER(Events::Game::NEW_TURN, MovementSystem::GameNextTurnListener));
}

void MovementSystem::Update(float dt) {
    for (auto entity : mEntities) {
        auto& transform = gMediator.GetComponent<Transform>(entity);
        auto& movement  = gMediator.GetComponent<Movement>(entity);

        // If the path is empty or finished, skip this entity.
        if (movement.waypoints.empty() || movement.currentIndex >= movement.waypoints.size())
            continue;

        glm::vec3 targetPos = movement.waypoints[movement.currentIndex];
        glm::vec3 direction = targetPos - transform.position;
        float distance = glm::length(direction);

        float moveSpeed = 10.0f;

        if (distance < 0.1f) {
            movement.currentIndex++;
        } else {
            transform.position += (direction / distance) * moveSpeed * dt;
        }
    }
}

int MovementSystem::Heuristic(const Tile& a, const Tile& b) const {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<Tile> MovementSystem::AStarPathfinding(const Tile& start, const Tile& goal) const {
    using Node = Tile;
    std::unordered_map<Node, Node> cameFrom;
    std::unordered_map<Node, int> costSoFar;

    // Define a lambda for the priority order:
    auto cmp = [&](const Node& left, const Node& right) {
        return (costSoFar[left] + Heuristic(left, goal)) >
               (costSoFar[right] + Heuristic(right, goal));
    };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> frontier(cmp);

    frontier.push(start);
    cameFrom[start] = start;
    costSoFar[start] = 0;

    std::vector<std::pair<int, int>> neighbors = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    while (!frontier.empty()) {
        Node current = frontier.top();
        frontier.pop();

        if (current == goal) {
            break;
        }

        for (auto& offset : neighbors) {
            Node next{ current.x + offset.first, current.y + offset.second };
            int newCost = costSoFar[current] + 1; // assume cost 1 per move
            if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                costSoFar[next] = newCost;
                frontier.push(next);
                cameFrom[next] = current;
            }
        }
    }

    std::vector<Tile> path;
    if (cameFrom.find(goal) == cameFrom.end()) {
        // No valid path found.
        return path;
    }

    Node current = goal;
    while (!(current == start)) {
        path.push_back(current);
        current = cameFrom[current];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

void MovementSystem::GameNextTurnListener(Event &event) {
    const float tileSize = 1.0f;

     for (auto entity : mEntities) {
        auto& transform = gMediator.GetComponent<Transform>(entity);
        auto& movement  = gMediator.GetComponent<Movement>(entity);

        glm::ivec2 startTileCoord = transform.toTileCoordinate(tileSize);
        Tile startTile{ startTileCoord.x, startTileCoord.y };

        // For Zombies -- choose a random survivor target
        if (gMediator.HasComponent<Zombie>(entity)) {
            auto& zombie = gMediator.GetComponent<Zombie>(entity);
            // Get all survivor entities.
            std::vector<Entity> survivors = gMediator.GetEntitiesWithComponent<Survivor>();

            if (!survivors.empty()) {
                int randIndex = rand() % survivors.size();
                Entity targetSurvivor = survivors[randIndex];
                auto& targetTransform = gMediator.GetComponent<Transform>(targetSurvivor);
                glm::ivec2 targetTileCoord = targetTransform.toTileCoordinate(tileSize);

                // Update the zombie's goal.
                zombie.goalX = targetTileCoord.x;
                zombie.goalY = targetTileCoord.y;
                Tile goalTile{ zombie.goalX, zombie.goalY };

                // Compute the path using A*.
                std::vector<Tile> gridPath = AStarPathfinding(startTile, goalTile);
                std::vector<glm::vec3> worldPath;
                for (const auto &tile : gridPath) {
                    worldPath.push_back(tile.toWorldPosition(tileSize));
                }

                int allowedSteps = zombie.movementPoints;
                if (worldPath.size() > (allowedSteps + 1)) {
                    worldPath.resize(allowedSteps + 1);
                }

                movement.waypoints = worldPath;
                movement.currentIndex = 1;
            }
        }

         // For Survivors: choose a random tile.
        else if (gMediator.HasComponent<Survivor>(entity)) {
            auto& survivor = gMediator.GetComponent<Survivor>(entity);

            int goalX = rand() % 9;
            int goalY = rand() % 9;
            survivor.goalX = goalX;
            survivor.goalY = goalY;
            Tile goalTile{ goalX, goalY };

            // Compute the A* path.
            std::vector<Tile> gridPath = AStarPathfinding(startTile, goalTile);
            std::vector<glm::vec3> worldPath;
            for (const auto &tile : gridPath) {
                worldPath.push_back(tile.toWorldPosition(tileSize));
            }

            int allowedSteps = survivor.movementPoints;
            if (worldPath.size() > (allowedSteps + 1)) {
                worldPath.resize(allowedSteps + 1);
            }

            movement.waypoints = worldPath;
            movement.currentIndex = 1;
        }
    }
}

