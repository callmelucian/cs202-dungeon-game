#include "game-over-state.hpp"
#include "main-menu-state.hpp"
#include "game-play-state.hpp"
#include "../game.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include <string>
#include <vector>
#include <algorithm>

GameOverState::GameOverState(StateManager& manager, std::optional<EndingType> endingType)
    : GameState(manager), endingType(endingType), fadeInTimer(0.0f), fadeInDuration(1.2f) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    if (!endingType.has_value()) {
        // Death / Chamber Failure screen
        titleText = layoutBox->createChild<UI::Text>("header", 36)
            ->setString("YOU DIED");
        titleText->setFillColor(sf::Color(220, 40, 40));

        endingText = layoutBox->createChild<UI::Text>("italic", 22)
            ->setString("Serin has fallen. The Vault remains unbroken.");
        
        auto buttonBox = layoutBox->createChild<UI::HorizontalBox>()
            ->setModeX(UI::SizeMode::Contained)
            ->setModeY(UI::SizeMode::Contained)
            ->setSpacing(25.f);

        menuButton = buttonBox->createChild<UI::Button>("Retry Chamber", "regular", 22)
            ->setModeX(UI::SizeMode::Fixed)
            ->setModeY(UI::SizeMode::Fixed)
            ->setFixedWidth(220.f)
            ->setFixedHeight(50.f)
            ->setOnClick([this]() {
                // Restore HP from the last save (HP recorded at the start of this chamber)
                RunState& runState = Game::getInstance().getRunState();
                SaveLoadManager::getInstance().loadGame(runState);
                stateManager.changeState(std::make_unique<GameplayState>(stateManager));
            });

        buttonBox->createChild<UI::Button>("Main Menu", "regular", 22)
            ->setModeX(UI::SizeMode::Fixed)
            ->setModeY(UI::SizeMode::Fixed)
            ->setFixedWidth(220.f)
            ->setFixedHeight(50.f)
            ->setOnClick([this]() {
                stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
            });
    } else {
        // Ending screen
        std::string titleStr;
        std::string subtitleStr;
        std::string narrativeStr;
        sf::Color themeColor;

        switch (endingType.value()) {
            case EndingType::ENDING_A_SHATTER:
                titleStr = "ENDING A: THE SHATTER";
                subtitleStr = "The Vault Collapses - Purity Restored";
                narrativeStr = "Every Echo has been preserved in its pure essence.\nWith all five relics united, Serin shatters Malachar's ancient seal\nand breaks the curse of the Ashen Vault forever.\nRadiant memories flood back to the world.";
                themeColor = sf::Color(255, 215, 0); // Gold
                break;
            case EndingType::ENDING_B_RETREAT:
                titleStr = "ENDING B: THE RETREAT";
                subtitleStr = "Ascended to Light - Fractured Truths";
                narrativeStr = "Though Malachar has been driven back, some Echoes were corrupted\nand lost to the abyss. Serin escapes the crumbling vault with\nonly fragments of what once was. The realm is saved for now,\nyet lingering shadows wait in the deep.";
                themeColor = sf::Color(140, 175, 210); // Somber Blue-Grey
                break;
            case EndingType::ENDING_C_WARNING:
                titleStr = "ENDING C: THE WARNING";
                subtitleStr = "Lost in the Dark - The Eternal Watch";
                narrativeStr = "The Vault's corruption runs too deep. With most Echoes stolen\nand consumed by darkness, Malachar's defeat is merely an illusion.\nSerin seals the gate from without, leaving a grim warning etched in stone:\nlet no living soul enter the Ashen Vault again.";
                themeColor = sf::Color(220, 60, 60); // Dark Red
                break;
        }

        titleText = layoutBox->createChild<UI::Text>("header", 30)
            ->setString(titleStr);
        titleText->setFillColor(themeColor);

        subtitleText = layoutBox->createChild<UI::Text>("bold", 18)
            ->setString(subtitleStr);
        subtitleText->setFillColor(sf::Color(210, 210, 210));

        endingText = layoutBox->createChild<UI::Text>("italic", 16)
            ->setString(narrativeStr);
        endingText->setFillColor(sf::Color(230, 230, 230));

        // Echo Outcome Summary Table
        auto summaryBox = layoutBox->createChild<UI::VerticalBox>()
            ->setModeX(UI::SizeMode::Contained)
            ->setModeY(UI::SizeMode::Contained)
            ->setAlignmentX(UI::AlignmentX::Center)
            ->setSpacing(6.f);

        summaryBox->createChild<UI::Text>("header", 16)
            ->setString("--- ECHO STATUS SUMMARY ---")
            ->setFillColor(themeColor);

        const RunState& runState = Game::getInstance().getRunState();

        struct EchoMeta {
            EchoType type;
            std::string name;
            std::string chamber;
        };

        const std::vector<EchoMeta> echoList = {
            {EchoType::MARROW, "Marrow Echo", "L1 Ch.1"},
            {EchoType::HOLLOW_BELL, "Hollow Bell", "L1 Ch.2"},
            {EchoType::CLARITY_SHARD, "Clarity Shard", "L2 Ch.1"},
            {EchoType::RESONANCE_CORE, "Resonance Core", "L2 Ch.2"},
            {EchoType::OBSIDIAN_KEY, "Obsidian Key", "L3 Ch.2"}
        };

        for (const auto& item : echoList) {
            std::string statusStr = "[ UNCOLLECTED ]";
            sf::Color statusColor = sf::Color(160, 160, 160);

            auto it = runState.echoOutcomes.find(item.type);
            if (it != runState.echoOutcomes.end()) {
                if (it->second == EchoOutcome::COLLECTED) {
                    statusStr = "[ COLLECTED ]";
                    statusColor = sf::Color(80, 220, 100);
                } else if (it->second == EchoOutcome::STOLEN) {
                    statusStr = "[ STOLEN ]";
                    statusColor = sf::Color(220, 70, 70);
                }
            }

            std::string line = item.name + " (" + item.chamber + "): " + statusStr;
            summaryBox->createChild<UI::Text>("regular", 15)
                ->setString(line)
                ->setFillColor(statusColor);
        }

        menuButton = layoutBox->createChild<UI::Button>("Back to Main Menu", "regular", 20)
            ->setModeX(UI::SizeMode::Fixed)
            ->setModeY(UI::SizeMode::Fixed)
            ->setFixedWidth(280.f)
            ->setFixedHeight(45.f)
            ->setOnClick([this]() {
                stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
            });
    }
}

void GameOverState::update(float deltaTime) {
    GameState::update(deltaTime);
    if (fadeInTimer < fadeInDuration) {
        fadeInTimer = std::min(fadeInTimer + deltaTime, fadeInDuration);
    }
}

void GameOverState::draw(sf::RenderWindow& window) const {
    GameState::draw(window);
    
    if (fadeInTimer < fadeInDuration) {
        SettingManager& settings = SettingManager::getInstance();
        float alpha = 255.0f * std::max(0.0f, 1.0f - (fadeInTimer / fadeInDuration));
        fadeOverlay.setSize(sf::Vector2f(static_cast<float>(settings.getWindowWidth()), static_cast<float>(settings.getWindowHeight())));
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha)));
        window.draw(fadeOverlay);
    }
}
