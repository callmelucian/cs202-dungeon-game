#include "echo-log-state.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

EchoLogState::EchoLogState(StateManager& manager) : GameState(manager) {
    drawBackground = false;
    root->setAlignmentY(UI::AlignmentY::Middle);
    root->setAlignmentX(UI::AlignmentX::Center);

    const RunState& runState = Game::getInstance().getRunState();

    // Create a vertical layout box centered on screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(10.f)
        ->setPadding(20.f, 20.f, 30.f, 30.f)
        ->setColor(sf::Color(18, 14, 28, 240));

    // Title Text
    titleText = layoutBox->createChild<UI::Text>("header", 22)
        ->setString("ECHO RESONANCE LOG")
        ->setFillColor(sf::Color(255, 215, 80));

    subtitleText = layoutBox->createChild<UI::Text>("regular", 13)
        ->setString("Vault Echo Outcomes and Active Combat Modifiers")
        ->setFillColor(sf::Color(180, 180, 200));

    // Container for Echo cards
    cardsContainer = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(6.f)
        ->setPadding(4.f, 4.f, 4.f, 4.f);

    struct EchoMeta {
        EchoType type;
        std::string name;
        std::string chamber;
        std::string previewDesc;
        std::string collectedDesc;
        std::string intactDesc;
        std::string stolenDesc;
    };

    const std::vector<EchoMeta> echoes = {
        {
            EchoType::CLARITY_SHARD,
            "Clarity Shard",
            "Level 1 - Chamber 1 (Protect)",
            "Guards temporal flow. If collected, shortens collection times and foretells Malachar's attacks.",
            "+10% Faster Echo Collection. Foretell grants +0.6s warning on Malachar's attacks starting in Phase 2.",
            "+20% Faster Echo Collection. Foretell grants +0.6s warning on Malachar's attacks starting immediately in Phase 1!",
            "Standard collection speed. Malachar's attacks execute rapidly with no foretell warnings."
        },
        {
            EchoType::MARROW,
            "Marrow Echo",
            "Level 1 - Chamber 2 (Prevent)",
            "Pulsating bone essence. If stolen, Malachar gains continuous health regeneration during battle.",
            "Echo Secured. Malachar is denied all vitality regeneration.",
            "Echo Secured. Malachar is denied all vitality regeneration.",
            "Malachar regenerates +2% Max HP/sec (25 HP/s) continuously in Phases 2, 3, and 4!"
        },
        {
            EchoType::HOLLOW_BELL,
            "Hollow Bell",
            "Level 2 - Chamber 1 (Protect)",
            "Resonating void chime. If collected, lowers Special Ability 1 Momentum cost. If stolen, Malachar gains Reflect Ward.",
            "Special Ability 1 Momentum threshold reduced from 50 to 42.5 (15% reduction).",
            "Special Ability 1 Momentum threshold reduced from 50 to 35.0 (30% reduction)!",
            "Malachar gains a Reflect Ward in Phase 1 that reflects 20% damage every 8 seconds."
        },
        {
            EchoType::RESONANCE_CORE,
            "Resonance Core",
            "Level 3 - Chamber 1 (Protect)",
            "Violent kinetic heart. If collected, deals massive burst damage to Malachar upon phase transitions.",
            "Deals an 8% Max HP transition burst of damage to Malachar at every phase change.",
            "Deals a DOUBLE transition burst (16% total HP damage, ~1s apart) to Malachar at every phase change!",
            "No phase-transition damage dealt to Malachar."
        },
        {
            EchoType::OBSIDIAN_KEY,
            "Obsidian Key",
            "Level 3 - Chamber 2 (Prevent)",
            "Fractured dimensional stone. If stolen, Malachar teleports unpredictably across the arena.",
            "Echo Secured. Malachar is denied the Phase-Shift Blink ability.",
            "Echo Secured. Malachar is denied the Phase-Shift Blink ability.",
            "Malachar gains Phase-Shift Blink, teleporting across the arena every 6-9 seconds in Phases 2 & 3!"
        }
    };

    for (const auto& meta : echoes) {
        auto card = cardsContainer->createChild<UI::VerticalBox>()
            ->setModeX(UI::SizeMode::Fixed)
            ->setModeY(UI::SizeMode::Contained)
            ->setFixedWidth(680.f)
            ->setSpacing(3.f)
            ->setPadding(6.f, 6.f, 10.f, 10.f)
            ->setColor(sf::Color(30, 24, 45, 200));

        EchoOutcome outcome = EchoOutcome::UNCOLLECTED;
        auto outcomeIt = runState.echoOutcomes.find(meta.type);
        if (outcomeIt != runState.echoOutcomes.end()) {
            outcome = outcomeIt->second;
        }

        float power = 0.0f;
        auto powerIt = runState.echoPowers.find(meta.type);
        if (powerIt != runState.echoPowers.end()) {
            power = powerIt->second;
        }

        std::string statusTag = "[ UNRESOLVED ]";
        sf::Color statusColor = sf::Color(150, 150, 160);
        std::string effectText = meta.previewDesc;
        sf::Color effectColor = sf::Color(190, 190, 200);

        if (outcome == EchoOutcome::COLLECTED) {
            std::ostringstream ss;
            if (power >= 90.0f) {
                ss << "[ COLLECTED (" << static_cast<int>(std::round(power)) << "% - FULLY INTACT) ]";
                statusTag = ss.str();
                statusColor = sf::Color(80, 240, 140);
                effectText = meta.intactDesc;
                effectColor = sf::Color(120, 255, 170);
            } else {
                ss << "[ COLLECTED (" << static_cast<int>(std::round(power)) << "%) ]";
                statusTag = ss.str();
                statusColor = sf::Color(100, 220, 200);
                effectText = meta.collectedDesc;
                effectColor = sf::Color(160, 240, 220);
            }
        } else if (outcome == EchoOutcome::STOLEN) {
            statusTag = "[ STOLEN BY MALACHAR ]";
            statusColor = sf::Color(255, 75, 75);
            effectText = meta.stolenDesc;
            effectColor = sf::Color(255, 150, 150);
        }

        auto headerRow = card->createChild<UI::HorizontalBox>()
            ->setModeX(UI::SizeMode::Expanded)
            ->setModeY(UI::SizeMode::Contained)
            ->setDistribution(UI::Distribution::SpaceBetween);

        std::string headerStr = meta.name + " (" + meta.chamber + ")";
        headerRow->createChild<UI::Text>("header", 14)
            ->setString(headerStr)
            ->setFillColor(sf::Color(255, 230, 130));

        headerRow->createChild<UI::Text>("regular", 13)
            ->setString(statusTag)
            ->setFillColor(statusColor);

        card->createChild<UI::Text>("regular", 12)
            ->setString(effectText)
            ->setFillColor(effectColor);
    }

    // Close button
    closeButton = layoutBox->createChild<UI::Button>("Close", "regular", 20.f)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(160.f)
        ->setFixedHeight(40.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}

void EchoLogState::draw(sf::RenderWindow& window) const {
    // Dim background gameplay view
    sf::RectangleShape backdrop(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
    backdrop.setFillColor(sf::Color(8, 6, 14, 215));
    window.draw(backdrop);

    GameState::draw(window);
}

void EchoLogState::handleEvents(sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape ||
            keyPressed->scancode == sf::Keyboard::Scancode::E) {
            stateManager.popState();
            return;
        }
    }
    GameState::handleEvents(event);
}
