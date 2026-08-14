#ifndef DEBUG_STATE_HPP
#define DEBUG_STATE_HPP

#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../../entities/player.hpp"
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/widgets/slider.hpp"
#include "../../ui/base/text.hpp"

// DebugState: Temporary state allowing real-time adjustment of Player HP and Momentum using UI sliders.
class DebugState : public GameState {
public:
    DebugState(StateManager& manager, Player& player);

private:
    Player& playerRef;
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* widgetBox;
    
    UI::Text* hpLabel;
    UI::Slider* hpSlider;
    
    UI::Text* momentumLabel;
    UI::Slider* momentumSlider;
    
    UI::Button* backButton;
};

#endif // DEBUG_STATE_HPP
