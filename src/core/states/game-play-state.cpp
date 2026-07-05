#include "game-play-state.hpp"
#include "pause-state.hpp"
#include "game-over-state.hpp"

GameplayState::GameplayState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(50.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (bold, size 54, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("bold", 54)
        ->setString("Gameplay State");

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

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

    // Add a couple of test wall obstacles
    obstacles.push_back(sf::FloatRect({150.f, 150.f}, {100.f, 100.f}));
    obstacles.push_back(sf::FloatRect({600.f, 200.f}, {200.f, 80.f}));
    // Add screen boundaries (Borders)
    // obstacles.push_back(sf::FloatRect({0.f, 0.f}, {1800.f, 10.f}));   // Top
    // obstacles.push_back(sf::FloatRect({0.f, 890.f}, {1800.f, 10.f})); // Bottom
    // obstacles.push_back(sf::FloatRect({0.f, 0.f}, {10.f, 900.f}));    // Left
    // obstacles.push_back(sf::FloatRect({1790.f, 0.f}, {10.f, 900.f}));  // Right
}

void GameplayState::update(float deltaTime) {
    // 1. Update player logic (including real-time WASD movement)
    player->update(deltaTime);

    // 2. Resolve movement collisions
    CollisionSolver::resolveAABB(*player, obstacles, deltaTime);

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
    player->draw(window);

    // Draw UI components on top
    GameState::draw(window);
}

void GameplayState::handleEvents(sf::Event& event) {
    // 1. Let the player handle its own single-press inputs
    player->handleInput(event);

    // 2. Pass down to UI components
    GameState::handleEvents(event);
}