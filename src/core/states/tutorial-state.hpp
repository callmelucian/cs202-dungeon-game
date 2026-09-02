#ifndef TUTORIAL_STATE_HPP
#define TUTORIAL_STATE_HPP

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../../entities/player.hpp"
#include "../../entities/playable-character.hpp"
#include "../../chambers/tutorial-chamber.hpp"
#include "../../utils/camera.hpp"
#include "../../ui/widgets/hud.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/base/text.hpp"
#include <memory>

enum class TutorialTransitionState {
    OBJECTIVE_DISPLAY,
    FADING_IN,
    TITLE_DISPLAY,
    ZOOMING_IN,
    PLAYING,
    FADING_OUT,
    COMPLETED
};

// TutorialState: Dedicated game state for the tutorial mode.
// Orchestrates the 8-island training grounds, UI HUD, camera tracking,
// form explanation modal dialog, and seamless completion flow back to Main Menu.
class TutorialState : public GameState, public ChamberObserver {
public:
    explicit TutorialState(StateManager& manager);
    virtual ~TutorialState() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;

    void onChamberCompleted() override;
    void onChamberFailed() override;

private:
    std::unique_ptr<PlayableCharacter> playableChar;
    std::unique_ptr<Player> player;
    std::unique_ptr<TutorialChamber> activeChamber;

    Camera camera;
    TutorialTransitionState transitionState = TutorialTransitionState::OBJECTIVE_DISPLAY;
    enum class ObjectivePhase { FADING_IN, DISPLAY, FADING_OUT };
    ObjectivePhase objectivePhase = ObjectivePhase::FADING_IN;
    float objectiveTimer = 0.0f;
    float objectiveAlpha = 0.0f;
    const float OBJECTIVE_FADE_DURATION = 0.8f;

    float introTimer = 0.0f;
    float fadeTimer = 0.0f;
    float fadeAlpha = 255.0f;
    const float FADE_DURATION = 1.5f;
    mutable sf::RectangleShape fadeOverlay;

    // UI elements
    UI::HUD* hud = nullptr;
    UI::Container* titleContainer = nullptr;
    UI::Text* chamberTitleText = nullptr;

    // Objective modal dialog
    std::unique_ptr<UI::Container> objectiveModal;

    // Island 4 Modal Form Guide Dialog
    std::unique_ptr<UI::Container> modalRoot;
    UI::VerticalBox* modalCard = nullptr;
    UI::Button* modalCloseButton = nullptr;
    bool isModalOpen = false;

    void setupUI();
    void setupObjectiveModal();
    void proceedFromObjective();
    void setupModalDialog();
    void initPlayerPosition();
    void startTutorialIntro();
    void showFormGuideModal();
};

#endif // TUTORIAL_STATE_HPP
