#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "echo-log-state.hpp"
#include "debug-state.hpp"
#include "game-over-state.hpp"
#include "main-menu-state.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../chambers/protect-chamber.hpp"
#include "../../chambers/boss-chamber.hpp"
#include "../../chambers/map-loader.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../ui/widgets/floating-text-manager.hpp"
#include "choose-chamber-state.hpp"
#include "../game.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include "../../global-settings/sound-manager.hpp"
#include <cmath>
#include <iostream>

GameplayState::GameplayState(StateManager& manager, bool isIndividualMode)
    : GameState(manager), isDebugMode(false), isIndividualMode(isIndividualMode)
{
    drawBackground = false;
    setupUI();
    
    RunState& runState = Game::getInstance().getRunState();
    if (runState.currentLevel >= 4 && runState.hasDecoyReliquaryBuff && player) {
        player->setMaxHp(120.0f);
        player->heal(20.0f);
    }
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
    std::string objectiveStr;

    if (filepath.find("level-1/chamber-1.json") != std::string::npos) {
        titleStr = "Level 1 - The Drowned Archive (Clarity Shard)";
        objectiveStr = "Protect the Clarity Shard from approaching enemy waves until time expires.";
    } else if (filepath.find("level-1/chamber-2.json") != std::string::npos) {
        titleStr = "Level 1 - The Bone Corridor (Marrow Echo)";
        objectiveStr = "Defend the Marrow Echo against corrupted enemies and prevent its destruction.";
    } else if (filepath.find("level-1/chamber-3.json") != std::string::npos) {
        titleStr = "Level 1 - The Collapsed Barracks (Gauntlet)";
        objectiveStr = "Survive and eliminate all enemy waves to unseal the exit portal.";
    } else if (filepath.find("level-2/chamber-1.json") != std::string::npos) {
        titleStr = "Level 2 - The Drowned Choir (Hollow Bell)";
        objectiveStr = "Protect the Hollow Bell Echo from waves of dark horrors until time runs out.";
    } else if (filepath.find("level-2/chamber-2.json") != std::string::npos) {
        titleStr = "Level 2 - The Choir Loft (Prevent)";
        objectiveStr = "Prevent corrupted enemies from escaping into the upper rift.";
    } else if (filepath.find("level-2/chamber-3.json") != std::string::npos) {
        titleStr = "Level 2 - The Silent Nave (Gauntlet)";
        objectiveStr = "Defeat all incoming enemy waves in the nave to open the exit portal.";
    } else if (filepath.find("level-3/chamber-1.json") != std::string::npos) {
        titleStr = "Level 3 - The Resonance Hall (Resonance Core)";
        objectiveStr = "Guard the Resonance Core from assault until resonance stabilizes.";
    } else if (filepath.find("level-3/chamber-2.json") != std::string::npos) {
        titleStr = "Level 3 - The Mirror Vault (Obsidian Key)";
        objectiveStr = "Defend the Obsidian Key from incoming enemy waves.";
    } else if (filepath.find("level-3/chamber-3.json") != std::string::npos) {
        titleStr = "Level 3 - The Hunger Pit (Gauntlet)";
        objectiveStr = "Survive the relentless onslaught in the pit to unseal the gate.";
    } else if (filepath.find("mid.json") != std::string::npos) {
        titleStr = "Resting Sanctuary (Free Form Swap)";
        objectiveStr = "Rest safely and freely customize character forms.";
    } else if (filepath.find("boss.json") != std::string::npos) {
        titleStr = "The Heart of the Ashen Vault (Boss: Malachar)";
        objectiveStr = "Defeat Lord Malachar and claim the core of the Ashen Vault.";
    } else {
        ChamberConfig cfg = MapLoader::loadChamber(filepath);
        titleStr = "Level " + std::to_string(runState.currentLevel) + " - Chamber " + std::to_string(runState.currentChamber);
        objectiveStr = "Defeat all enemies and complete chamber objectives.";
    }
    startChamberIntro(titleStr, objectiveStr);
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
    startChamberIntro("Debug Chamber", "> Test mechanics, combat, and enemy interactions.");
}

GameplayState::~GameplayState() {
    UI::FloatingTextManager::getInstance().clear();
    ParticleSystem::getInstance().clear();
    AuraRenderer::getInstance().clearScreenFlash();
}

void GameplayState::setupUI() {
    SettingManager& settings = SettingManager::getInstance();
    // root->setAlignmentY(UI::AlignmentY::Middle);
    root->setChildDefaults({
        .modeX = UI::SizeMode::Expanded,
        .modeY = UI::SizeMode::Expanded
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
        player->setMomentum(runState.wraithbladeMomentum, FormType::WRAITHBLADE);
        player->setMomentum(runState.voidcasterMomentum, FormType::VOIDCASTER);
        player->setMomentum(runState.ironshellMomentum, FormType::IRONSHELL);
        player->setSpecial1Threshold(runState.special1MomentumThreshold);
    }

    camera.init({static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())}, 0.5f);
}

void GameplayState::setupObjectiveModal(const std::string& /*titleStr*/, const std::string& objectiveStr) {
    objectiveModal = std::make_unique<UI::Container>();
    objectiveModal->setRoot(true);
    objectiveModal->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Expanded)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setAlignmentY(UI::AlignmentY::Middle)
        ->setColor(sf::Color(0, 0, 0, 255));

    auto* centerBox = objectiveModal->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(45.f)
        ->setPadding(20.f, 20.f, 20.f, 20.f);

    // Single white text in regular font with margin bottom for spacing
    centerBox->createChild<UI::Text>("regular", 22)
        ->setString("Objective: " + objectiveStr)
        ->setFillColor(sf::Color::White)
        ->setMarginBottom(40.f);

    // Continue button
    centerBox->createChild<UI::Button>("Continue", "regular", 20)
        ->setModeX(UI::SizeMode::Fixed)
        ->setFixedWidth(200.f)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedHeight(48.f)
        ->setOnClick([this]() {
            proceedFromObjective();
        });
}

void GameplayState::proceedFromObjective() {
    if (objectivePhase != ObjectivePhase::FADING_OUT) {
        SoundManager::getInstance().playSound("menu-nav");
        objectivePhase = ObjectivePhase::FADING_OUT;
        objectiveTimer = 0.0f;
    }
}

void GameplayState::startChamberIntro(const std::string& titleStr, const std::string& objectiveStr) {
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

    if (player) {
        player->update(0.f);
    }

    if (titleContainer && chamberTitleText) {
        titleContainer->setColor(sf::Color(10, 10, 20, 220));
        chamberTitleText->setString(titleStr);
    }

    setupObjectiveModal(titleStr, objectiveStr);
    currentChamberTitle = titleStr;
    chamberElapsedTime = 0.0f;
    objectivePhase = ObjectivePhase::FADING_IN;
    objectiveTimer = 0.0f;
    objectiveAlpha = 0.0f;
    introTimer = 0.0f;
    fadeTimer = 0.0f;
    fadeAlpha = 255.0f;
    transitionState = ChamberTransitionState::OBJECTIVE_DISPLAY;
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

    if (transitionState == ChamberTransitionState::OBJECTIVE_DISPLAY) {
        if (objectivePhase == ObjectivePhase::FADING_IN) {
            objectiveTimer += deltaTime;
            objectiveAlpha = std::min(255.0f, (objectiveTimer / OBJECTIVE_FADE_DURATION) * 255.0f);
            if (objectiveTimer >= OBJECTIVE_FADE_DURATION) {
                objectiveAlpha = 255.0f;
                objectivePhase = ObjectivePhase::DISPLAY;
            }
        } else if (objectivePhase == ObjectivePhase::FADING_OUT) {
            objectiveTimer += deltaTime;
            objectiveAlpha = std::max(0.0f, 255.0f * (1.0f - objectiveTimer / OBJECTIVE_FADE_DURATION));
            if (objectiveTimer >= OBJECTIVE_FADE_DURATION) {
                objectiveAlpha = 0.0f;
                transitionState = ChamberTransitionState::FADING_IN;
                fadeTimer = 0.0f;
                fadeAlpha = 255.0f;
            }
        }

        if (objectiveModal) {
            objectiveModal->computeSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
            objectiveModal->setPosition({0.f, 0.f});
            objectiveModal->update(deltaTime);
        }
        GameState::update(deltaTime);
        return;
    } else if (transitionState == ChamberTransitionState::FADING_IN) {
        fadeTimer += deltaTime;
        fadeAlpha = std::max(0.0f, 255.0f * (1.0f - fadeTimer / FADE_DURATION));
        camera.update(0.0f, mapBounds); // Camera stays frozen at zoomed out view

        if (fadeTimer >= FADE_DURATION) {
            fadeAlpha = 0.0f;
            transitionState = ChamberTransitionState::TITLE_DISPLAY;
            introTimer = 0.0f;
        }
        GameState::update(deltaTime);
        return; // Skip player and enemy updates during initial fade-in
    } else if (transitionState == ChamberTransitionState::TITLE_DISPLAY) {
        introTimer += deltaTime;
        camera.update(0.0f, mapBounds); // Camera stays frozen at zoomed out view

        if (introTimer >= 1.5f) {
            if (titleContainer && chamberTitleText) {
                titleContainer->setColor(sf::Color::Transparent);
                chamberTitleText->setString("");
            }
            transitionState = ChamberTransitionState::ZOOMING_IN;
            if (player) {
                float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                            gridHeight / static_cast<float>(settings.getWindowHeight()));
                camera.setTargetCenter(player->getPosition());
                camera.setTargetZoom(0.5f, maxZoomOut);
            }
        }
        GameState::update(deltaTime);
        return; // Skip player and enemy updates so characters remain frozen!
    } else if (transitionState == ChamberTransitionState::ZOOMING_IN) {
        if (player) {
            float maxZoomOut = std::max(gridWidth / static_cast<float>(settings.getWindowWidth()), 
                                        gridHeight / static_cast<float>(settings.getWindowHeight()));
            camera.setTargetCenter(player->getPosition());
            camera.update(deltaTime, mapBounds);

            if (std::abs(camera.getCurrentZoom() - camera.getTargetZoom()) < 0.03f) {
                transitionState = ChamberTransitionState::PLAYING;
                // Return immediately — game logic (chamber update, player movement) runs
                // from the NEXT frame so that completeChamber() cannot fire on the same
                // tick the intro ends.
                GameState::update(deltaTime);
                return;
            }
        }
        GameState::update(deltaTime);
        return; // Skip player and enemy updates while camera zooms in!
    } else if (transitionState == ChamberTransitionState::FADING_OUT) {
        fadeTimer += deltaTime;
        fadeAlpha = std::min(255.0f, (fadeTimer / FADE_DURATION) * 255.0f);
        
        if (player) {
            player->setVelocity({0.0f, 0.0f});
            player->setKnockbackVelocity({0.0f, 0.0f});
            player->update(deltaTime);
        }

        if (activeChamber) {
            activeChamber->update(deltaTime);
        }
        ParticleSystem::getInstance().update(deltaTime);
        AuraRenderer::getInstance().update(deltaTime);
        UI::FloatingTextManager::getInstance().update(deltaTime);

        if (player && hud) {
            hud->updatePlayerState(*player);
        }

        if (player) {
            camera.setTargetCenter(player->getPosition());
            camera.update(deltaTime, mapBounds);
        }

        GameState::update(deltaTime);

        if (fadeTimer >= FADE_DURATION) {
            transitionState = ChamberTransitionState::COMPLETED;
            onChamberCompleted();
        }
        return;
    }
    // 1. Update player logic (including real-time WASD movement)
    Game::getInstance().setActiveWorldView(camera.getView());
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
    AuraRenderer::getInstance().update(deltaTime);
    UI::FloatingTextManager::getInstance().update(deltaTime);

    // Check if player entered an active exit gate (bounding box overlap >= 50%)
    if (transitionState == ChamberTransitionState::PLAYING && activeChamber && activeChamber->getExitGate() && player) {
        if (activeChamber->getExitGate()->checkPlayerOverlap(*player, 0.5f)) {
            transitionState = ChamberTransitionState::FADING_OUT;
            fadeTimer = 0.0f;
            fadeAlpha = 0.0f;
            player->setVelocity({0.0f, 0.0f});
            player->setKnockbackVelocity({0.0f, 0.0f});
            SoundManager::getInstance().playSound("echo-collect");
        }
    }

    // 3. Update HUD data — lerp animation runs via the UI tree's Container::update()
    if (player && hud) {
        hud->updatePlayerState(*player);
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
    Game::getInstance().setActiveWorldView(camera.getView());
    window.setView(camera.getView());

    if (activeChamber) activeChamber->draw(window);
    
    // Draw player
    if (player) player->draw(window);
    
    // Draw particles on top of world
    window.draw(ParticleSystem::getInstance());
    
    // Draw screen flash if active (drawn over world before floating text, so red crit text stays visible on top!)
    if (AuraRenderer::getInstance().hasScreenFlash()) {
        window.setView(uiView);
        AuraRenderer::getInstance().drawScreenFlash(window, window.getSize());
        window.setView(camera.getView());
    }

    // Draw floating text in world space (visible on top of the white flash!)
    UI::FloatingTextManager::getInstance().draw(window);
    
    // Restore UI View for HUD
    window.setView(uiView);
    GameState::draw(window);

    // Draw objective modal on top of HUD when in OBJECTIVE_DISPLAY
    if (transitionState == ChamberTransitionState::OBJECTIVE_DISPLAY) {
        if (objectiveModal) {
            objectiveModal->draw(window);
        }
        if (objectiveAlpha < 255.0f) {
            SettingManager& settings = SettingManager::getInstance();
            fadeOverlay.setSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
            fadeOverlay.setPosition({0.0f, 0.0f});
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(std::clamp(255.0f - objectiveAlpha, 0.0f, 255.0f))));
            window.draw(fadeOverlay);
        }
    }

    // Draw full-screen fade transition overlay on top of everything
    if (fadeAlpha > 0.0f && transitionState != ChamberTransitionState::OBJECTIVE_DISPLAY) {
        SettingManager& settings = SettingManager::getInstance();
        fadeOverlay.setSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
        fadeOverlay.setPosition({0.0f, 0.0f});
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(std::clamp(fadeAlpha, 0.0f, 255.0f))));
        window.draw(fadeOverlay);
    }
}

void GameplayState::handleEvents(sf::Event& event) {
    if (transitionState == ChamberTransitionState::OBJECTIVE_DISPLAY) {
        if (objectivePhase != ObjectivePhase::FADING_OUT) {
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Enter || 
                    keyEvent->scancode == sf::Keyboard::Scancode::Space ||
                    keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                    proceedFromObjective();
                    return;
                }
            }
            if (objectiveModal) {
                objectiveModal->handleEvent(event);
            }
        }
        return;
    }

    if (transitionState != ChamberTransitionState::PLAYING) {
        // Pass events to UI components but skip player gameplay inputs
        GameState::handleEvents(event);
        return;
    }

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
            stateManager.pushState(std::make_unique<PauseState>(stateManager));
            return;
        }
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
                    if (len > 0.001f) dir /= len;
                    else dir = player->getFacingVector();
                } else {
                    dir = player->getVelocity();
                    if (std::abs(dir.x) < 0.001f && std::abs(dir.y) < 0.001f) {
                        dir = player->getFacingVector();
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

void GameplayState::onEchoPowerChanged(float /*power*/) {
}

void GameplayState::onChamberCompleted() {
    if (transitionState != ChamberTransitionState::COMPLETED && transitionState != ChamberTransitionState::FADING_OUT) {
        transitionState = ChamberTransitionState::FADING_OUT;
        fadeTimer = 0.0f;
        fadeAlpha = 0.0f;
        if (player) {
            player->setVelocity({0.0f, 0.0f});
            player->setKnockbackVelocity({0.0f, 0.0f});
        }
        SoundManager::getInstance().playSound("echo-collect");
        return;
    }

    if (isIndividualMode) {
        stateManager.popState();
        return;
    }
    if (player) {
        RunState& runState = Game::getInstance().getRunState();
        runState.playerHP = player->getHp();
        runState.activeForm = player->getActiveFormType();
        runState.wraithbladeMomentum = player->getMomentum(FormType::WRAITHBLADE);
        runState.voidcasterMomentum = player->getMomentum(FormType::VOIDCASTER);
        runState.ironshellMomentum = player->getMomentum(FormType::IRONSHELL);
    }
    if (isDebugMode) {
        stateManager.changeState(std::make_unique<ChooseChamberState>(stateManager));
    } else {
        RunState& runState = Game::getInstance().getRunState();
        std::string nextPath = MapLoader::getChamberFilepath(runState.currentLevel, runState.currentChamber + 1);
        
        if (!nextPath.empty()) {
            // Next chamber in same level
            runState.currentChamber++;
            SaveLoadManager::getInstance().saveGame(runState);
        } else {
            // Check if there's a next level
            std::string nextLevelPath = MapLoader::getChamberFilepath(runState.currentLevel + 1, 1);
            if (!nextLevelPath.empty()) {
                runState.currentLevel++;
                runState.currentChamber = 1;
                SaveLoadManager::getInstance().saveGame(runState);
            } else {
                // Game completely over! (Win)
                std::remove("savegame.json");
                int stolenCount = 0;
                int collectedCount = 0;
                for (const auto& [type, outcome] : runState.echoOutcomes) {
                    if (outcome == EchoOutcome::STOLEN) {
                        stolenCount++;
                    } else if (outcome == EchoOutcome::COLLECTED) {
                        collectedCount++;
                    }
                }
                int uncollectedCount = 5 - (collectedCount + stolenCount);
                if (uncollectedCount > 0) {
                    stolenCount += uncollectedCount;
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
    if (isIndividualMode) {
        stateManager.popState();
        return;
    }
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
        const auto& grid = activeChamber->getTypeGrid();
        bool validSpawn = false;

        if (cfgSpawn.x >= 0.0f && cfgSpawn.y >= 0.0f && !grid.empty()) {
            int r = static_cast<int>(cfgSpawn.y);
            int c = static_cast<int>(cfgSpawn.x);
            if (r >= 0 && r < static_cast<int>(grid.size()) &&
                c >= 0 && c < static_cast<int>(grid[r].size())) {
                const std::string& t = grid[r][c];
                if (t == "L" || t == "S" || t == "V" || t == "H" || t == "E" || t == "X") {
                    spawnX = cfgSpawn.x;
                    spawnY = cfgSpawn.y;
                    validSpawn = true;
                }
            }
        }

        if (!validSpawn) {
            bool found = false;
            // Search for the first walkable ground tile ("L")
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