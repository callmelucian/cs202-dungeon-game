#include "debug-state.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iomanip>
#include <sstream>

DebugState::DebugState(StateManager& manager, Player& player)
    : GameState(manager), playerRef(player) {
    drawBackground = false; // Overlay on top of active gameplay view

    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(30.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    titleText = layoutBox->createChild<UI::Text>("header", 36)
        ->setString("Debug Controls");

    widgetBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(20.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    widgetBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 260.f,
        .fixedHeight = 50.f
    });

    float maxHp = playerRef.getEffectiveStats().maxHp;
    float currentHp = playerRef.getHp();

    std::ostringstream hpSs;
    hpSs << "Player HP: " << static_cast<int>(currentHp) << " / " << static_cast<int>(maxHp);

    hpLabel = widgetBox->createChild<UI::Text>("regular", 20)
        ->setString(hpSs.str())
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    hpSlider = widgetBox->createChild<UI::Slider>(0.0f, maxHp, currentHp)
        ->setOnValueChanged([this, maxHp](float val) {
            playerRef.setHp(val);
            Game::getInstance().getRunState().playerHP = val;
            std::ostringstream ss;
            ss << "Player HP: " << static_cast<int>(val) << " / " << static_cast<int>(maxHp);
            hpLabel->setString(ss.str());
        });

    FormType activeForm = playerRef.getActiveFormType();
    float currentMomentum = playerRef.getMomentum(activeForm);

    std::ostringstream momSs;
    momSs << "Active Form Momentum: " << static_cast<int>(currentMomentum) << " / 100";

    momentumLabel = widgetBox->createChild<UI::Text>("regular", 20)
        ->setString(momSs.str())
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    momentumSlider = widgetBox->createChild<UI::Slider>(0.0f, 100.0f, currentMomentum)
        ->setOnValueChanged([this, activeForm](float val) {
            playerRef.setMomentum(val, activeForm);
            std::ostringstream ss;
            ss << "Active Form Momentum: " << static_cast<int>(val) << " / 100";
            momentumLabel->setString(ss.str());
        });

    backButton = widgetBox->createChild<UI::Button>("Back to Game", "regular", 25.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}
