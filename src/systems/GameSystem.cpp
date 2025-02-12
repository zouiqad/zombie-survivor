#include "GameSystem.h"
#include "core/Mediator.h"
#include "components/Transform.h"
#include "components/Renderable.h"
#include "components/Tile.h"
#include "graphics/PrimitiveMeshes.h"
#include <glm/glm.hpp>
#include <iostream>
#include "components/Movement.h"
#include "core/Logger.h"
#include "game/Config.h"

extern Mediator gMediator;

GameSystem::GameSystem()
    : mMapWidth(10), mMapHeight(10),
      mTileSize(1.0f),
      mTurnTimer(0.0f),
      mCurrentTurn(0)
{
}

void GameSystem::Init() {
    for (uint32_t y = 0; y < mMapHeight; ++y) {
        for (uint32_t x = 0; x < mMapWidth; ++x) {
            CreateTile(x, y);
        }
    }

    gMediator.AddEventListener(METHOD_LISTENER(Events::GUI::START_GAME_CLICKED, GameSystem::StartGameListener));
    gMediator.AddEventListener(METHOD_LISTENER(Events::GUI::RESET_GAME_CLICKED, GameSystem::ResetGameListener));
    gMediator.AddEventListener(METHOD_LISTENER(Events::GUI::NEXT_TURN_CLICKED, GameSystem::NextTurnClickedListener));
}

// TODO dynamically upload texture
std::shared_ptr<Model> GetTileModel(float tileSize) {
    static std::shared_ptr<Model> tileModel = std::make_shared<Model>(
        createTileModel("ground/ground_diffuse.jpg", tileSize)
    );
    return tileModel;
}

// TODO load actual sprite instead of sphere
std::shared_ptr<Model> GetHumanoidModel() {
    static std::shared_ptr<Model> tileModel = std::make_shared<Model>(
        createSphereMesh()
    );
    return tileModel;
}

void GameSystem::CreateTile(uint32_t gridX, uint32_t gridY) {
    Entity tileEntity = gMediator.CreateEntity();

    Tile tileData;
    tileData.x = gridX;
    tileData.y = gridY;
    tileData.type = TileType::Floor;

    gMediator.AddComponent<Tile>(tileEntity, tileData);

    Transform tileTransform;
    tileTransform.position = tileData.toWorldPosition(mTileSize);
    tileTransform.scale = glm::vec3(mTileSize, 0.1f, mTileSize);
    gMediator.AddComponent<Transform>(tileEntity, tileTransform);

    Renderable tileRenderable;

    tileRenderable.model = GetTileModel(mTileSize);
    tileRenderable.color = glm::vec3(0.8f, 0.8f, 0.8f);
    gMediator.AddComponent<Renderable>(tileEntity, tileRenderable);
}

void GameSystem::Start() {
    // Init zombies count
    mNumBaseZombies =  Config::GetInstance().baseZombieCount;
    mNumExplosiveZombies = Config::GetInstance().explosiveZombieCount;
    mNumRunnerZombies = Config::GetInstance().runnerZombieCount;

    mNumTotalZombies = mNumBaseZombies + mNumExplosiveZombies + mNumRunnerZombies;

    // Init Survivors count
    mNumBaseSurvivors = Config::GetInstance().baseSurvivorCount;
    mNumDoctorSurvivors = Config::GetInstance().doctorSurvivorCount;
    mNumMilitarySurvivors = Config::GetInstance().militarySurvivorCount;

    mNumTotalSurvivors = mNumBaseSurvivors + mNumDoctorSurvivors + mNumMilitarySurvivors;

    SpawnEntities();
}

void GameSystem::Update(float dt) {
    if(!Config::GetInstance().autoplay || !Config::GetInstance().simulationStarted || Config::GetInstance().simulationFinished) return;

    mTurnTimer += dt;
    if(mTurnTimer >= Config::GetInstance().turnDuration) {
        mTurnTimer = 0.0f;
        NextTurn();
    }
}

void GameSystem::NextTurn() {
    mTurnTimer = 0.0f;
    mCurrentTurn++;

    Event turnEvent(Events::Game::NEW_TURN);
    turnEvent.SetParam<uint32_t>(Events::Game::CURRENT_TURN, mCurrentTurn);
    gMediator.SendEvent(turnEvent);

    CheckWinCondition();
}

void GameSystem::CheckWinCondition() {
    if(gMediator.GetEntitiesWithComponent<Survivor>().size() - 1 >= mNumTotalSurvivors + mNumTotalZombies) {
        Log("Simulation finished, Surviors WIN!");
        Config::GetInstance().simulationFinished = true;
    }
    else if(gMediator.GetEntitiesWithComponent<Zombie>().size() - 1 >= mNumTotalSurvivors + mNumTotalZombies) {
        Log("Simulation finished, Zombies WIN!");
        Config::GetInstance().simulationFinished = true;
    }
}

// TODO find a cleaner way to reset member variables
void GameSystem::Reset() {
    for(auto& entity : gMediator.GetEntitiesWithComponent<Zombie>())
        gMediator.DestroyEntity(entity);

    for(auto& entity : gMediator.GetEntitiesWithComponent<Survivor>())
        gMediator.DestroyEntity(entity);

    mTurnTimer = 0.0f;
    mCurrentTurn = 0;

    Config::GetInstance().simulationStarted = false;
    Config::GetInstance().simulationFinished = false;
}

void GameSystem::CreateZombie(int gridX, int gridY, ZombieType zType) {
    Entity zombieEntity = gMediator.CreateEntity();

    // Add Transform Component
    Transform zombieTransform;
    zombieTransform.position = glm::vec3(gridX * mTileSize, 0.0f, gridY * mTileSize);
    zombieTransform.scale = glm::vec3(0.8f); // Adjust as needed.
    gMediator.AddComponent<Transform>(zombieEntity, zombieTransform);

    // Add Renderable Component
    Renderable zombieRenderable;
    zombieRenderable.model = GetHumanoidModel();
    gMediator.AddComponent<Renderable>(zombieEntity, zombieRenderable);

    // Add Zombie Component
    Zombie zombieData;
    zombieData.type = zType;
    switch (zType) {
        case ZombieType::Base:    zombieData.movementPoints = 1; break;
        case ZombieType::Runner:   zombieData.movementPoints = Config::GetInstance().runnerZombieSteps; break;
        case ZombieType::Explosive: zombieData.movementPoints = 1; break;
    }
    zombieData.goalX = gridX;
    zombieData.goalY = gridY;
    gMediator.AddComponent<Zombie>(zombieEntity, zombieData);

    // Add an empty Path component
    Movement pathData;
    gMediator.AddComponent<Movement>(zombieEntity, pathData);
}

void GameSystem::CreateSurvivor(int gridX, int gridY, SurvivorType sType) {
    Entity survivorEntity = gMediator.CreateEntity();

    // Add Transform Component
    Transform survivorTransform;
    survivorTransform.position = glm::vec3(gridX * mTileSize, 0.0f, gridY * mTileSize);
    survivorTransform.scale = glm::vec3(0.8f);
    gMediator.AddComponent<Transform>(survivorEntity, survivorTransform);

    // Add Renderable Component
    Renderable survivorRenderable;
    survivorRenderable.model = GetHumanoidModel();
    gMediator.AddComponent<Renderable>(survivorEntity, survivorRenderable);

    // Add Survivor Component
    Survivor survivorData;
    survivorData.type = sType;
    survivorData.movementPoints = 1;
    survivorData.goalX = gridX;
    survivorData.goalY = gridY;
    gMediator.AddComponent<Survivor>(survivorEntity, survivorData);

    // Add an empty Path component.
    Movement movement;
    gMediator.AddComponent<Movement>(survivorEntity, movement);
}

void GameSystem::SpawnEntities() {
    // Spawn zombies
    auto spawnZombies = [&](ZombieType type, int count) {
        for (int i = 0; i < count; ++i) {
            int x = rand() % mMapWidth;
            int y = rand() % mMapHeight;

            CreateZombie(x, y, type);
        }
    };

    spawnZombies(ZombieType::Base, mNumBaseZombies);
    spawnZombies(ZombieType::Runner, mNumRunnerZombies);
    spawnZombies(ZombieType::Explosive, mNumExplosiveZombies);

    // Spawn survivors
    auto spawnSurvivors = [&](SurvivorType type, int count) {
        for (int i = 0; i < count; ++i) {
            int x = rand() % mMapWidth;
            int y = rand() % mMapHeight;

            CreateSurvivor(x, y, type);
        }
    };

    spawnSurvivors(SurvivorType::Base, mNumBaseSurvivors);
    spawnSurvivors(SurvivorType::Doctor, mNumDoctorSurvivors);
    spawnSurvivors(SurvivorType::Military, mNumMilitarySurvivors);
}

// Listeners
void GameSystem::StartGameListener(Event &event) {
    if (Config::GetInstance().simulationStarted) return;

    Start();
    Config::GetInstance().simulationStarted = true;
}

void GameSystem::ResetGameListener(Event &event) {
    if (!Config::GetInstance().simulationStarted) return;

    Reset();
}

void GameSystem::NextTurnClickedListener(Event &event) {
    if (!Config::GetInstance().simulationStarted || Config::GetInstance().simulationFinished) return;

    NextTurn();
}
