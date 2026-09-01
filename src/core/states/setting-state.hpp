#ifndef SETTING_STATE
#define SETTING_STATE

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
#include "../../ui/widgets/slider.hpp"
#include "../../ui/base/text.hpp"

#include <unordered_map>
#include <string>

// SettingState: Represents the settings/options state, allowing changes to volume and keybindings.
class SettingState : public GameState {
public:
    SettingState(StateManager& manager);
    bool isTransparent() const override { return true; }
    void handleEvents(sf::Event& event) override;

private:
    void refreshKeyBindingLabels();

    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::HorizontalBox* columnsBox;
    UI::VerticalBox* audioBox;
    UI::VerticalBox* keybindBox;

    UI::Text* musicLabel;
    UI::Slider* musicSlider;
    UI::Text* sfxLabel;
    UI::Slider* sfxSlider;
    UI::Button* backButton;

    std::string pendingRebindAction;
    std::unordered_map<std::string, UI::Button*> keyButtons;
};

#endif // SETTING_STATE
