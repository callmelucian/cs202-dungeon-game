#include "echo-log-state.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

EchoLogState::EchoLogState(StateManager& manager) : GameState(manager) {
    drawBackground = false;
    root->setAlignmentY(UI::AlignmentY::Middle);
    root->setAlignmentX(UI::AlignmentX::Center);

    const RunState& runState = Game::getInstance().getRunState();

    // Create a spacious, beautiful vertical layout box centered on screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(16.f)
        ->setPadding(26.f, 26.f, 40.f, 40.f)
        ->setColor(sf::Color(14, 10, 24, 248));

    // Header Title Section
    auto titleHeaderBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(8.f);

    titleText = titleHeaderBox->createChild<UI::Text>("header", 26)
        ->setString("ECHO RESONANCE LOG")
        ->setFillColor(sf::Color(255, 220, 85));

    subtitleText = titleHeaderBox->createChild<UI::Text>("italic", 16)
        ->setString("Vault Echo Outcomes and Active Combat Modifiers")
        ->setFillColor(sf::Color(195, 195, 220));

    // Container for Echo cards with generous spacing
    cardsContainer = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(12.f)
        ->setPadding(6.f, 6.f, 6.f, 6.f);

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
            "CLARITY SHARD",
            "Level 1 - Chamber 1 [Protect]",
            "Guards the temporal flow within the Vault.\nIf secured: Shortens collection times and foretells Malachar's incoming attacks.",
            "+10% Faster Echo Collection speed.\nForetell Warning: Serin receives a 0.6s advance warning on Malachar's attacks starting in Phase 2.",
            "+20% Faster Echo Collection speed (Fully Intact).\nForetell Warning: Serin receives a 0.6s advance warning on Malachar's attacks immediately in Phase 1!",
            "Malachar's Distortion:\nMalachar strikes with blinding swiftness. Serin receives no attack foretell warnings."
        },
        {
            EchoType::MARROW,
            "MARROW ECHO",
            "Level 1 - Chamber 2 [Prevent]",
            "Pulsating bone essence of vital endurance.\nIf stolen: Malachar continuously regenerates health throughout combat.",
            "Echo Secured:\nMalachar is permanently stripped of all vitality regeneration across all phases.",
            "Echo Secured (Fully Intact):\nMalachar is permanently stripped of all vitality regeneration across all phases.",
            "Malachar's Vitality:\nMalachar continuously regenerates +2% Max HP/sec (25 HP/s) during Phases 2, 3, and 4!"
        },
        {
            EchoType::HOLLOW_BELL,
            "HOLLOW BELL",
            "Level 2 - Chamber 1 [Protect]",
            "Resonating chime of void resonance.\nIf secured: Reduces Special Ability 1 Momentum cost. If stolen: Malachar activates Reflect Ward.",
            "Special Ability 1 Momentum cost reduced from 50 to 42.5 (15% reduction).\nAllows Serin to unleash special attacks with less Momentum.",
            "Special Ability 1 Momentum cost reduced from 50 to 35.0 (30% reduction)!\nAllows Serin to unleash special attacks at a rapid pace.",
            "Malachar's Ward:\nMalachar gains a Reflect Ward in Phase 1 that reflects 20% of damage received back to Serin every 8s."
        },
        {
            EchoType::RESONANCE_CORE,
            "RESONANCE CORE",
            "Level 3 - Chamber 1 [Protect]",
            "Violent kinetic heart of explosive resonance.\nIf secured: Deals massive kinetic burst damage to Malachar upon phase transitions.",
            "Phase Detonation:\nDeals an 8% Max HP transition burst of damage to Malachar whenever he enters a new phase.",
            "Double Phase Detonation:\nDeals a DOUBLE burst (16% total Max HP damage, ~1s apart) to Malachar upon every phase change!",
            "Kinetic Energy Lost:\nNo transition burst damage is dealt to Malachar upon phase changes."
        },
        {
            EchoType::OBSIDIAN_KEY,
            "OBSIDIAN KEY",
            "Level 3 - Chamber 2 [Prevent]",
            "Fractured dimensional stone anchoring spatial coordinates.\nIf stolen: Malachar gains chaotic Phase-Shift teleportation.",
            "Dimensional Anchor Secured:\nMalachar is denied the Phase-Shift Blink ability and remains tethered to physical space.",
            "Dimensional Anchor Secured:\nMalachar is denied the Phase-Shift Blink ability and remains tethered to physical space.",
            "Malachar's Phase-Shift:\nMalachar teleports unpredictably across the arena every 6-9 seconds during Phases 2 and 3!"
        }
    };

    for (const auto& meta : echoes) {
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
        sf::Color statusColor = sf::Color(165, 165, 180);
        std::string effectText = meta.previewDesc;
        sf::Color effectColor = sf::Color(210, 210, 225);
        sf::Color cardBg = sf::Color(26, 20, 40, 230);

        if (outcome == EchoOutcome::COLLECTED) {
            std::ostringstream ss;
            if (power >= 90.0f) {
                ss << "[ COLLECTED - " << static_cast<int>(std::round(power)) << "% INTACT ]";
                statusTag = ss.str();
                statusColor = sf::Color(75, 255, 140);
                effectText = meta.intactDesc;
                effectColor = sf::Color(150, 255, 195);
                cardBg = sf::Color(18, 38, 38, 235);
            } else {
                ss << "[ COLLECTED - " << static_cast<int>(std::round(power)) << "% ]";
                statusTag = ss.str();
                statusColor = sf::Color(80, 230, 220);
                effectText = meta.collectedDesc;
                effectColor = sf::Color(170, 245, 235);
                cardBg = sf::Color(18, 34, 42, 235);
            }
        } else if (outcome == EchoOutcome::STOLEN) {
            statusTag = "[ STOLEN BY MALACHAR ]";
            statusColor = sf::Color(255, 80, 80);
            effectText = meta.stolenDesc;
            effectColor = sf::Color(255, 170, 170);
            cardBg = sf::Color(44, 18, 26, 235);
        }

        auto card = cardsContainer->createChild<UI::VerticalBox>()
            ->setModeX(UI::SizeMode::Fixed)
            ->setModeY(UI::SizeMode::Contained)
            ->setFixedWidth(1160.f)
            ->setSpacing(8.f)
            ->setPadding(12.f, 12.f, 20.f, 20.f)
            ->setColor(cardBg);

        // Header Row: Left has Title & Chamber, Right has Status Badge
        auto headerRow = card->createChild<UI::HorizontalBox>()
            ->setModeX(UI::SizeMode::Expanded)
            ->setModeY(UI::SizeMode::Contained)
            ->setAlignmentY(UI::AlignmentY::Middle)
            ->setDistribution(UI::Distribution::SpaceBetween);

        auto titleAndChamber = headerRow->createChild<UI::HorizontalBox>()
            ->setModeX(UI::SizeMode::Contained)
            ->setModeY(UI::SizeMode::Contained)
            ->setAlignmentY(UI::AlignmentY::Middle)
            ->setSpacing(18.f);

        titleAndChamber->createChild<UI::Text>("header", 14)
            ->setString(meta.name)
            ->setFillColor(sf::Color(255, 235, 130));

        titleAndChamber->createChild<UI::Text>("italic", 15)
            ->setString(meta.chamber)
            ->setFillColor(sf::Color(180, 180, 210));

        headerRow->createChild<UI::Text>("bold", 15)
            ->setString(statusTag)
            ->setFillColor(statusColor);

        // Effect Description Text in regular typography with multi-line end lines for high readability
        card->createChild<UI::Text>("regular", 15)
            ->setString(effectText)
            ->setFillColor(effectColor);
    }

    // Bottom controls
    auto bottomBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(8.f);

    closeButton = bottomBox->createChild<UI::Button>("Close", "regular", 22.f)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(200.f)
        ->setFixedHeight(48.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });

    bottomBox->createChild<UI::Text>("italic", 14)
        ->setString("Press [E] or [ESC] to return to game")
        ->setFillColor(sf::Color(160, 160, 185));
}

void EchoLogState::draw(sf::RenderWindow& window) const {
    // Dim background gameplay view with smooth dark vignette
    sf::RectangleShape backdrop(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
    backdrop.setFillColor(sf::Color(8, 6, 14, 230));
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
