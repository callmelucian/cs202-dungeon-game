#ifndef GAME_OVER_STATE
#define GAME_OVER_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <memory>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

#include "../enums.hpp"

// GameOverState: Represents the state when the game ends, displaying the corresponding EndingType (A, B, or C) or Chamber Failure.
class GameOverState : public GameState {
public:
    GameOverState(StateManager& manager, std::optional<EndingType> endingType = std::nullopt);
    virtual ~GameOverState() = default;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    std::optional<EndingType> endingType;
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::Text* subtitleText;
    UI::Text* endingText;
    UI::Button* menuButton;

    float fadeInTimer;
    float fadeInDuration;
    mutable sf::RectangleShape fadeOverlay;
};

#endif // GAME_OVER_STATE
