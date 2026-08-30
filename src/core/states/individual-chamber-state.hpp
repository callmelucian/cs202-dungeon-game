#ifndef INDIVIDUAL_CHAMBER_STATE_HPP
#define INDIVIDUAL_CHAMBER_STATE_HPP

#include "../game-state.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

class IndividualChamberState : public GameState {
public:
    IndividualChamberState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::HorizontalBox* columnsContainer;
    UI::Button* backBtn;

    void startChamber(int level, int chamberIndex);
};

#endif // INDIVIDUAL_CHAMBER_STATE_HPP
