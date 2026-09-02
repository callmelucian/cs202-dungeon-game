#include "tutorial-state.hpp"
#include "main-menu-state.hpp"
#include "pause-state.hpp"
#include "setting-state.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../global-settings/asset-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../ui/widgets/floating-text-manager.hpp"
#include "../../utils/collision-solver.hpp"
#include "../../utils/math-utility.hpp"
#include <cmath>
#include <algorithm>

TutorialState::TutorialState(StateManager& manager)
    : GameState(manager)
{
    drawBackground = false;
    setupUI();

    activeChamber = std::make_unique<TutorialChamber>(*player);
    if (activeChamber) {
        activeChamber->setObserver(this);
        activeChamber->setFormGuideCallback([this]() {
            showFormGuideModal();
        });
    }

    initPlayerPosition();
    startTutorialIntro();
}

TutorialState::~TutorialState() {
    UI::FloatingTextManager::getInstance().clear();
    ParticleSystem::getInstance().clear();
    AuraRenderer::getInstance().clearScreenFlash();
}

void TutorialState::setupUI() {
    SettingManager& settings = SettingManager::getInstance();

    root->setChildDefaults({
        .modeX = UI::SizeMode::Expanded,
        .modeY = UI::SizeMode::Expanded
    });

    // Intro title overlay
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
        ->setString("Tutorial - Controls & Forms Training Grounds");

    // HUD
    hud = root->createChild<UI::HUD>();

    // Instantiate Serin Player
    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);
    player->setHp(100.0f);
    player->switchForm(FormType::WRAITHBLADE);
    player->setMomentum(0.0f, FormType::WRAITHBLADE);
    player->setMomentum(0.0f, FormType::VOIDCASTER);
    player->setMomentum(0.0f, FormType::IRONSHELL);

    camera.init({static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())}, 0.5f);

    setupModalDialog();
}

void TutorialState::setupObjectiveModal() {
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
        ->setString("Objective: Learn movement, form switching, combat actions, and abilities.")
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

void TutorialState::proceedFromObjective() {
    if (objectivePhase != ObjectivePhase::FADING_OUT) {
        SoundManager::getInstance().playSound("menu-nav");
        objectivePhase = ObjectivePhase::FADING_OUT;
        objectiveTimer = 0.0f;
    }
}

void TutorialState::setupModalDialog() {
    modalRoot = std::make_unique<UI::Container>();
    modalRoot->setRoot(true);
    modalRoot->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Expanded)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setAlignmentY(UI::AlignmentY::Middle)
        ->setColor(sf::Color(0, 0, 0, 220));

    modalCard = modalRoot->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Fixed)
        ->setFixedWidth(1420.f)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(32.f)
        ->setPadding(45.f, 45.f, 45.f, 45.f)
        ->setColor(sf::Color(14, 18, 32, 252));

    // Title Header
    modalCard->createChild<UI::Text>("header", 24)
        ->setString("CHARACTER FORMS GUIDE")
        ->setFillColor(sf::Color(255, 215, 80))
        ->setMarginBottom(12.f);

    // 3 Cards Horizontal Layout
    auto* formsRow = modalCard->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(28.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    auto setupCard = [](UI::VerticalBox* card) {
        card->setModeX(UI::SizeMode::Fixed)
            ->setFixedWidth(410.f)
            ->setModeY(UI::SizeMode::Contained)
            ->setSpacing(8.f)
            ->setPadding(28.f, 28.f, 28.f, 28.f)
            ->setColor(sf::Color(22, 28, 48, 220));
    };

    // Card 1: Wraithblade
    auto* card1 = formsRow->createChild<UI::VerticalBox>();
    setupCard(card1);
    card1->createChild<UI::Text>("header", 14)
        ->setString("1. WRAITHBLADE")
        ->setFillColor(sf::Color(220, 130, 255))
        ->setMarginBottom(8.f);
    card1->createChild<UI::Text>("bold", 15)
        ->setString("Key [1] - Melee & Mobility")
        ->setFillColor(sf::Color(240, 200, 255))
        ->setMarginBottom(18.f);
    card1->createChild<UI::Text>("regular", 15)
        ->setString("> Attack: Fast melee slashes")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card1->createChild<UI::Text>("regular", 15)
        ->setString("> Sprint: [Right Click] Dash")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card1->createChild<UI::Text>("regular", 15)
        ->setString("> Role: Swift 1v1 duels & speed")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(10.f);

    // Card 2: Voidcaster
    auto* card2 = formsRow->createChild<UI::VerticalBox>();
    setupCard(card2);
    card2->createChild<UI::Text>("header", 14)
        ->setString("2. VOIDCASTER")
        ->setFillColor(sf::Color(100, 210, 255))
        ->setMarginBottom(8.f);
    card2->createChild<UI::Text>("bold", 15)
        ->setString("Key [2] - Ranged Archery")
        ->setFillColor(sf::Color(180, 235, 255))
        ->setMarginBottom(18.f);
    card2->createChild<UI::Text>("regular", 15)
        ->setString("> Attack: Piercing bow & arrow")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card2->createChild<UI::Text>("regular", 15)
        ->setString("> Aiming: Aim with Mouse cursor")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card2->createChild<UI::Text>("regular", 15)
        ->setString("> Role: Distant & isolated foes")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(10.f);

    // Card 3: Ironshell
    auto* card3 = formsRow->createChild<UI::VerticalBox>();
    setupCard(card3);
    card3->createChild<UI::Text>("header", 14)
        ->setString("3. IRONSHELL")
        ->setFillColor(sf::Color(255, 185, 90))
        ->setMarginBottom(8.f);
    card3->createChild<UI::Text>("bold", 15)
        ->setString("Key [3] - Defense & Crowd")
        ->setFillColor(sf::Color(255, 220, 170))
        ->setMarginBottom(18.f);
    card3->createChild<UI::Text>("regular", 15)
        ->setString("> Attack: Crushing ground slam")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card3->createChild<UI::Text>("regular", 15)
        ->setString("> Aura: Slows nearby enemies")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(12.f);
    card3->createChild<UI::Text>("regular", 15)
        ->setString("> Role: Heavy tank & swarms")
        ->setFillColor(sf::Color(235, 240, 255))
        ->setMarginBottom(10.f);

    // Close button
    modalCloseButton = modalCard->createChild<UI::Button>("Got It", "regular", 20)
        ->setModeX(UI::SizeMode::Fixed)
        ->setFixedWidth(200.f)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedHeight(50.f)
        ->setMarginTop(12.f)
        ->setOnClick([this]() {
            isModalOpen = false;
        });
}

void TutorialState::showFormGuideModal() {
    isModalOpen = true;
    SoundManager::getInstance().playSound("menu-nav");
}

void TutorialState::initPlayerPosition() {
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    float spawnX = 6.0f;
    float spawnY = 12.0f;

    if (activeChamber) {
        sf::Vector2f cfgSpawn = activeChamber->getPlayerSpawn();
        if (cfgSpawn.x >= 0.0f && cfgSpawn.y >= 0.0f) {
            spawnX = cfgSpawn.x;
            spawnY = cfgSpawn.y;
        }
    }

    if (player) {
        player->setPosition({ox + spawnX * cellSize, oy + spawnY * cellSize});
    }
}

void TutorialState::startTutorialIntro() {
    SettingManager& settings = SettingManager::getInstance();

    float gridMinX = settings.getGridOffsetX();
    float gridMinY = settings.getGridOffsetY();
    float gridWidth = 165.0f * settings.getCellSize();
    float gridHeight = 22.0f * settings.getCellSize();

    if (activeChamber) {
        const auto& grid = activeChamber->getTypeGrid();
        if (!grid.empty() && !grid[0].empty()) {
            gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
            gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
        }
    }

    float cellSize = settings.getCellSize();
    sf::Vector2f startIslandCenter({gridMinX + 6.0f * cellSize, gridMinY + 12.0f * cellSize});

    camera.setTargetCenter(startIslandCenter);
    camera.setTargetZoom(0.5f, 0.5f);
    camera.snapToTarget();

    if (player) {
        player->update(0.f);
    }

    if (titleContainer && chamberTitleText) {
        titleContainer->setColor(sf::Color(10, 10, 20, 220));
        chamberTitleText->setString("Tutorial - Controls & Forms Training Grounds");
    }

    setupObjectiveModal();
    objectivePhase = ObjectivePhase::FADING_IN;
    objectiveTimer = 0.0f;
    objectiveAlpha = 0.0f;
    introTimer = 0.0f;
    fadeTimer = 0.0f;
    fadeAlpha = 255.0f;
    transitionState = TutorialTransitionState::OBJECTIVE_DISPLAY;
}

void TutorialState::update(float deltaTime) {
    SettingManager& settings = SettingManager::getInstance();
    float gridMinX = settings.getGridOffsetX();
    float gridMinY = settings.getGridOffsetY();
    float gridWidth = 165.0f * settings.getCellSize();
    float gridHeight = 22.0f * settings.getCellSize();

    if (activeChamber) {
        const auto& grid = activeChamber->getTypeGrid();
        if (!grid.empty() && !grid[0].empty()) {
            gridWidth = static_cast<float>(grid[0].size()) * settings.getCellSize();
            gridHeight = static_cast<float>(grid.size()) * settings.getCellSize();
        }
    }

    sf::FloatRect mapBounds({gridMinX, gridMinY}, {gridWidth, gridHeight});

    // Handle Transitions
    if (transitionState == TutorialTransitionState::OBJECTIVE_DISPLAY) {
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
                transitionState = TutorialTransitionState::FADING_IN;
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
    } else if (transitionState == TutorialTransitionState::FADING_IN) {
        fadeTimer += deltaTime;
        fadeAlpha = std::max(0.0f, 255.0f * (1.0f - fadeTimer / FADE_DURATION));
        camera.update(0.0f, mapBounds);

        if (fadeTimer >= FADE_DURATION) {
            fadeAlpha = 0.0f;
            transitionState = TutorialTransitionState::TITLE_DISPLAY;
            introTimer = 0.0f;
        }
        GameState::update(deltaTime);
        return;
    } else if (transitionState == TutorialTransitionState::TITLE_DISPLAY) {
        introTimer += deltaTime;
        camera.update(0.0f, mapBounds);

        if (introTimer >= 1.2f) {
            if (titleContainer && chamberTitleText) {
                titleContainer->setColor(sf::Color::Transparent);
                chamberTitleText->setString("");
            }
            transitionState = TutorialTransitionState::PLAYING;
            if (player) {
                camera.setTargetCenter(player->getPosition());
                camera.setTargetZoom(0.5f, 0.5f);
            }
        }
        GameState::update(deltaTime);
        return;
    } else if (transitionState == TutorialTransitionState::FADING_OUT) {
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
            transitionState = TutorialTransitionState::COMPLETED;
            onChamberCompleted();
        }
        return;
    }

    // Modal Pause: if Form Guide dialog is open, update modal UI and skip world updates
    if (isModalOpen) {
        if (modalRoot) {
            modalRoot->computeSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
            modalRoot->setPosition({0.f, 0.f});
            modalRoot->update(deltaTime);
        }
        GameState::update(deltaTime);
        return;
    }

    // 1. Update Player
    Game::getInstance().setActiveWorldView(camera.getView());
    if (player) {
        player->update(deltaTime);
    }

    // 2. Resolve Collisions
    if (activeChamber && player) {
        CollisionSolver::resolveX(*player, activeChamber->getObstaclesFor(player.get()), deltaTime);
        CollisionSolver::resolveY(*player, activeChamber->getObstaclesFor(player.get()), deltaTime);
    }

    // 3. Update Chamber & Effects
    if (activeChamber) {
        activeChamber->update(deltaTime);
    }
    ParticleSystem::getInstance().update(deltaTime);
    AuraRenderer::getInstance().update(deltaTime);
    UI::FloatingTextManager::getInstance().update(deltaTime);

    // 4. Check Exit Gate on Island 8
    if (transitionState == TutorialTransitionState::PLAYING && activeChamber && activeChamber->getExitGate() && player) {
        if (activeChamber->getExitGate()->checkPlayerOverlap(*player, 0.5f)) {
            transitionState = TutorialTransitionState::FADING_OUT;
            fadeTimer = 0.0f;
            fadeAlpha = 0.0f;
            player->setVelocity({0.0f, 0.0f});
            player->setKnockbackVelocity({0.0f, 0.0f});
            SoundManager::getInstance().playSound("echo-collect");
        }
    }

    // 5. Update HUD
    if (player && hud) {
        hud->updatePlayerState(*player);
    }

    // 6. Update Camera: Follow player with smart framing
    if (player) {
        float cellSize = settings.getCellSize();
        float playerCol = (player->getPosition().x - gridMinX) / cellSize;

        if (playerCol >= 85.0f && playerCol <= 97.0f) {
            // Island 5: Frame both player platform (row 14) and isolated enemy platform (row 6)
            sf::Vector2f island5ViewCenter({gridMinX + 90.0f * cellSize, gridMinY + 10.5f * cellSize});
            camera.setTargetCenter(island5ViewCenter);
            camera.setTargetZoom(0.58f, 0.58f);
        } else {
            camera.setTargetCenter(player->getPosition());
            if (playerCol < 148.0f) {
                camera.setTargetZoom(0.5f, 0.5f);
            }
        }

        camera.update(deltaTime, mapBounds);
    }

    GameState::update(deltaTime);
}

void TutorialState::draw(sf::RenderWindow& window) const {
    sf::View uiView = window.getDefaultView();

    window.clear(sf::Color(18, 20, 28));

    // Apply Camera View for World
    Game::getInstance().setActiveWorldView(camera.getView());
    window.setView(camera.getView());

    if (activeChamber) activeChamber->draw(window);
    if (player) player->draw(window);

    window.draw(ParticleSystem::getInstance());

    if (AuraRenderer::getInstance().hasScreenFlash()) {
        window.setView(uiView);
        AuraRenderer::getInstance().drawScreenFlash(window, window.getSize());
        window.setView(camera.getView());
    }

    UI::FloatingTextManager::getInstance().draw(window);

    // Restore UI View
    window.setView(uiView);
    GameState::draw(window);

    // Draw Objective Modal Dialog
    if (transitionState == TutorialTransitionState::OBJECTIVE_DISPLAY) {
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

    // Draw Modal Dialog on top of HUD
    if (isModalOpen && modalRoot) {
        modalRoot->draw(window);
    }

    // Full screen fade transition
    if (fadeAlpha > 0.0f && transitionState != TutorialTransitionState::OBJECTIVE_DISPLAY) {
        SettingManager& settings = SettingManager::getInstance();
        fadeOverlay.setSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
        fadeOverlay.setPosition({0.0f, 0.0f});
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(std::clamp(fadeAlpha, 0.0f, 255.0f))));
        window.draw(fadeOverlay);
    }
}

void TutorialState::handleEvents(sf::Event& event) {
    if (transitionState == TutorialTransitionState::OBJECTIVE_DISPLAY) {
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

    if (isModalOpen) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->scancode == sf::Keyboard::Scancode::Enter || 
                keyEvent->scancode == sf::Keyboard::Scancode::Space ||
                keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                isModalOpen = false;
                return;
            }
        }
        if (modalRoot) {
            modalRoot->handleEvent(event);
        }
        return;
    }

    if (transitionState != TutorialTransitionState::PLAYING) {
        GameState::handleEvents(event);
        return;
    }

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
            stateManager.pushState(std::make_unique<PauseState>(stateManager));
            return;
        }
    }

    if (player) {
        player->handleInput(event);
    }

    SettingManager& settings = SettingManager::getInstance();

    // Check for Attack action: Key J / Attack keybinding OR Left Mouse Button
    bool isAttackTriggered = false;
    if (settings.matchesEvent("Attack", event)) {
        isAttackTriggered = true;
    } else if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            isAttackTriggered = true;
        }
    }

    if (isAttackTriggered && activeChamber && player) {
        sf::Vector2f dir;
        if (player->getActiveFormType() == FormType::VOIDCASTER) {
            sf::RenderWindow& window = Game::getInstance().getWindow();
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixel, camera.getView());
            dir = mouseWorldPos - player->getPosition();
            float len = Math::length(dir);
            if (len > 0.001f) dir /= len;
            else dir = player->getFacingVector();
        } else {
            dir = player->getVelocity();
            if (Math::length(dir) < 0.001f) {
                dir = player->getFacingVector();
            } else {
                dir = Math::normalize(dir);
            }
        }
        player->attack(dir, *activeChamber);
    }

    // Zoom handling: ONLY permitted on Island 8 (col >= 148.0f)
    if (player) {
        float ox = settings.getGridOffsetX();
        float playerCol = (player->getPosition().x - ox) / settings.getCellSize();

        if (playerCol >= 148.0f) {
            if (const auto* scrollEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
                if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
                    float gridWidth = 165.0f * settings.getCellSize();
                    float gridHeight = 22.0f * settings.getCellSize();
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
        }
    }

    GameState::handleEvents(event);
}

void TutorialState::onChamberCompleted() {
    stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
}

void TutorialState::onChamberFailed() {
    stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
}
