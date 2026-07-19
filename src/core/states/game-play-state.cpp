#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "game-over-state.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../chambers/protect-chamber.hpp"
#include "../game.hpp"
#include <cmath>

GameplayState::GameplayState(StateManager& manager, ChamberSelectionType type) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create an overlaying container
    overlays = root->createChild<UI::Container>()
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(SettingManager::getInstance().getWindowWidth())
        ->setFixedHeight(SettingManager::getInstance().getWindowHeight())
        ->setAlignmentX(UI::AlignmentX::Right)
        ->setAlignmentY(UI::AlignmentY::Top)
        ->setPadding(20.f, 20.f, 20.f, 20.f);

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = overlays->createChild<UI::HorizontalBox>()
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

    // Create HUD at bottom left with a fixed width to prevent text layout jitter
    hudBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Fixed)
        ->setFixedWidth(300.f)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Left)
        ->setAlignmentY(UI::AlignmentY::Bottom)
        ->setPadding(20.f, 20.f, 20.f, 20.f);
        // Note: setSpacing() is ignored by FlexBox when using FlexStart distribution without Expanded children,
        // so we use setMarginBottom on the text elements instead.

    formText = hudBox->createChild<UI::Text>("regular", 24)->setString("Form: Wraithblade")->setFixedHeight(30.f)->setMarginBottom(15.f);
    hpText = hudBox->createChild<UI::Text>("regular", 24)->setString("HP: 100/100")->setFixedHeight(30.f)->setMarginBottom(15.f);
    momentumText = hudBox->createChild<UI::Text>("regular", 24)->setString("Momentum: 0")->setFixedHeight(30.f)->setMarginBottom(15.f);
    cooldownText = hudBox->createChild<UI::Text>("regular", 24)->setString("Cooldown: Ready")->setFixedHeight(30.f);
    echoPowerText = hudBox->createChild<UI::Text>("regular", 24)->setString("")->setFixedHeight(30.f);

    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    player->setPosition({ox + 5.5f * cellSize, oy + 5.5f * cellSize}); // Spawn exactly in the center of cell (5, 5)

    activeChamber = ChamberFactory::createChamber(type, 1, 1, *player);
    // Only Protect Chambers have a physical Echo artifact.
    // If we are in one, safely cast the active chamber to access the Echo and register GameplayState as its observer.
    auto* protect = dynamic_cast<ProtectChamber*>(activeChamber.get());
    if (protect && protect->getEcho()) {
        Echo* echo = protect->getEcho();
        echo->attach(this); // Register GameplayState (this) to receive power level callbacks
        
        // Populate and format the HUD element for the active Echo power
        echoPowerText->setString("Echo Power: " + std::to_string((int)echo->getPower()) + "%");
        echoPowerText->setMarginBottom(15.f);
        cooldownText->setMarginBottom(15.f);
    }

    // Initialize camera View
    currentZoom = 0.5f;
    cameraView.setSize({static_cast<float>(settings.getWindowWidth()) * currentZoom, static_cast<float>(settings.getWindowHeight()) * currentZoom});
}

void GameplayState::update(float deltaTime) {
    // 1. Update player logic (including real-time WASD movement)
    player->update(deltaTime);

    // 2. Resolve movement collisions
    // NOTE: CollisionSolver::resolveAABB applies velocity to position internally.
    // Do NOT manually integrate position (pos += vel * dt) in Character::update() 
    // or Player::update() to prevent double-movement bugs!
    if (activeChamber) {
        CollisionSolver::resolveAABB(*player, activeChamber->getObstacles(), deltaTime);
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

    // 4. Update camera
    if (player) {
        sf::Vector2f playerPos = player->getPosition();
        
        SettingManager& settings = SettingManager::getInstance();
        float viewWidth = cameraView.getSize().x;
        float viewHeight = cameraView.getSize().y;
        
        // Define world boundaries based on grid
        float gridMinX = settings.getGridOffsetX();
        float gridMinY = settings.getGridOffsetY();
        float gridMaxX = gridMinX + settings.getGridCols() * settings.getCellSize();
        float gridMaxY = gridMinY + settings.getGridRows() * settings.getCellSize();
        
        float camX = playerPos.x;
        float camY = playerPos.y;
        
        float halfW = viewWidth / 2.0f;
        float halfH = viewHeight / 2.0f;
        
        // Clamp to edges
        if (camX - halfW < gridMinX) camX = gridMinX + halfW;
        if (camX + halfW > gridMaxX) camX = gridMaxX - halfW;
        if (camY - halfH < gridMinY) camY = gridMinY + halfH;
        if (camY + halfH > gridMaxY) camY = gridMaxY - halfH;
        
        // Center if grid is smaller than view
        if (viewWidth > (gridMaxX - gridMinX)) camX = gridMinX + (gridMaxX - gridMinX) / 2.0f;
        if (viewHeight > (gridMaxY - gridMinY)) camY = gridMinY + (gridMaxY - gridMinY) / 2.0f;
        
        cameraView.setCenter({camX, camY});
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
    window.setView(cameraView);

    if (activeChamber) activeChamber->draw(window);
    
    // Draw player
    if (player) player->draw(window);
    
    // Restore UI View for HUD
    window.setView(uiView);
    GameState::draw(window);
}



void GameplayState::handleEvents(sf::Event& event) {
    // 1. Let the player handle its own single-press inputs
    player->handleInput(event);

    SettingManager& settings = SettingManager::getInstance();

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            if (activeChamber && player) {
                sf::Vector2f dir;
                if (player->getActiveFormType() == FormType::VOIDCASTER) {
                    sf::RenderWindow& window = Game::getInstance().getWindow();
                    sf::Vector2f mouseWorldPos = window.mapPixelToCoords({mouseEvent->position.x, mouseEvent->position.y}, cameraView);
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
            // Negative delta means scrolling down (zoom out), positive means scrolling up (zoom in)
            // But we want smaller currentZoom = zoom in, so subtract delta
            currentZoom -= scrollEvent->delta * 0.05f; 
            
            // Calculate max zoom out based on grid boundaries so it never shows out of bounds
            float gridWidth = settings.getGridCols() * settings.getCellSize();
            float gridHeight = settings.getGridRows() * settings.getCellSize();
            float maxZoomOut = std::min(gridWidth / settings.getWindowWidth(), gridHeight / settings.getWindowHeight());
            
            // Enforce constraints (min zoom = 0.5f, max zoom = bounded by map)
            if (currentZoom < 0.5f) currentZoom = 0.5f;
            if (currentZoom > maxZoomOut) currentZoom = maxZoomOut;
            
            cameraView.setSize({settings.getWindowWidth() * currentZoom, settings.getWindowHeight() * currentZoom});
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