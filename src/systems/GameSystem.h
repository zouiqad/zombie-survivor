#pragma once

#include "core/System.h"
#include <vector>
#include <cstdint>

#include "components/Survivor.h"
#include "components/Zombie.h"
#include "core/Event.h"

class GameSystem : public System {
public:
    GameSystem();
    virtual ~GameSystem() = default;

    void Init();

    void Update(float dt);

    void Start();

    void Reset();

    void NextTurn();

    uint32_t GetCurrentTurn() const { return mCurrentTurn; }

private:
    uint32_t mMapWidth;
    uint32_t mMapHeight;

    float mTileSize;

    float mTurnTimer;
    uint32_t mCurrentTurn;

    // Metrics
    int mNumTotalZombies;
    int mNumBaseZombies;
    int mNumRunnerZombies;
    int mNumExplosiveZombies;

    int mNumTotalSurvivors;
    int mNumBaseSurvivors;
    int mNumDoctorSurvivors;
    int mNumMilitarySurvivors;

    void SpawnEntities();
    void CheckWinCondition();

    void CreateTile(uint32_t gridX, uint32_t gridY);
    void CreateZombie(int gridX, int gridY, ZombieType zType);
    void CreateSurvivor(int gridX, int gridY, SurvivorType sType);

    // Listeners
    void StartGameListener(Event& event);
    void ResetGameListener(Event& event);
    void NextTurnClickedListener(Event& event);
};
