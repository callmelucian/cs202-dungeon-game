#ifndef PAUSE_STATE
#define PAUSE_STATE

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

// PauseState: Represents the overlay menu shown when the game is paused, allowing the player to resume or quit.
class PauseState : public GameState {
public:
    PauseState(StateManager& manager);
    virtual ~PauseState() override = default;
    bool isTransparent() const override { return true; }
    void handleEvents(sf::Event& event) override;

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* buttonBox;
    UI::Button* resumeButton;
    UI::Button* echoButton;
    UI::Button* saveButton;
    UI::Button* settingButton;
    UI::Button* menuButton;
};

#endif // PAUSE_STATE
