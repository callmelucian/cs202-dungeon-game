#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "game-over-state.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../chambers/protect-chamber.hpp"
#include "../../global-settings/map-loader.hpp"
#include "choose-chamber-state.hpp"
#include "../game.hpp"
#include <cmath>

GameplayState::GameplayState(StateManager& manager) : GameState(manager), isDebugMode(false) {
    setupUI();
    
    RunState& runState = Game::getInstance().getRunState();
    activeChamber = ChamberFactory::createChamber(runState.currentLevel, runState.currentChamber, *player);
    if (activeChamber) {
        activeChamber->setObserver(this);
    }
    
    auto* protect = dynamic_cast<ProtectChamber*>(activeChamber.get());
    if (protect && protect->getEcho()) {
        Echo* echo = protect->getEcho();
        echo->attach(this);
        echoPowerText->setString("Echo Power: " + std::to_string((int)echo->getPower()) + "%");
        echoPowerText->setMarginBottom(15.f);
        cooldownText->setMarginBottom(15.f);
    }

    initPlayerPosition();

    std::string filepath = MapLoader::getChamberFilepath(runState.currentLevel, runState.currentChamber);
    ChamberConfig cfg = MapLoader::loadChamber(filepath);
    std::string chamberName = cfg.chamberType;
    std::string titleStr = "Level " + std::to_string(runState.currentLevel) + " - Chamber " + std::to_string(runState.currentChamber) + ": " + chamberName;
    startChamberIntro(titleStr);
}

GameplayState::GameplayState(StateManager& manager, ChamberSelectionType type) : GameState(manager), isDebugMode(true) {
    setupUI();
    
    activeChamber = ChamberFactory::createDebugChamber(type, *player);
    if (activeChamber) {
        activeChamber->setObserver(this);
    }
    
    auto* protect = dynamic_cast<ProtectChamber*>(activeChamber.get());
    if (protect && protect->getEcho()) {
        Echo* echo = protect->getEcho();
        echo->attach(this);
        echoPowerText->setString("Echo Power: " + std::to_string((int)echo->getPower()) + "%");
        echoPowerText->setMarginBottom(15.f);
        cooldownText->setMarginBottom(15.f);
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
        // ->setFixedWidth(SettingManager::getInstance().getWindowWidth())
        // ->setFixedHeight(SettingManager::getInstance().getWindowHeight())
        ->setAlignmentX(UI::AlignmentX::Right)
        ->setAlignmentY(UI::AlignmentY::Top)
        ->setPadding(20.f, 20.f, 20.f, 20.f);

    playerInfoBoxWrapper = root->createChild<UI::Container>()
        ->setAlignmentX(UI::AlignmentX::Left)
        ->setAlignmentY(UI::AlignmentY::Bottom)
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
    quitButton = buttonBox->createChild<UI::Button>("Quit Game", "regular", 25)
        ->setOnClick([this]() {
            stateManager.changeState(std::make_unique<GameOverState>(stateManager, EndingType::ENDING_A_SHATTER));
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

    // Create HUD at bottom left with a fixed width to prevent text layout jitter
    // hudBox = root->createChild<UI::VerticalBox>()
    //     ->setModeX(UI::SizeMode::Fixed)
    //     ->setFixedWidth(300.f)
    //     ->setModeY(UI::SizeMode::Contained)
    //     ->setAlignmentX(UI::AlignmentX::Left)
    //     ->setAlignmentY(UI::AlignmentY::Bottom)
    //     ->setPadding(20.f, 20.f, 20.f, 20.f);

    cooldownText = new UI::Text("regular", 24);
    echoPowerText = new UI::Text("regular", 24);
    // formText = hudBox->createChild<UI::Text>("regular", 24)->setString("Form: Wraithblade")->setFixedHeight(30.f)->setMarginBottom(15.f);
    // hpText = hudBox->createChild<UI::Text>("regular", 24)->setString("HP: 100/100")->setFixedHeight(30.f)->setMarginBottom(15.f);
    // momentumText = hudBox->createChild<UI::Text>("regular", 24)->setString("Momentum: 0")->setFixedHeight(30.f)->setMarginBottom(15.f);
    // cooldownText = hudBox->createChild<UI::Text>("regular", 24)->setString("Cooldown: Ready")->setFixedHeight(30.f);
    // echoPowerText = hudBox->createChild<UI::Text>("regular", 24)->setString("")->setFixedHeight(30.f);

    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);

    camera.init({static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())}, 0.5f);
}

void GameplayState::startChamberIntro(const std::string& titleStr) {
    SettingManager& settings = SettingManager::getInstance();

    float gridMinX = settings.getGridOffsetX();
    float gridMinY = settings.getGridOffsetY();
    float gridWidth = settings.getGridCols() * settings.getCellSize();
    float gridHeight = settings.getGridRows() * settings.getCellSize();

    if (activeChamber) {
        const auto& grid = activeChamber->getGrid();
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
        const auto& grid = activeChamber->getGrid();
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

    // 3. Update HUD text
    FormType currentForm = player->getActiveFormType();
    std::string formStr = "Unknown";
    if (currentForm == FormType::WRAITHBLADE) formStr = "Wraithblade";
    else if (currentForm == FormType::VOIDCASTER) formStr = "Voidcaster";
    else if (currentForm == FormType::IRONSHELL) formStr = "Ironshell";
    // formText->setString("Form: " + formStr);
    
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

    // 4. Update camera position lerp & zoom lerp
    if (player) {
        camera.setTargetCenter(player->getPosition());

        SettingManager& settings = SettingManager::getInstance();
        float gridMinX = settings.getGridOffsetX();
        float gridMinY = settings.getGridOffsetY();
        float gridWidth = settings.getGridCols() * settings.getCellSize();
        float gridHeight = settings.getGridRows() * settings.getCellSize();

        if (activeChamber) {
            const auto& grid = activeChamber->getGrid();
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
                const auto& grid = activeChamber->getGrid();
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
                stateManager.changeState(std::make_unique<GameOverState>(stateManager, EndingType::ENDING_A_SHATTER));
                return;
            }
        }
        stateManager.changeState(std::make_unique<GameplayState>(stateManager));
    }
}

void GameplayState::onChamberFailed() {
    std::cout << "GameplayState: Chamber Failed! Transitioning to GameOverState (Retry)...\n";
    stateManager.changeState(std::make_unique<GameOverState>(stateManager, std::nullopt));
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
            const auto& grid = activeChamber->getGrid();
            bool found = false;
            // Search for the first walkable ground tile (type 0)
            for (size_t y = 1; y < grid.size() && !found; ++y) {
                for (size_t x = 1; x < grid[y].size() && !found; ++x) {
                    if (grid[y][x] == 0) {
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