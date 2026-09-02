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
#include "../../utils/camera.hpp"
#include "../../chambers/chamber.hpp"
#include "../../chambers/chamber-factory.hpp"
#include "../../economy/echo.hpp"
#include <vector>
#include <memory>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"
#include "../../ui/widgets/player-health-bar.hpp"
#include "../../ui/widgets/enemy-health-bar.hpp"
#include "../../ui/widgets/hud.hpp"
#include "../../ui/widgets/boss-health-bar.hpp"


enum class ChamberTransitionState {
    OBJECTIVE_DISPLAY,
    FADING_IN,
    TITLE_DISPLAY,
    ZOOMING_IN,
    PLAYING,
    FADING_OUT,
    COMPLETED
};

// GameplayState: Manages the active game session, processing entity updates, level progression, and combat logic.
class GameplayState : public GameState, public EchoObserver, public ChamberObserver {
public:
    GameplayState(StateManager& manager, bool isIndividualMode = false); // Campaign or Individual mode
    GameplayState(StateManager& manager, ChamberSelectionType type); // Debug mode
    virtual ~GameplayState() override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;
    void onEchoPowerChanged(float power) override;
    void onChamberCompleted() override;
    void onChamberFailed() override;
private:
    // UI::Container* overlays;
    UI::HorizontalBox* buttonBox;
    UI::Container* buttonBoxWrapper;
    
    UI::Button* pauseButton;
    UI::Button* echoesButton;
    UI::Button* debugButton;
    UI::Button* quitButton;

    UI::Container* titleContainer = nullptr;
    UI::Text* chamberTitleText = nullptr;

    // Objective modal dialog
    std::unique_ptr<UI::Container> objectiveModal;

    UI::HUD* hud;
    UI::BossHealthBar* bossHealthBar = nullptr;

    std::unique_ptr<PlayableCharacter> playableChar;
    std::unique_ptr<Player> player;
    std::unique_ptr<Chamber> activeChamber;
    bool isDebugMode = false;
    bool isIndividualMode = false;

    Camera camera;
    ChamberTransitionState transitionState = ChamberTransitionState::OBJECTIVE_DISPLAY;
    enum class ObjectivePhase { FADING_IN, DISPLAY, FADING_OUT };
    ObjectivePhase objectivePhase = ObjectivePhase::FADING_IN;
    float objectiveTimer = 0.0f;
    float objectiveAlpha = 0.0f;
    const float OBJECTIVE_FADE_DURATION = 0.8f;

    float introTimer = 0.0f;
    float fadeTimer = 0.0f;
    float fadeAlpha = 255.0f;
    const float FADE_DURATION = 2.0f;
    mutable sf::RectangleShape fadeOverlay;

    float chamberElapsedTime = 0.0f;
    std::string currentChamberTitle = "";
    
    void setupUI();
    void setupObjectiveModal(const std::string& titleStr, const std::string& objectiveStr);
    void proceedFromObjective();
    void initPlayerPosition();
    void startChamberIntro(const std::string& titleStr, const std::string& objectiveStr);
};

#endif // GAME_PLAY_STATE
