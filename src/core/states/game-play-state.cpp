#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "game-over-state.hpp"
#include <cmath>

GameplayState::GameplayState(StateManager& manager) : GameState(manager) {
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

    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);
    player->setPosition({300.f, 300.f}); // Spawn at coordinate (300, 300)

    activeChamber = std::make_unique<BaseChamber>(*player);

    setupTestRoom();
}

void GameplayState::update(float deltaTime) {
    // 1. Update player logic (including real-time WASD movement)
    player->update(deltaTime);

    // 2. Resolve movement collisions
    // NOTE: CollisionSolver::resolveAABB applies velocity to position internally.
    // Do NOT manually integrate position (pos += vel * dt) in Character::update() 
    // or Player::update() to prevent double-movement bugs!
    CollisionSolver::resolveAABB(*player, obstacles, deltaTime);

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

    // 4. Base class updates UI layouts
    GameState::update(deltaTime);
}

void GameplayState::draw(sf::RenderWindow& window) const {
    // Draw background
    window.clear(sf::Color(20, 20, 25));

    // Draw obstacles
    for (const auto& obs : obstacles) {
        sf::RectangleShape shape({obs.size.x, obs.size.y});
        shape.setPosition(obs.position);
        shape.setFillColor(sf::Color(180, 50, 50));
        window.draw(shape);
    }

    // Draw player
    if (player) player->draw(window);
    
    if (activeChamber) activeChamber->draw(window);
    
    GameState::draw(window);
}

void GameplayState::setupTestRoom() {
    // Setup test environment barriers (Blocks & 1800x900 Screen Borders)
    obstacles = {
        // Inner obstacles
        sf::FloatRect({150.f, 150.f}, {100.f, 100.f}),
        sf::FloatRect({600.f, 200.f}, {200.f, 80.f}),
        // Room boundaries
        sf::FloatRect({0.f, 0.f}, {1800.f, 10.f}),   // Top
        sf::FloatRect({0.f, 890.f}, {1800.f, 10.f}), // Bottom
        sf::FloatRect({0.f, 0.f}, {10.f, 900.f}),    // Left
        sf::FloatRect({1790.f, 0.f}, {10.f, 900.f})  // Right
    };
}

void GameplayState::handleEvents(sf::Event& event) {
    // 1. Let the player handle its own single-press inputs
    player->handleInput(event);

    SettingManager& settings = SettingManager::getInstance();

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == settings.getKeyBinding("Attack")) {
            if (activeChamber && player) {
                // Determine attack direction based on facing
                sf::Vector2f dir = player->getVelocity();
                if (dir.x == 0 && dir.y == 0) {
                    dir = sf::Vector2f(player->getIsFacingRight() ? 1.0f : -1.0f, 0.0f);
                } else {
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir /= len;
                }
                player->attack(dir, *activeChamber);
            }
        }
    }

    // 2. Pass down to UI components
    GameState::handleEvents(event);
}