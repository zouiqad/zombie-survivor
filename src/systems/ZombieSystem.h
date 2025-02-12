#pragma once
#include "core/Event.h"
#include "core/System.h"

class ZombieSystem : public System {
public:
    virtual void Init() override;
    virtual void Update(float dt) override;

private:
    void Infect(const Entity& entity, const Entity& chosen);

    void performBaseAction(const Entity& entity);
    void performExplosiveAction(const Entity& entity);

    // Listeners
    void GameNextTurnListener(Event& event);
};
