#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "debug-state.hpp"
#include "game-over-state.hpp"
#include "main-menu-state.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../chambers/protect-chamber.hpp"
#include "../../chambers/boss-chamber.hpp"
#include "../../chambers/map-loader.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../graphics/particle-system.hpp"
#include "choose-chamber-state.hpp"
#include "../game.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include <cmath>

GameplayState::GameplayState(StateManager& manager) : GameState(manager), isDebugMode(false) {
    drawBackground = false;
    setupUI();
    
    RunState& runState = Game::getInstance().getRunState();
    activeChamber = ChamberFactory::createChamber(runState.currentLevel, runState.currentChamber, *player);
    if (activeChamber) {
        activeChamber->setObserver(this);
    }
    
    auto* protect = dynamic_cast<ProtectChamber*>(activeChamber.get());
    if (protect && protect->getEcho()) {
        Echo* echo = protect->getEcho();
        echo->attach(hud);
        echo->attach(this);
        hud->setHasEcho(true);
        hud->onEchoPowerChanged(echo->getPower());
    }

    initPlayerPosition();

    std::string filepath = MapLoader::getChamberFilepath(runState.currentLevel, runState.currentChamber);
    std::string titleStr;
    if (filepath.find("level-1/chamber-1.json") != std::string::npos) {
        titleStr = "Level 1 - The Outer Vault (Marrow Echo)";
    } else if (filepath.find("level-1/chamber-2.json") != std::string::npos) {
        titleStr = "Level 1 - The Sunken Corridor (Hollow Bell)";
    } else if (filepath.find("level-1/chamber-3.json") != std::string::npos) {
        titleStr = "Level 1 - The Gate Gauntlet";
    } else if (filepath.find("level-2/chamber-1.json") != std::string::npos) {
        titleStr = "Level 2 - The Sunken Choir (Clarity Shard)";
    } else if (filepath.find("level-2/chamber-2.json") != std::string::npos) {
        titleStr = "Level 2 - The Hall of Resonance (Resonance Core)";
    } else if (filepath.find("level-2/chamber-3.json") != std::string::npos) {
        titleStr = "Level 2 - The Choir Gauntlet";
    } else if (filepath.find("level-3/chamber-1.json") != std::string::npos) {
        titleStr = "Level 3 - The Resonance Hall (Noise Hall)";
    } else if (filepath.find("level-3/chamber-2.json") != std::string::npos) {
        titleStr = "Level 3 - The Hall of Mirrors (Obsidian Key)";
    } else if (filepath.find("level-3/chamber-3.json") != std::string::npos) {
        titleStr = "Level 3 - The Hunger Pit (Gauntlet)";
    } else if (filepath.find("level-3/chamber-4.json") != std::string::npos) {
        titleStr = "Level 3 - The Sarcophagus Approach (Decoy Reliquary)";
    } else if (filepath.find("mid.json") != std::string::npos) {
        titleStr = "Resting Sanctuary (Free Form Swap)";
    } else if (filepath.find("boss.json") != std::string::npos) {
        titleStr = "The Heart of the Ashen Vault (Boss: Malachar)";
    } else {
        ChamberConfig cfg = MapLoader::loadChamber(filepath);
        titleStr = "Level " + std::to_string(runState.currentLevel) + " - Chamber " + std::to_string(runState.currentChamber);
    }
    startChamberIntro(titleStr);
}

GameplayState::GameplayState(StateManager& manager, ChamberSelectionType type) : GameState(manager), isDebugMode(true) {
    drawBackground = false;
    setupUI();
    
    activeChamber = ChamberFactory::createDebugChamber(type, *player);
    if (activeChamber) {
        activeChamber->setObserver(this);
    }
    
    auto* protect = dynamic_cast<ProtectChamber*>(activeChamber.get());
    if (protect && protect->getEcho()) {
        Echo* echo = protect->getEcho();
        echo->attach(hud);
        echo->attach(this);
        hud->setHasEcho(true);
        hud->onEchoPowerChanged(echo->getPower());
    }

    initPlayerPosition();
    startChamberIntro("Debug Chamber");
}

void GameplayState::setupUI() {
    SettingManager& settings = SettingManager::getInstance();
    // root->setAlignmentY(UI::AlignmentY::Middle);
    root->setChildDefaults({
        .modeX = UI::SizeMode::Expanded,
        .modeY = UI::SizeMode::Expanded
    });

    // Create an overlaying container
    buttonBoxWrapper = root->createChild<UI::Container>()
        ->setAlignmentX(UI::AlignmentX::Right)
        ->setAlignmentY(UI::AlignmentY::Top)
        ->setPadding(20.f, 20.f, 20.f, 20.f);

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = buttonBoxWrapper->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween)
        ->setPadding(20.f, 20.f, 20.f, 20.f)
        ->setColor(sf::Color({255, 255, 255, 10}));

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 180.f,
        .fixedHeight = 50.f
    });

    // Add buttons
    pauseButton = buttonBox->createChild<UI::Button>("Pause", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<PauseState>(stateManager));
        });
    debugButton = buttonBox->createChild<UI::Button>("Debug", "regular", 25)
        ->setOnClick([this]() {
            if (player) {
                stateManager.pushState(std::make_unique<DebugState>(stateManager, *player));
            }
        });
    quitButton = buttonBox->createChild<UI::Button>("Quit Game", "regular", 25)
        ->setOnClick([this]() {
            stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
        });

    // Create centered overlay for Chamber Intro title container
    UI::Container* centerOverlay = root->createChild<UI::Container>()
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(settings.getWindowWidth())
        ->setFixedHeight(settings.getWindowHeight())
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setAlignmentY(UI::AlignmentY::Middle);

    titleContainer = centerOverlay->createChild<UI::Container>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setAlignmentY(UI::AlignmentY::Middle)
        ->setPadding(20.f, 20.f, 40.f, 40.f)
        ->setColor(sf::Color(10, 10, 20, 220));

    chamberTitleText = titleContainer->createChild<UI::Text>("header", 28)
        ->setString("");

    // Add old debug HUD
    playerInfoBoxWrapper = root->createChild<UI::Container>()
        ->setAlignmentX(UI::AlignmentX::Left)
        ->setAlignmentY(UI::AlignmentY::Bottom)
        ->setPadding(20.f, 20.f, 20.f, 20.f);

    playerInfoBox = playerInfoBoxWrapper->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Left)
        ->setPadding(30.f, 30.f, 30.f, 30.f)
        ->setColor(sf::Color(10, 10, 10, 200))
        ->setSpacing(10.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    formText = playerInfoBox->createChild<UI::Text>("header", 25)
        ->setString("Serin");
    hpText = playerInfoBox->createChild<UI::Text>("regular", 20)
        ->setString("HP: 100/100");
    momentumText = playerInfoBox->createChild<UI::Text>("regular", 20)
        ->setString("Momentum: 0");
    cooldownText = playerInfoBox->createChild<UI::Text>("regular", 20)
        ->setString("Cooldown: Ready");
    echoPowerText = playerInfoBox->createChild<UI::Text>("regular", 20)
        ->setString("");

    // Add the new HUD component
    hud = root->createChild<UI::HUD>();

    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);

    if (!isDebugMode) {
        RunState& runState = Game::getInstance().getRunState();
        runState.syncEchoModifiers();
        if (runState.playerHP <= 0.0f) {
            runState.playerHP = 100.0f;
        }
        player->setHp(runState.playerHP);
        player->switchForm(runState.activeForm);
        player->gainMomentum(runState.wraithbladeMomentum, FormType::WRAITHBLADE);
        player->gainMomentum(runState.voidcasterMomentum, FormType::VOIDCASTER);
        player->gainMomentum(runState.ironshellMomentum, FormType::IRONSHELL);
        player->setSpecial1Threshold(runState.special1MomentumThreshold);
    }

    camera.init({static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())}, 0.5f);
}

void GameplayState::startChamberIntro(const std::string& titleStr) {
    SettingManager& settings = SettingManager::getInstance();

    float gridMinX = settings.getGridOffsetX();
    float gridMinY = settings.getGridOffsetY();
    float gridWidth = settings.getGridCols() * settings.getCellSize();
    float gridHeight = settings.getGridRows() * settings.getCellSize();

    if (activeChamber) {
        const auto& grid = activeChamber->getTypeGrid();
        if (!grid.empty() && !grid[0].empty()) {
            gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
            gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
        }
    }

    sf::Vector2f mapCenter({gridMinX + gridWidth / 2.0f, gridMinY + gridHeight / 2.0f});
    float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                gridHeight / static_cast<float>(settings.getWindowHeight()));

    camera.setTargetCenter(mapCenter);
    camera.setTargetZoom(maxZoomOut, maxZoomOut);
    camera.snapToTarget();

    // Prime the player's animator to the correct first-frame texture rect
    // so it doesn't flash the full spritesheet on the very first draw.
    // NOTE: We deliberately do NOT call activeChamber->update(0.f) here —
    // doing so would run completion checks (e.g. enemies.empty()) before
    // any enemies have been wave-spawned, causing chambers to complete prematurely.
    if (player) {
        player->update(0.f);
    }

    if (titleContainer && chamberTitleText) {
        titleContainer->setColor(sf::Color(10, 10, 20, 220));
        chamberTitleText->setString(titleStr);
    }

    introState = ChamberIntroState::TITLE_DISPLAY;
    introTimer = 0.0f;
}


void GameplayState::update(float deltaTime) {
    SettingManager& settings = SettingManager::getInstance();
    float gridMinX = settings.getGridOffsetX();
    float gridMinY = settings.getGridOffsetY();
    float gridWidth = settings.getGridCols() * settings.getCellSize();
    float gridHeight = settings.getGridRows() * settings.getCellSize();

    if (activeChamber) {
        const auto& grid = activeChamber->getTypeGrid();
        if (!grid.empty() && !grid[0].empty()) {
            gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
            gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
        }
    }

    sf::FloatRect mapBounds({gridMinX, gridMinY}, {gridWidth, gridHeight});

    if (introState == ChamberIntroState::TITLE_DISPLAY) {
        introTimer += deltaTime;
        camera.update(0.0f, mapBounds); // Camera stays frozen at zoomed out view

        if (introTimer >= 2.0f) {
            if (titleContainer && chamberTitleText) {
                titleContainer->setColor(sf::Color::Transparent);
                chamberTitleText->setString("");
            }
            introState = ChamberIntroState::ZOOMING_IN;
            if (player) {
                float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                            gridHeight / static_cast<float>(settings.getWindowHeight()));
                camera.setTargetCenter(player->getPosition());
                camera.setTargetZoom(0.5f, maxZoomOut);
            }
        }
        GameState::update(deltaTime);
        return; // Skip player and enemy updates so characters remain frozen!
    } else if (introState == ChamberIntroState::ZOOMING_IN) {
        if (player) {
            float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                        gridHeight / static_cast<float>(settings.getWindowHeight()));
            camera.setTargetCenter(player->getPosition());
            camera.update(deltaTime, mapBounds);

            if (std::abs(camera.getCurrentZoom() - camera.getTargetZoom()) < 0.03f) {
                introState = ChamberIntroState::PLAYING;
                // Return immediately — game logic (chamber update, player movement) runs
                // from the NEXT frame so that completeChamber() cannot fire on the same
                // tick the intro ends.
                GameState::update(deltaTime);
                return;
            }
        }
        GameState::update(deltaTime);
        return; // Skip player and enemy updates while camera zooms in!
    }
    // 1. Update player logic (including real-time WASD movement)
    if (player) {
        player->update(deltaTime);

        if (!player->isAlive()) {
            onChamberFailed();
            return;
        }
    }

    // Check if active chamber is BossChamber and currently in phase transition sequence
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(activeChamber.get());
    if (bossChamber && bossChamber->getBoss() && bossChamber->getBoss()->isAlive()) {
        if (!bossHealthBar) {
            bossHealthBar = root->createChild<UI::BossHealthBar>();
        }
        bossHealthBar->updateBossState(*bossChamber->getBoss());
    }

    if (bossChamber && bossChamber->isPhaseTransitioning()) {
        PhaseTransitionStage stage = bossChamber->getTransitionStage();

        float gridMinX = settings.getGridOffsetX();
        float gridMinY = settings.getGridOffsetY();
        float gridWidth = settings.getGridCols() * settings.getCellSize();
        float gridHeight = settings.getGridRows() * settings.getCellSize();

        if (!bossChamber->getTypeGrid().empty() && !bossChamber->getTypeGrid()[0].empty()) {
            gridWidth = static_cast<float>(bossChamber->getTypeGrid()[0].size()) * settings.getCellSize();
            gridHeight = static_cast<float>(bossChamber->getTypeGrid().size()) * settings.getCellSize();
        }

        sf::Vector2f mapCenter({gridMinX + gridWidth / 2.0f, gridMinY + gridHeight / 2.0f});
        float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                    gridHeight / static_cast<float>(settings.getWindowHeight()));

        if (stage == PhaseTransitionStage::ZOOM_OUT || stage == PhaseTransitionStage::FADE_LERP_ISLANDS) {
            camera.setTargetCenter(mapCenter);
            camera.setTargetZoom(maxZoomOut * 1.2f, maxZoomOut * 1.2f);
        } else if (stage == PhaseTransitionStage::ZOOM_IN) {
            if (player) {
                camera.setTargetCenter(player->getPosition());
                camera.setTargetZoom(0.5f, maxZoomOut * 1.2f);
            }
        }

        sf::FloatRect mapBounds({gridMinX, gridMinY}, {gridWidth, gridHeight});
        camera.update(deltaTime, mapBounds);

        // Update active chamber during transition (ticks phase transition timers)
        activeChamber->update(deltaTime);

        GameState::update(deltaTime);
        return; // Skip player inputs and entity movement while camera transitions!
    }

    // 2. Resolve movement collisions
    if (activeChamber) {
        CollisionSolver::resolveX(*player, activeChamber->getObstaclesFor(player.get()), deltaTime);
        CollisionSolver::resolveY(*player, activeChamber->getObstaclesFor(player.get()), deltaTime);
    } else {
        std::vector<sf::FloatRect> emptyObstacles;
        CollisionSolver::resolveAABB(*player, emptyObstacles, deltaTime);
    }

    if (activeChamber) {
        activeChamber->update(deltaTime);
    }
    ParticleSystem::getInstance().update(deltaTime);

    // 3. Update HUD data — lerp animation runs via the UI tree's Container::update()
    if (player && hud) {
        hud->updatePlayerState(*player);
    }
    
    // 3.5 Update Debug HUD text
    if (player && formText && hpText && momentumText && cooldownText) {
        FormType currentForm = player->getActiveFormType();
        std::string formStr = "Unknown";
        if (currentForm == FormType::WRAITHBLADE) formStr = "Wraithblade";
        else if (currentForm == FormType::VOIDCASTER) formStr = "Voidcaster";
        else if (currentForm == FormType::IRONSHELL) formStr = "Ironshell";
        formText->setString("Form: " + formStr);
        
        Stats currentStats = player->getEffectiveStats();
        hpText->setString("HP: " + std::to_string((int)currentStats.hp) + "/" + std::to_string((int)currentStats.maxHp));
        momentumText->setString("Momentum: " + std::to_string((int)player->getMomentum(currentForm)));
        
        float cd = player->getSwitchCooldownTimer();
        if (cd > 0.0f) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "Cooldown: %.1fs", cd);
            cooldownText->setString(buffer);
        } else {
            cooldownText->setString("Cooldown: Ready");
        }
    }

    // 4. Update camera position lerp & zoom lerp
    if (player) {
        camera.setTargetCenter(player->getPosition());

        SettingManager& settings = SettingManager::getInstance();
        float gridMinX = settings.getGridOffsetX();
        float gridMinY = settings.getGridOffsetY();
        float gridWidth = settings.getGridCols() * settings.getCellSize();
        float gridHeight = settings.getGridRows() * settings.getCellSize();

        if (activeChamber) {
            const auto& grid = activeChamber->getTypeGrid();
            if (!grid.empty() && !grid[0].empty()) {
                gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
                gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
            }
        }

        sf::FloatRect mapBounds({gridMinX, gridMinY}, {gridWidth, gridHeight});
        camera.update(deltaTime, mapBounds);
    }

    // 5. Base class updates UI layouts
    GameState::update(deltaTime);
}

void GameplayState::draw(sf::RenderWindow& window) const {
    // Save original UI View
    sf::View uiView = window.getDefaultView();

    // Draw background
    window.clear(sf::Color(20, 20, 25));

    // Apply Camera View for World
    window.setView(camera.getView());

    if (activeChamber) activeChamber->draw(window);
    
    // Draw player
    if (player) player->draw(window);
    
    // Draw particles on top of world
    window.draw(ParticleSystem::getInstance());
    
    // Restore UI View for HUD
    window.setView(uiView);
    GameState::draw(window);
}

void GameplayState::handleEvents(sf::Event& event) {
    if (introState != ChamberIntroState::PLAYING) {
        // Pass events to UI components (e.g. pause/quit buttons) but skip player gameplay inputs
        GameState::handleEvents(event);
        return;
    }

    // 1. Let the player handle its own single-press inputs
    player->handleInput(event);

    SettingManager& settings = SettingManager::getInstance();

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            if (activeChamber && player) {
                sf::Vector2f dir;
                if (player->getActiveFormType() == FormType::VOIDCASTER) {
                    sf::RenderWindow& window = Game::getInstance().getWindow();
                    sf::Vector2f mouseWorldPos = window.mapPixelToCoords({mouseEvent->position.x, mouseEvent->position.y}, camera.getView());
                    dir = mouseWorldPos - player->getPosition();
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (len != 0.0f) dir /= len;
                    else dir = sf::Vector2f(player->getIsFacingRight() ? 1.0f : -1.0f, 0.0f);
                } else {
                    dir = player->getVelocity();
                    if (dir.x == 0 && dir.y == 0) {
                        dir = sf::Vector2f(player->getIsFacingRight() ? 1.0f : -1.0f, 0.0f);
                    } else {
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        dir /= len;
                    }
                }
                
                player->attack(dir, *activeChamber);
            }
        }
    } else if (const auto* scrollEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
            float gridWidth = settings.getGridCols() * settings.getCellSize();
            float gridHeight = settings.getGridRows() * settings.getCellSize();
            if (activeChamber) {
                const auto& grid = activeChamber->getTypeGrid();
                if (!grid.empty() && !grid[0].empty()) {
                    gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
                    gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
                }
            }
            float maxZoomOut = std::max(gridWidth / settings.getWindowWidth(), gridHeight / settings.getWindowHeight());
            
            camera.zoomBy(scrollEvent->delta, maxZoomOut);
        }
    }

    // 2. Pass down to UI components
    GameState::handleEvents(event);
}

void GameplayState::onEchoPowerChanged(float power) {
    if (echoPowerText) {
        echoPowerText->setString("Echo Power: " + std::to_string((int)power) + "%");
    }
}

void GameplayState::onChamberCompleted() {
    std::cout << "GameplayState: Chamber Completed!\n";
    if (player) {
        RunState& runState = Game::getInstance().getRunState();
        runState.playerHP = player->getHp();
        runState.activeForm = player->getActiveFormType();
        runState.wraithbladeMomentum = player->getMomentum(FormType::WRAITHBLADE);
        runState.voidcasterMomentum = player->getMomentum(FormType::VOIDCASTER);
        runState.ironshellMomentum = player->getMomentum(FormType::IRONSHELL);
        
        // BUG-20: Auto-save game progress on chamber completion
        SaveLoadManager::getInstance().saveGame(runState);
    }
    if (isDebugMode) {
        stateManager.changeState(std::make_unique<ChooseChamberState>(stateManager));
    } else {
        RunState& runState = Game::getInstance().getRunState();
        std::string nextPath = MapLoader::getChamberFilepath(runState.currentLevel, runState.currentChamber + 1);
        
        if (!nextPath.empty()) {
            // Next chamber in same level
            runState.currentChamber++;
        } else {
            // Check if there's a next level
            std::string nextLevelPath = MapLoader::getChamberFilepath(runState.currentLevel + 1, 1);
            if (!nextLevelPath.empty()) {
                runState.currentLevel++;
                runState.currentChamber = 1;
            } else {
                // Game completely over! (Win)
                int stolenCount = 0;
                for (const auto& [type, outcome] : runState.echoOutcomes) {
                    if (outcome == EchoOutcome::STOLEN) {
                        stolenCount++;
                    }
                }
                runState.echoesStolen = stolenCount;

                EndingType ending = EndingType::ENDING_A_SHATTER;
                if (stolenCount == 0) {
                    ending = EndingType::ENDING_A_SHATTER;
                } else if (stolenCount <= 2) {
                    ending = EndingType::ENDING_B_RETREAT;
                } else {
                    ending = EndingType::ENDING_C_WARNING;
                }

                std::cout << "Campaign completed with " << stolenCount << " stolen Echo(es). Transitioning to Ending "
                          << (ending == EndingType::ENDING_A_SHATTER ? "A (Shatter)" : (ending == EndingType::ENDING_B_RETREAT ? "B (Retreat)" : "C (Warning)")) << "\n";

                stateManager.clearAndSetState(std::make_unique<GameOverState>(stateManager, ending));
                return;
            }
        }
        stateManager.changeState(std::make_unique<GameplayState>(stateManager));
    }
}

void GameplayState::onChamberFailed() {
    std::cout << "GameplayState: Chamber Failed! Transitioning to GameOverState (Retry)...\n";
    stateManager.clearAndSetState(std::make_unique<GameOverState>(stateManager, std::nullopt));
}

void GameplayState::initPlayerPosition() {
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    
    float spawnX = -1.0f;
    float spawnY = -1.0f;

    if (activeChamber) {
        sf::Vector2f cfgSpawn = activeChamber->getPlayerSpawn();
        if (cfgSpawn.x >= 0.0f && cfgSpawn.y >= 0.0f) {
            spawnX = cfgSpawn.x;
            spawnY = cfgSpawn.y;
        } else {
            const auto& grid = activeChamber->getTypeGrid();
            bool found = false;
            // Search for the first walkable ground tile (type 0)
            for (size_t y = 1; y < grid.size() && !found; ++y) {
                for (size_t x = 1; x < grid[y].size() && !found; ++x) {
                    if (grid[y][x] == "L") {
                        spawnX = static_cast<float>(x) + 0.5f;
                        spawnY = static_cast<float>(y) + 0.5f;
                        found = true;
                    }
                }
            }
        }
    }

    if (spawnX < 0.0f || spawnY < 0.0f) {
        spawnX = 2.5f;
        spawnY = 2.5f;
    }

    if (player) {
        player->setPosition({ox + spawnX * cellSize, oy + spawnY * cellSize});
    }
}