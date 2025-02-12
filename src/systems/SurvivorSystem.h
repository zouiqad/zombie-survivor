#pragma once
#include "core/Event.h"
#include "core/System.h"

class SurvivorSystem : public System {
public:
    virtual void Init() override;
    virtual void Update(float dt) override;

private:
    // Better way is to use overload in this case we couldn't cause SurvivorType is an enum
    // and we're following ecs pattern which follows the principle of composition over inheritance
    // source: https://www.fluentcpp.com/2017/08/15/function-templates-partial-specialization-cpp/
    void performDoctorAction(const Entity& entity);
    void performMilitaryAction(const Entity& entity);

    // Listeners
    void GameNextTurnListener(Event& event);
};
