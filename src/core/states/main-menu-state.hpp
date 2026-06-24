#ifndef MAIN_MENU_STATE
#define MAIN_MENU_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

class MainMenuState : public GameState {
public:
    MainMenuState(StateManager& manager);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;

private:
};

#endif // MAIN_MENU_STATE