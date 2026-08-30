#ifndef ECHO_LOG_STATE_HPP
#define ECHO_LOG_STATE_HPP

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>
#include <vector>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

/**
 * @brief EchoLogState represents the interactive modal menu displaying
 * all 5 Vault Echoes, their chamber origins, current status, power percentage,
 * and active gameplay modifiers for both Serin and Boss Malachar.
 */
class EchoLogState : public GameState {
public:
    EchoLogState(StateManager& manager);
    virtual ~EchoLogState() = default;

    bool isTransparent() const override { return true; }
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;

private:
    UI::VerticalBox* layoutBox = nullptr;
    UI::Text* titleText = nullptr;
    UI::Text* subtitleText = nullptr;
    UI::VerticalBox* cardsContainer = nullptr;
    UI::Button* closeButton = nullptr;
};

#endif // ECHO_LOG_STATE_HPP
