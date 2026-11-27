
/**
 * Game Engine Development
 * Technische Hochschule Mittelhessen
 * Copyright © 2021-2026 Tobias Reimann
 * Copyright © 2026 Stefan Ganswint
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "app/src/mygame.h"

#include "app/src/submarine/shipscript.h"
#include "engine/src/core/inputMapper.h"
#include "engine/src/core/inputhandler.h"
#include "engine/src/math/vector3.h"
#include "engine/src/sim/entityspawner.h"
#include "app/src/world/aquariumgenerator.h"
#include "app/src/submarine/shipscript.h"
#include "app/src/bubblescript.h"
#include "engine/src/math/random.h"

#include <chrono>
#include <iostream>
#include <memory>

using Engine::Core::InputHandler;
using Engine::Math::Vector3;
using Engine::Sim::EntitySpawner;

namespace App
{
    MyGame::MyGame(int argc, char *argv[])
        : Application(argc, argv)
    {
        settings.title = "Awesome Game";
    }

    bool MyGame::init()
    {
        std::chrono::steady_clock::time_point start =
            std::chrono::steady_clock::now();

        // Initialize engine systems
        bool ok = Application::init();
        if (!ok)
        {
            return false;
        }

        // Load all keyboard and gamepad mappings
        if (!Engine::Core::InputMapper::loadMappings("input.json"))
        {
            std::cerr << "! Failed to load input mappings"
                      << std::endl;
            return false;
        }

        renderer->loadSkybox("models/skybox.model");
        renderer->loadSun("models/sun.model", "models/lensflare.model");

        // aquarium config load
        layout = AquariumLayout::from(aquariumConfig);
        AquariumGenerator::generate(*simulation, aquariumConfig, layout);

        // TODO in richtige COnfig bringen fish inside layout 
        double waterHalfHeight = (layout.surfaceHeight - layout.floorTop) * 0.5;
        fishConfig.shape      = SpawnShape::Box;
        fishConfig.center     = {0.0, 0.0, layout.floorTop + waterHalfHeight};
        fishConfig.boxExtentX = layout.tank.size().x * 0.5 - 10.0;
        fishConfig.boxExtentY = layout.tank.size().y * 0.5 - 10.0;
        fishConfig.heightMin  = -waterHalfHeight;
        fishConfig.heightMax  =  waterHalfHeight;

        asteroids = EntitySpawner::spawnProcedural(*simulation, fishConfig);

        std::chrono::duration<double> startupTime =
            std::chrono::steady_clock::now() - start;

        std::cout
            << "+ Startup time: "
            << startupTime.count()
            << " s"
            << std::endl;

        //! Physics World
        // Water environment for all physics bodies
        physicsWorld = std::make_shared<PhysicsWorld>();
        physicsWorld->setGravity(Vector3::up * -5.0); // game feel, not 9.81
        physicsWorld->setWaterSurface(layout.surfaceHeight);

        //! Bubbles: fixed pool per air stone, placed from the world seed
        const BubbleConfig &bubbles = aquariumConfig.bubbles;
        std::vector<Vector3> bubblers = AquariumGenerator::placeBubblers(aquariumConfig, layout);
        Engine::Math::Random bubbleRandom(aquariumConfig.seed + 5);   // sizes

        for (size_t b = 0; b < bubblers.size(); b++)
        {
            for (int k = 0; k < bubbles.bubblesPerBubbler; k++)
            {
                // Spread start heights over the column → no burst at the start
                double height = (layout.surfaceHeight - bubblers[b].z) * k / bubbles.bubblesPerBubbler;
                double size = bubbleRandom.nextDouble(bubbles.sizeMin, bubbles.sizeMax);

                if (auto bubble = simulation->loadEntity("entities/bubble.entity"))
                {
                    bubble->setPosition(bubblers[b] + Vector3::up * height);
                    bubble->setScale({size, size, size});
                    bubble->addScript(std::make_unique<BubbleScript>(
                        physicsWorld, bubblers[b], layout.surfaceHeight,
                        aquariumConfig.seed + 100 + static_cast<uint32_t>(b * 1000 + k),   // unique per bubble
                        bubbles.jitter));
                }
            }
        }
        //! Spawn Ship
        ok = spawnShip();
        if (!ok) return false;

        //! Respawn Callback
        // Remvoe the exsisting ship and create a new instance
        // when the respawn action is triggered
        InputHandler::onAction("respawn", [this]()
        {
            if (ship)
            {
                ship->queueUnload();
                ship = nullptr;
            }

            if (!spawnShip())
            {
                std::cerr << "! Failed to respawn ship"
                          << std::endl;
            }
        });

        InputHandler::onAction("unloadAsteroid", [this]()
        {
            if (!ship) return;

            std::shared_ptr<Entity> nearest = nullptr;
            double nearestDistance = 0.0;

            for (const std::shared_ptr<Entity> &fish : simulation->findEntitiesWithTag("fish"))
            {
                double distance = (ship->getPosition() - fish->getPosition()).length();

                if (!nearest || distance < nearestDistance)
                {
                    nearest = fish;
                    nearestDistance = distance;
                }
            }

            if (!nearest) return;

            nearest->queueUnload();

            std::cout << "  Queued fish " << nearest->getEntityId() << " for removal" 
            << simulation->getEntityCount() << " entities, "
            << renderer->getModelCount() << " models"
            << std::endl;
        });

        return true;
    }

    //! Spawn Ship
    bool MyGame::spawnShip()
    {
        ship = simulation->loadEntity("entities/ship.entity");
        if (!ship) return false;

        // Set initial position
        ship->setPosition(layout.shipSpawn);

        // Attach movement and physics script to the ship entity
        ship->addScript(std::make_unique<ShipScript>(physicsWorld));

        // Set ship as the active focus entity
        ship->setFocus(true);
        return true;
    }
}