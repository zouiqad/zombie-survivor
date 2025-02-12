#include "WindowManager.h"
#include "components/Camera.h"
#include "systems/RenderSystem.h"
#include "systems/CameraControlSystem.h"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "core/Mediator.h"
#include <chrono>
#include "GUIManager.h"
#include "components/Cubemap.h"
#include "components/Movement.h"
#include "components/Survivor.h"
#include "components/Tile.h"
#include "components/Zombie.h"
#include "systems/GameSystem.h"
#include "systems/MovementSystem.h"
#include "systems/SkyboxRenderSystem.h"
#include "systems/SurvivorSystem.h"
#include "systems/ZombieSystem.h"

Mediator gMediator;

bool quit = false;

void QuitHandler(Event& event)
{
    quit = true;
}

int main() {
    gMediator.Init();

    WindowManager windowManager;
    windowManager.Init("Zombie Survivor", 1920, 1080, 0, 0);

    gMediator.AddEventListener(FUNCTION_LISTENER(Events::Window::QUIT, QuitHandler));

    GUIManager guiManager;
    guiManager.Init(windowManager.getWindow());

    gMediator.RegisterComponent<Camera>();
    gMediator.RegisterComponent<Cubemap>();
    gMediator.RegisterComponent<Renderable>();
    gMediator.RegisterComponent<Transform>();
    gMediator.RegisterComponent<Tile>();
    gMediator.RegisterComponent<Zombie>();
    gMediator.RegisterComponent<Survivor>();
    gMediator.RegisterComponent<Movement>();


    auto cameraControlSystem = gMediator.RegisterSystem<CameraControlSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Camera>());
        signature.set(gMediator.GetComponentType<Transform>());
        gMediator.SetSystemSignature<CameraControlSystem>(signature);
    }

    cameraControlSystem->Init();

    auto renderSystem = gMediator.RegisterSystem<RenderSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Renderable>());
        signature.set(gMediator.GetComponentType<Transform>());
        gMediator.SetSystemSignature<RenderSystem>(signature);
    }

    renderSystem->Init();

    auto skyboxRenderSystem = gMediator.RegisterSystem<SkyboxRenderSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Cubemap>());
        gMediator.SetSystemSignature<SkyboxRenderSystem>(signature);
    }

    auto movementSystem = gMediator.RegisterSystem<MovementSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Movement>());
        signature.set(gMediator.GetComponentType<Transform>());

        gMediator.SetSystemSignature<MovementSystem>(signature);
    }

    movementSystem->Init();

    auto survivorSystem = gMediator.RegisterSystem<SurvivorSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Survivor>());
        gMediator.SetSystemSignature<SurvivorSystem>(signature);
    }

    survivorSystem->Init();

    auto zombieSystem = gMediator.RegisterSystem<ZombieSystem>();
    {
        Signature signature;
        signature.set(gMediator.GetComponentType<Zombie>());
        gMediator.SetSystemSignature<ZombieSystem>(signature);
    }

    zombieSystem->Init();

    auto gameSystem = gMediator.RegisterSystem<GameSystem>();
    {
        Signature signature;
        gMediator.SetSystemSignature<GameSystem>(signature);
    }

    gameSystem->Init();

    // Delta time
    float dt = 0.0f;

    while(!quit) {
        auto startTime = std::chrono::high_resolution_clock::now();

        guiManager.NewFrame();

        windowManager.ProcessEvents();

        cameraControlSystem->Update(dt);

        gameSystem->Update(dt);

        movementSystem->Update(dt);

        survivorSystem->Update(dt);

        zombieSystem->Update(dt);

        renderSystem->Update(dt);

        guiManager.Update();

        windowManager.Update();

        auto stopTime = std::chrono::high_resolution_clock::now();

        dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }

    // Clean resources
    guiManager.Shutdown();
    windowManager.Shutdown();

    return 0;
}
