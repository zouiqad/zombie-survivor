#include "ZombieSystem.h"
#include "core/Mediator.h"
#include "components/Transform.h"
#include "components/Zombie.h"
#include "components/Survivor.h"
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

#include "components/Renderable.h"
#include "core/Logger.h"
#include "game/Config.h"

extern Mediator gMediator;

void GameNextTurnListener(Event& event);

void ZombieSystem::Init() {
    gMediator.AddEventListener(METHOD_LISTENER(Events::Game::NEW_TURN, ZombieSystem::GameNextTurnListener));
}

void ZombieSystem::Update(float dt) {
    for(Entity entity : mEntities) {
        auto& renderable = gMediator.GetComponent<Renderable>(entity);
        auto& zombieComponent = gMediator.GetComponent<Zombie>(entity);

        // TODO maybe find a more consistent way to define colors
        switch (zombieComponent.type) {
            case (ZombieType::Base):
                renderable.color = glm::vec3(0.8f, 0.0f, 0.0f);
                break;
            case (ZombieType::Explosive):
                renderable.color = glm::vec3(0.5f, 0.0f, 0.0f);
                break;
            case (ZombieType::Runner):
                renderable.color = glm::vec3(1.0f, 0.2f, 0.2f);
                break;
        }
    }
}

void ZombieSystem::GameNextTurnListener(Event &event) {
    for (auto entity : mEntities) {
        auto& zombieComponent = gMediator.GetComponent<Zombie>(entity);

        switch (zombieComponent.type)
        {
            case (ZombieType::Base):
            case (ZombieType::Runner):
                performBaseAction(entity);
                break;
            case (ZombieType::Explosive):
                performExplosiveAction(entity);
                break;
        }
    }
}

void ZombieSystem::performBaseAction(const Entity& entity) {
    auto& zTransform = gMediator.GetComponent<Transform>(entity);
    auto zCoord = zTransform.toTileCoordinate(1.0f);

    std::vector<Entity> survivors = gMediator.GetEntitiesWithComponent<Survivor>();
    std::vector<Entity> candidates;
    for (Entity survEntity : survivors) {
        auto& sTransform = gMediator.GetComponent<Transform>(survEntity);
        glm::ivec2 sCoord = sTransform.toTileCoordinate(1.0f);
        int dx = std::abs(zCoord.x - sCoord.x);
        int dy = std::abs(zCoord.y - sCoord.y);

        if (dx <= 1 && dy <= 1) {
            candidates.push_back(survEntity);
        }
    }

    if(candidates.size() > 0) {
        int index = rand() % candidates.size();
        Entity chosen = candidates[index];

        Infect(entity, chosen);
    }
}

void ZombieSystem::performExplosiveAction(const Entity &entity) {
    auto& zTransform = gMediator.GetComponent<Transform>(entity);
    auto zCoord = zTransform.toTileCoordinate(1.0f);

    std::vector<Entity> survivors = gMediator.GetEntitiesWithComponent<Survivor>();
    std::vector<Entity> candidates;
    for (Entity survEntity : survivors) {
        auto& sTransform = gMediator.GetComponent<Transform>(survEntity);
        glm::ivec2 sCoord = sTransform.toTileCoordinate(1.0f);
        int dx = std::abs(zCoord.x - sCoord.x);
        int dy = std::abs(zCoord.y - sCoord.y);
        if (dx <= Config::GetInstance().explosiveZombieRange && dy <= Config::GetInstance().explosiveZombieRange) {
            auto msg = "Explosive Zombie " + std::to_string(entity) + " detonated and infected Survivor " + std::to_string(survEntity);

            Log(msg);

            Infect(entity, survEntity);
        }
    }
}

void ZombieSystem::Infect(const Entity& entity, const Entity& chosen) {
    if(!gMediator.HasComponent<Survivor>(chosen)) return;

    auto msg = "Zombie " + std::to_string(entity) + " infected Survivor " + std::to_string(chosen);
    Log(msg);

    auto& survivorComp = gMediator.GetComponent<Survivor>(chosen);
    survivorComp.isInfected = true;
}




