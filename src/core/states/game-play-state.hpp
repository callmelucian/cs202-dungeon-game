#ifndef GAME_PLAY_STATE
#define GAME_PLAY_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../entities/player.hpp"
#include "../../entities/playable-character.hpp"
#include "../../utils/collision-solver.hpp"
#include <iostream>
#include <vector>
#include <memory>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

// GameplayState: Manages the active game session, processing entity updates, level progression, and combat logic.
class GameplayState : public GameState {
public:
    GameplayState(StateManager& manager);
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;
private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* buttonBox;
    UI::Button* pauseButton;
    UI::Button* quitButton;

    UI::VerticalBox* hudBox;
    UI::Text* formText;
    UI::Text* hpText;
    UI::Text* momentumText;
    UI::Text* cooldownText;

    std::unique_ptr<PlayableCharacter> playableChar;
    std::unique_ptr<Player> player;
    std::vector<sf::FloatRect> obstacles;

    void setupTestRoom();
};

#endif // GAME_PLAY_STATE
