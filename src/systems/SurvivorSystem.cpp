#include "SurvivorSystem.h"
#include "core/Mediator.h"
#include "components/Transform.h"
#include "components/Survivor.h"
#include "components/Zombie.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

#include "components/Renderable.h"
#include "core/Logger.h"
#include "game/Config.h"

extern Mediator gMediator;

void SurvivorSystem::Init() {
    gMediator.AddEventListener(METHOD_LISTENER(Events::Game::NEW_TURN, SurvivorSystem::GameNextTurnListener));

}

void SurvivorSystem::Update(float dt) {
    for(Entity entity : mEntities) {
        auto& renderable = gMediator.GetComponent<Renderable>(entity);
        auto& survivor = gMediator.GetComponent<Survivor>(entity);

        // TODO maybe find a more consistent way to define colors
        switch (survivor.type) {
            case (SurvivorType::Base):
                renderable.color = glm::vec3(0.0f, 0.7f, 0.0f);
                break;
            case (SurvivorType::Doctor):
                renderable.color = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case (SurvivorType::Military):
                renderable.color = glm::vec3(0.1f, 0.1f, 0.8f);
                break;
        }
    }
}

void SurvivorSystem::GameNextTurnListener(Event &event) {
    std::vector<Entity> entitiesToProcess(mEntities.begin(), mEntities.end());

    for (Entity entity : entitiesToProcess) {
        auto& survivorComponent = gMediator.GetComponent<Survivor>(entity);

        switch(survivorComponent.type) {
            case SurvivorType::Doctor:
                performDoctorAction(entity);
                break;
            case SurvivorType::Military:
                performMilitaryAction(entity);
                break;
            default:
                break;
        }

        if(survivorComponent.isInfected) {
            survivorComponent.infectionCountDown++;
        }
        if(survivorComponent.infectionCountDown > Config::GetInstance().infectionSpeed) {
            auto msg = "Survivor " + std::to_string(entity) + " transfomed into a zombie ! ";
            Log(msg);

            Zombie newZombie;
            newZombie.type = ZombieType::Base;
            newZombie.movementPoints = 1;
            gMediator.AddComponent<Zombie>(entity, newZombie);
            gMediator.RemoveComponent<Survivor>(entity);
        }
    }
}


void SurvivorSystem::performDoctorAction(const Entity& entity) {
    auto& tTransform = gMediator.GetComponent<Transform>(entity);
    glm::ivec2 tCoord = tTransform.toTileCoordinate(1.0f);

    std::vector<Entity> candidates;
    for (Entity other : mEntities) {
        auto& otherSurvivor = gMediator.GetComponent<Survivor>(other);
        if (other == entity || !otherSurvivor.isInfected == true)
            continue;

        auto& oTransform = gMediator.GetComponent<Transform>(other);
        glm::ivec2 oCoord = oTransform.toTileCoordinate(1.0f);
        int dx = std::abs(tCoord.x - oCoord.x);
        int dy = std::abs(tCoord.y - oCoord.y);
        if (dx <= 1 && dy <= 1) {
            candidates.push_back(other);
        }
    }

    // Heal a single random survivor from candidate vector
    if (!candidates.empty()) {
        int index = rand() % candidates.size();
        Entity chosen = candidates[index];
        gMediator.GetComponent<Survivor>(chosen).isInfected = false;
        gMediator.GetComponent<Survivor>(chosen).infectionCountDown = 0;

        auto msg = "Doctor Survivor " + std::to_string(entity) + " heals Survivor " + std::to_string(chosen);
        Log(msg);
    }
}

void SurvivorSystem::performMilitaryAction(const Entity& entity) {
    auto& tTransform = gMediator.GetComponent<Transform>(entity);
    glm::ivec2 tCoord = tTransform.toTileCoordinate(1.0f);

    std::vector<Entity> zombies = gMediator.GetEntitiesWithComponent<Zombie>();
    std::vector<Entity> candidates;
    for (Entity zombieEntity : zombies) {
        auto& zTransform = gMediator.GetComponent<Transform>(zombieEntity);
        glm::ivec2 zCoord = zTransform.toTileCoordinate(1.0f);
        int dx = std::abs(tCoord.x - zCoord.x);
        int dy = std::abs(tCoord.y - zCoord.y);
        if (dx <= Config::GetInstance().militarySurvivorRange && dy <= Config::GetInstance().militarySurvivorRange) {
            candidates.push_back(zombieEntity);
        }
    }

    if (!candidates.empty()) {
        int index = rand() % candidates.size();
        Entity chosenZombie = candidates[index];
        gMediator.DestroyEntity(chosenZombie);

        auto msg = "Military Survivor " + std::to_string(entity) + " kills Zombie " + std::to_string(chosenZombie);
        Log(msg);
    }
}


