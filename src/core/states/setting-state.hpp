#ifndef SETTING_STATE
#define SETTING_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/widgets/slider.hpp"
#include "../../ui/base/text.hpp"

// SettingState: Represents the settings/options state, allowing changes to volume and other settings.
class SettingState : public GameState {
public:
    SettingState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* buttonBox;
    UI::Text* musicLabel;
    UI::Slider* musicSlider;
    UI::Text* sfxLabel;
    UI::Slider* sfxSlider;
    UI::Button* backButton;
};

#endif // SETTING_STATE
