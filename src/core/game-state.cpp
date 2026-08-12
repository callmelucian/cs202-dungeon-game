#include "state-manager.hpp"
#include "game-state.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/asset-manager.hpp"

GameState::GameState (StateManager& manager)
    : stateManager(manager),
      root(std::make_unique<UI::Container>()) {
    // get singleton instances
    SettingManager &settings = SettingManager::getInstance();

    // set root container configuration
    root->setModeX(UI::SizeMode::Fixed);
    root->setFixedWidth(settings.getWindowWidth());
    root->setModeY(UI::SizeMode::Fixed);
    root->setFixedHeight(settings.getWindowHeight());
    
    drawBackground = true;
    AssetManager& assets = AssetManager::getInstance();
    try {
        backgroundSprite.emplace(assets.getTexture("main-bg"));
    } catch (...) {
        drawBackground = false;
    }
    if (drawBackground && backgroundSprite.has_value()) {
        backgroundSprite->setColor(sf::Color(255, 255, 255, 230)); // 10% transparent
    }
}

GameState::~GameState() {
}

void GameState::update (float deltaTime) {
    SettingManager &settings = SettingManager::getInstance();
    root->computeSize(sf::Vector2f(settings.getWindowWidth(), settings.getWindowHeight()));
    root->setPosition(sf::Vector2f(0.f, 0.f));
    root->update(deltaTime);
    
    if (drawBackground && backgroundSprite.has_value()) {
        float scaleX = settings.getWindowWidth() / backgroundSprite->getLocalBounds().size.x;
        float scaleY = settings.getWindowHeight() / backgroundSprite->getLocalBounds().size.y;
        backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
    }
}

void GameState::draw (sf::RenderWindow &window) const {
    if (drawBackground && backgroundSprite.has_value()) {
        window.draw(*backgroundSprite);
    }
    root->draw(window);
}

void GameState::handleEvents (sf::Event &event) {
    root->handleEvent(event);
}