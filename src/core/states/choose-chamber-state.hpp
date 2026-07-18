#ifndef CHOOSE_CHAMBER_STATE_HPP
#define CHOOSE_CHAMBER_STATE_HPP

#include "../game-state.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

class ChooseChamberState : public GameState {
public:
    ChooseChamberState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* buttonBox;

    UI::Button* testChamberBtn;
    UI::Button* protectChamberBtn;
    UI::Button* preventChamberBtn;
    UI::Button* backBtn;
};

#endif // CHOOSE_CHAMBER_STATE_HPP
