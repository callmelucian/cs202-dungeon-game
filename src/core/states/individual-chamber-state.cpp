#include "individual-chamber-state.hpp"
#include "game-play-state.hpp"
#include "../game.hpp"

IndividualChamberState::IndividualChamberState(StateManager& manager) : GameState(manager) {
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Main layout box
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(35.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title
    titleText = layoutBox->createChild<UI::Text>("header", 36)
        ->setString("Select Individual Chamber");

    // Horizontal container for Level columns
    columnsContainer = layoutBox->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    auto setupColumn = [](UI::VerticalBox* col) {
        col->setModeX(UI::SizeMode::Contained)
           ->setModeY(UI::SizeMode::Contained)
           ->setAlignmentX(UI::AlignmentX::Center)
           ->setSpacing(12.f)
           ->setDistribution(UI::Distribution::SpaceBetween);
        col->setChildDefaults({
            .modeX = UI::SizeMode::Fixed,
            .modeY = UI::SizeMode::Fixed,
            .fixedWidth = 270.f,
            .fixedHeight = 44.f,
        });
    };

    // Column 1: Level 1 — The Outer Wards
    auto* col1 = columnsContainer->createChild<UI::VerticalBox>();
    setupColumn(col1);
    col1->createChild<UI::Text>("regular", 20)->setString("Level 1: Outer Wards");
    col1->createChild<UI::Button>("1. Drowned Archive (Protect)", "regular", 16)
        ->setOnClick([this]() { startChamber(1, 1); });
    col1->createChild<UI::Button>("2. Bone Corridor (Prevent)", "regular", 16)
        ->setOnClick([this]() { startChamber(1, 3); });
    col1->createChild<UI::Button>("3. Collapsed Barracks (Gauntlet)", "regular", 16)
        ->setOnClick([this]() { startChamber(1, 5); });

    // Column 2: Level 2 — The Sunken Choir
    auto* col2 = columnsContainer->createChild<UI::VerticalBox>();
    setupColumn(col2);
    col2->createChild<UI::Text>("regular", 20)->setString("Level 2: Sunken Choir");
    col2->createChild<UI::Button>("1. Drowned Choir (Protect)", "regular", 16)
        ->setOnClick([this]() { startChamber(2, 1); });
    col2->createChild<UI::Button>("2. Choir Loft (Prevent)", "regular", 16)
        ->setOnClick([this]() { startChamber(2, 3); });
    col2->createChild<UI::Button>("3. Silent Nave (Gauntlet)", "regular", 16)
        ->setOnClick([this]() { startChamber(2, 5); });

    // Column 3: Level 3 — The Inner Sanctum
    auto* col3 = columnsContainer->createChild<UI::VerticalBox>();
    setupColumn(col3);
    col3->createChild<UI::Text>("regular", 20)->setString("Level 3: Inner Sanctum");
    col3->createChild<UI::Button>("1. Resonance Hall (Protect)", "regular", 16)
        ->setOnClick([this]() { startChamber(3, 1); });
    col3->createChild<UI::Button>("2. Mirror Vault (Prevent)", "regular", 16)
        ->setOnClick([this]() { startChamber(3, 3); });
    col3->createChild<UI::Button>("3. Hunger Pit (Gauntlet)", "regular", 16)
        ->setOnClick([this]() { startChamber(3, 5); });

    // Column 4: Special Chambers
    auto* col4 = columnsContainer->createChild<UI::VerticalBox>();
    setupColumn(col4);
    col4->createChild<UI::Text>("regular", 20)->setString("Special Chambers");
    col4->createChild<UI::Button>("Resting Sanctuary (Mid)", "regular", 16)
        ->setOnClick([this]() { startChamber(1, 2); });
    col4->createChild<UI::Button>("Boss: Malachar (Final)", "regular", 16)
        ->setOnClick([this]() { startChamber(4, 1); });

    // Back button
    auto* bottomBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center);
    bottomBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 180.f,
        .fixedHeight = 45.f,
    });

    backBtn = bottomBox->createChild<UI::Button>("Back", "regular", 22)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}

void IndividualChamberState::startChamber(int level, int chamberIndex) {
    RunState& runState = Game::getInstance().getRunState();
    runState = RunState{};
    runState.currentLevel = level;
    runState.currentChamber = chamberIndex;

    // If jumping straight to Boss Malachar, activate Foretell so mechanics are fully testable
    if (level >= 4) {
        runState.foretellActive = true;
    }

    stateManager.pushState(std::make_unique<GameplayState>(stateManager));
}
