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

    // Create HUD at top left
    hudBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Left)
        ->setAlignmentY(UI::AlignmentY::Top)
        ->setPadding(20.f, 20.f, 20.f, 20.f)
        ->setSpacing(10.f);

    formText = hudBox->createChild<UI::Text>("regular", 24)->setString("Form: Wraithblade");
    hpText = hudBox->createChild<UI::Text>("regular", 24)->setString("HP: 100/100");
    momentumText = hudBox->createChild<UI::Text>("regular", 24)->setString("Momentum: 0");
    cooldownText = hudBox->createChild<UI::Text>("regular", 24)->setString("Cooldown: Ready");

    playableChar = std::make_unique<Serin>();
    player = std::make_unique<Player>(*playableChar);
    player->setPosition({300.f, 300.f}); // Spawn at coordinate (300, 300)
    
    // Temporarily set inMidChamber = true (which means disabling the switch cooldown)
    player->setSwitchCooldownEnabled(false);

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
    SettingManager& settings = SettingManager::getInstance();
    sf::Vector2f dir(0.f, 0.f);

    // Check real-time keyboard inputs
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveUp"))) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveDown"))) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveLeft"))) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveRight"))) dir.x += 1.f;

    // Normalize diagonal movement speed
    if (dir.x != 0.f || dir.y != 0.f) {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir.x /= length;
        dir.y /= length;
    }

    // Convert speed stat (e.g. 7.0f) to screen pixel units using a multiplier
    float screenScale = 60.f;
    sf::Vector2f vel = dir * player->getSpeed() * screenScale;
    player->setVelocity(vel);

    // Update character cooldowns & effects
    player->update(deltaTime);

    // Resolve movement: integrates velocity then depenetrates per axis
    CollisionSolver::resolveAABB(*player, obstacles, deltaTime);

    // Update HUD text
    std::string formStr = "Unknown";
    FormType type = player->getActiveFormType();
    if (type == FormType::WRAITHBLADE) formStr = "Wraithblade";
    else if (type == FormType::VOIDCASTER) formStr = "Voidcaster";
    else if (type == FormType::IRONSHELL) formStr = "Ironshell";
    formText->setString("Form: " + formStr);
    
    Stats currentStats = player->getEffectiveStats();
    hpText->setString("HP: " + std::to_string((int)currentStats.hp) + "/" + std::to_string((int)currentStats.maxHp));
    momentumText->setString("Momentum: " + std::to_string((int)player->getMomentum(type)));
    
    float cd = player->getSwitchCooldownTimer();
    if (cd > 0.0f) {
        cooldownText->setString("Cooldown: " + std::to_string(cd).substr(0, 4) + "s");
    } else {
        cooldownText->setString("Cooldown: Ready (or MidChamber)");
    }

    // Base class updates UI layouts
    GameState::update(deltaTime);
}


void GameplayState::draw(sf::RenderWindow& window) const {
    // Draw red obstacles
    for (const auto& obs : obstacles) {
        sf::RectangleShape shape({obs.size.x, obs.size.y});
        shape.setPosition(obs.position);
        shape.setFillColor(sf::Color(180, 50, 50));
        window.draw(shape);
    }

    // Draw green player box
    sf::FloatRect bounds = player->getBounds();
    sf::RectangleShape pShape({bounds.size.x, bounds.size.y});
    pShape.setPosition(bounds.position);
    pShape.setFillColor(sf::Color(50, 180, 50));
    window.draw(pShape);

    // Draw UI components on top
    GameState::draw(window);
}

void GameplayState::handleEvents(sf::Event& event) {
    SettingManager& settings = SettingManager::getInstance();

    // Listen for single KeyPressed events
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == settings.getKeyBinding("SwitchForm1")) {
            player->switchForm(FormType::WRAITHBLADE);
            std::cout << "Switched to Wraithblade! Speed: " << player->getSpeed() << std::endl;
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm2")) {
            player->switchForm(FormType::VOIDCASTER);
            std::cout << "Switched to Voidcaster! Speed: " << player->getSpeed() << std::endl;
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm3")) {
            player->switchForm(FormType::IRONSHELL);
            std::cout << "Switched to Ironshell! Speed: " << player->getSpeed() << std::endl;
        }
    }

    // Pass down to UI components
    GameState::handleEvents(event);
}