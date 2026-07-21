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
#include "../../chambers/chamber.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../economy/echo.hpp"
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
class GameplayState : public GameState, public EchoObserver, public ChamberObserver {
public:
    GameplayState(StateManager& manager, ChamberSelectionType type);
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;
    void onEchoPowerChanged(float power) override;
    void onChamberCompleted() override;
    void onChamberFailed() override;
private:
    UI::Container* overlays;
    UI::Text* titleText;
    UI::HorizontalBox* buttonBox;
    UI::Button* pauseButton;
    UI::Button* quitButton;

    UI::VerticalBox* hudBox;
    UI::Text* formText;
    UI::Text* hpText;
    UI::Text* momentumText;
    UI::Text* cooldownText;
    UI::Text* echoPowerText;

    std::unique_ptr<PlayableCharacter> playableChar;
    std::unique_ptr<Player> player;
    std::unique_ptr<Chamber> activeChamber;

    sf::View cameraView;
    float currentZoom;
};

#endif // GAME_PLAY_STATE
