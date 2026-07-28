#include "ui/widgets/button.hpp"
#include "ui/widgets/slider.hpp"
#include "ui/widgets/text-input.hpp"
#include "ui/widgets/player-health-bar.hpp"
#include "ui/widgets/enemy-health-bar.hpp"
#include "ui/base/text.hpp"
#include "ui/containers/flex-box.hpp"
#include "global-settings/asset-manager.hpp"
#include <type_traits>
#include <iostream>
#include <filesystem>
#include <stdexcept>

// Compile-time checks (Acceptance Criteria 2)
void compileTimeChecks() {
    UI::Button* b = nullptr;
    static_assert(std::is_same_v<decltype(b->setModeY(UI::SizeMode::Expanded)->setFixedWidth(200.f)), UI::Button*>);
    static_assert(std::is_same_v<decltype(b->setMargin(0.f, 0.f, 0.f, 0.f)->setOnClick(nullptr)), UI::Button*>);

    UI::Slider* s = nullptr;
    static_assert(std::is_same_v<decltype(s->setModeY(UI::SizeMode::Expanded)->setFixedWidth(200.f)), UI::Slider*>);
    static_assert(std::is_same_v<decltype(s->setRange(0.f, 10.f)), UI::Slider*>);

    UI::TextInput* ti = nullptr;
    static_assert(std::is_same_v<decltype(ti->setModeY(UI::SizeMode::Expanded)->setFixedWidth(200.f)), UI::TextInput*>);

    UI::Text* t = nullptr;
    static_assert(std::is_same_v<decltype(t->setModeY(UI::SizeMode::Expanded)->setFixedWidth(200.f)), UI::Text*>);

    UI::HorizontalBox* hb = nullptr;
    static_assert(std::is_same_v<decltype(hb->setModeY(UI::SizeMode::Expanded)->setPadding(10.f, 10.f, 10.f, 10.f)->setDistribution(UI::Distribution::SpaceBetween)), UI::HorizontalBox*>);

    UI::PlayerHealthBar* phb = nullptr;
    static_assert(std::is_convertible_v<decltype(phb->setHealthRatio(0.5f)->setMomentumRatio(0.2f)->setTogglerVisible(true)), UI::HealthBar*>);

    UI::EnemyHealthBar* ehb = nullptr;
    static_assert(std::is_convertible_v<decltype(ehb->setHealthRatio(0.8f)->setHealth(80.f, 100.f)), UI::HealthBar*>);
}

// Helper to load assets so widget constructors don't fail
void initializeAssetManager() {
    AssetManager& assets = AssetManager::getInstance();
    auto resolvePath = [](const std::string& path) {
        if (std::filesystem::exists(path)) return path;
        if (std::filesystem::exists("../" + path)) return "../" + path;
        return path;
    };
    assets.loadFont("regular", resolvePath("assets/typeface/GoogleSansCode-Regular.ttf"));
    assets.loadTexture("health-bar", resolvePath("assets/textures/health-bar.png"));
}

// Acceptance Criteria 3
void testChildDefaultsRegression() {
    UI::HorizontalBox parent;
    parent.setChildDefaults({
        .modeY = UI::SizeMode::Expanded,
        .fixedWidth = 200.f
    });

    auto b1 = parent.createChild<UI::Button>("Button-1", "regular");
    auto b2 = parent.createChild<UI::Button>("Button-2", "regular");
    auto b3 = parent.createChild<UI::Button>("Button-3", "regular");
    auto sl = parent.createChild<UI::Slider>(0.f, 100.f, 50.f);
    auto ti = parent.createChild<UI::TextInput>("regular");

    if (b1->getModeY() != UI::SizeMode::Expanded || b1->getFixedWidth() != 200.f) throw std::runtime_error("b1 defaults not applied!");
    if (b2->getModeY() != UI::SizeMode::Expanded || b2->getFixedWidth() != 200.f) throw std::runtime_error("b2 defaults not applied!");
    if (b3->getModeY() != UI::SizeMode::Expanded || b3->getFixedWidth() != 200.f) throw std::runtime_error("b3 defaults not applied!");
    if (sl->getModeY() != UI::SizeMode::Expanded || sl->getFixedWidth() != 200.f) throw std::runtime_error("sl defaults not applied!");
    if (ti->getModeY() != UI::SizeMode::Expanded || ti->getFixedWidth() != 200.f) throw std::runtime_error("ti defaults not applied!");
}

// Acceptance Criteria 4
void testOverrideWins() {
    UI::HorizontalBox parent;
    parent.setChildDefaults({
        .modeY = UI::SizeMode::Expanded,
        .fixedWidth = 200.f
    });

    auto b = parent.createChild<UI::Button>("Button", "regular")
                     ->setFixedWidth(350.f);

    if (b->getModeY() != UI::SizeMode::Expanded) throw std::runtime_error("b modeY default not applied!");
    if (b->getFixedWidth() != 350.f) throw std::runtime_error("b explicit width override failed!");
}

// Acceptance Criteria 5
void testPushPopScoping() {
    UI::HorizontalBox parent;
    parent.setChildDefaults({
        .modeY = UI::SizeMode::Expanded,
        .fixedWidth = 100.f
    });

    auto b1 = parent.createChild<UI::Button>("B1", "regular");

    parent.pushChildDefaults({
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 300.f
    });

    auto b2 = parent.createChild<UI::Button>("B2", "regular");

    parent.popChildDefaults();

    auto b3 = parent.createChild<UI::Button>("B3", "regular");

    if (b1->getModeY() != UI::SizeMode::Expanded || b1->getFixedWidth() != 100.f) throw std::runtime_error("b1 defaults incorrect!");
    if (b2->getModeY() != UI::SizeMode::Fixed || b2->getFixedWidth() != 300.f) throw std::runtime_error("b2 defaults incorrect!");
    if (b3->getModeY() != UI::SizeMode::Expanded || b3->getFixedWidth() != 100.f) throw std::runtime_error("b3 defaults incorrect!");
}

void testHealthBars() {
    UI::PlayerHealthBar phb;
    phb.setHealthRatio(0.75f, true)
       ->setMomentumRatio(0.5f, true)
       ->setTogglerVisible(true);
    if (phb.getHealthRatio() != 0.75f) throw std::runtime_error("PlayerHealthBar healthRatio failed!");
    if (phb.getDisplayedHealthRatio() != 0.75f) throw std::runtime_error("PlayerHealthBar displayedHealthRatio immediate failed!");
    if (phb.getMomentumRatio() != 0.5f) throw std::runtime_error("PlayerHealthBar momentumRatio failed!");
    if (!phb.isTogglerVisible()) throw std::runtime_error("PlayerHealthBar toggler flag failed!");

    // Test Lerp transition for PlayerHealthBar
    phb.setHealthRatio(0.25f); // target changed, displayed still 0.75f before update
    if (phb.getDisplayedHealthRatio() == 0.25f) throw std::runtime_error("PlayerHealthBar lerp did not start smooth!");
    phb.update(0.1f);
    if (phb.getDisplayedHealthRatio() >= 0.75f || phb.getDisplayedHealthRatio() <= 0.25f) throw std::runtime_error("PlayerHealthBar lerp update out of bounds!");
    phb.update(2.0f); // enough time to converge
    if (phb.getDisplayedHealthRatio() != 0.25f) throw std::runtime_error("PlayerHealthBar lerp did not converge!");

    phb.setTogglerVisible(false);
    if (phb.isTogglerVisible()) throw std::runtime_error("PlayerHealthBar toggler disable failed!");

    phb.setHealthRatio(1.5f, true);
    if (phb.getHealthRatio() != 1.0f) throw std::runtime_error("PlayerHealthBar ratio upper clamp failed!");

    phb.setHealthRatio(-0.5f, true);
    if (phb.getHealthRatio() != 0.0f) throw std::runtime_error("PlayerHealthBar ratio lower clamp failed!");

    UI::EnemyHealthBar ehb;
    ehb.setHealthRatio(1.0f, true);
    ehb.setHealth(50.f, 200.f); // target = 0.25f
    if (ehb.getHealthRatio() != 0.25f) throw std::runtime_error("EnemyHealthBar setHealth failed!");
    ehb.update(2.0f);
    if (ehb.getDisplayedHealthRatio() != 0.25f) throw std::runtime_error("EnemyHealthBar lerp convergence failed!");
}

void runUITests() {
    std::cout << "[RUNNING UI TESTS]" << std::endl;
    try {
        compileTimeChecks();
        std::cout << "Compile-time checks passed (via static_assert)." << std::endl;

        initializeAssetManager();
        std::cout << "AssetManager initialized." << std::endl;

        testChildDefaultsRegression();
        std::cout << "testChildDefaultsRegression passed." << std::endl;

        testOverrideWins();
        std::cout << "testOverrideWins passed.\n";

        testPushPopScoping();
        std::cout << "testPushPopScoping passed.\n";

        testHealthBars();
        std::cout << "testHealthBars passed.\n";

        // Test user's crashing case (using chained setters)
        std::cout << "Running testCrashIssue (chained setters)..." << std::endl;
        {
            UI::Container root;
            root.setModeX(UI::SizeMode::Fixed);
            root.setModeY(UI::SizeMode::Fixed);
            root.setFixedWidth(800.f);
            root.setFixedHeight(600.f);

            UI::Container* overlays = root.createChild<UI::Container>()
                ->setModeX(UI::SizeMode::Fixed)
                ->setModeY(UI::SizeMode::Fixed)
                ->setFixedWidth(800.f)
                ->setFixedHeight(600.f)
                ->setAlignmentX(UI::AlignmentX::Right)
                ->setAlignmentY(UI::AlignmentY::Top)
                ->setPadding(20.f, 20.f, 20.f, 20.f);

            auto buttonBox = overlays->createChild<UI::HorizontalBox>()
                ->setModeX(UI::SizeMode::Contained)
                ->setModeY(UI::SizeMode::Contained)
                ->setSpacing(25.f)
                ->setDistribution(UI::Distribution::SpaceBetween)
                ->setPadding(20.f, 20.f, 20.f, 20.f)
                ->setColor(sf::Color(255, 255, 255, 1));

            root.computeSize(sf::Vector2f(800.f, 600.f));
            root.setPosition(sf::Vector2f(0.f, 0.f));
        }

        std::cout << "[ALL UI TESTS PASSED SUCCESSFULLY]\n";
    } catch (const std::exception& e) {
        std::cerr << "[TEST FAILURE]: " << e.what() << "\n";
        std::exit(1);
    }
}
