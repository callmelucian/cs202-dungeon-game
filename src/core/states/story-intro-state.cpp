#include "story-intro-state.hpp"
#include "game-play-state.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <algorithm>

StoryIntroState::StoryIntroState(StateManager& manager)
    : GameState(manager), currentSlideIndex(0)
{
    drawBackground = false;

    // Define the 4-part narrative prologue in simple, engaging English
    slides = {
        {
            "PROLOGUE  |  PART 1",
            "THE ASHEN VAULT",
            "An Ancient Prison Deep Underground",
            "Deep underground lies the Ashen Vault -- a giant stone fortress\nbuilt long ago to lock away the dark Lich King, Lord Malachar.\n\nYou play as Serin, a relic hunter.\nWhile exploring the silent ruins, you accidentally broke the ancient seal.\nNow the vault doors are locked shut, and the dark king is waking up."
        },
        {
            "PROLOGUE  |  PART 2",
            "WHAT ARE THE ECHOES?",
            "Crystals of Ancient Memory & Soul",
            "Echoes are glowing crystals that hold the ancient memories and magic\nof the Vault. Long ago, Malachar bound his immortal soul to 5 Echoes.\n\nMalachar has sent his monsters across the chambers to steal all 5 Echoes.\nIf monsters steal an Echo, Malachar absorbs its power,\ngaining deadly abilities (like healing and teleporting) in the final boss battle!\nIf you defend an Echo, you deny Malachar that power."
        },
        {
            "PROLOGUE  |  PART 3",
            "SERIN'S THREE FORMS",
            "Channeling Ancient Warrior Spirits",
            "To survive the horrors within, Serin channels 3 shifting warrior forms:\n\n  * Wraithblade [Key 1]: A swift ghost-blade. Uses fast slashes, rift\n    knockback to push enemies away, and a quick dash [Right Click].\n  * Voidcaster [Key 2]: A void archer. Fires long-range piercing\n    arrows aimed with your mouse to eliminate distant foes.\n  * Ironshell [Key 3]: A living stone armor. Absorbs heavy damage,\n    cleaves crowds, and slows nearby monsters with a calming aura."
        },
        {
            "PROLOGUE  |  PART 4",
            "SURVIVE AND ESCAPE",
            "Defend the Vault -- Shape Your Destiny",
            "Serin is trapped. Your only way out is through Lord Malachar.\n\nSwitch forms freely in combat to counter different threats and build Momentum.\nIn each chamber, protect the Echoes from corruption and eliminate the monsters!\n\nHow many Echoes you save will decide the fate of your escape\nand which of the 3 story endings you unlock."
        }
    };

    setupUI();
    updateSlideContent();
}

void StoryIntroState::setupUI() {
    SettingManager& settings = SettingManager::getInstance();

    root->setAlignmentX(UI::AlignmentX::Center);
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Main layout container - sized comfortably to fit inside window boundaries
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Fixed)
        ->setFixedWidth(1120.f)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(20.f)
        ->setPadding(30.f, 30.f, 35.f, 35.f)
        ->setColor(sf::Color(14, 12, 24, 250));

    // Top Header Row: Tag on left, Skip button on top right
    auto topRow = layoutBox->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentY(UI::AlignmentY::Middle)
        ->setDistribution(UI::Distribution::SpaceBetween);

    slideTagText = topRow->createChild<UI::Text>("bold", 15)
        ->setString("PROLOGUE  |  PART 1")
        ->setFillColor(sf::Color(140, 180, 255));

    skipButton = topRow->createChild<UI::Button>("Skip Intro [Esc]", "regular", 16)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(170.f)
        ->setFixedHeight(36.f)
        ->setOnClick([this]() {
            startGame();
        });

    // Content Card Box
    contentCard = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(12.f)
        ->setPadding(24.f, 24.f, 28.f, 28.f)
        ->setColor(sf::Color(22, 20, 36, 230));

    slideTitleText = contentCard->createChild<UI::Text>("header", 28)
        ->setString("THE ASHEN VAULT")
        ->setFillColor(sf::Color(255, 215, 80));

    slideSubtitleText = contentCard->createChild<UI::Text>("italic", 17)
        ->setString("An Ancient Prison Deep Underground")
        ->setFillColor(sf::Color(200, 210, 235))
        ->setMarginBottom(8.f);

    slideBodyText = contentCard->createChild<UI::Text>("regular", 18)
        ->setString("")
        ->setFillColor(sf::Color(240, 240, 250))
        ->setLineSpacing(1.35f);

    // Bottom Navigation Bar
    navBar = layoutBox->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentY(UI::AlignmentY::Middle)
        ->setDistribution(UI::Distribution::SpaceBetween)
        ->setMarginTop(10.f);

    prevButton = navBar->createChild<UI::Button>("< Back", "regular", 18)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(160.f)
        ->setFixedHeight(46.f)
        ->setOnClick([this]() {
            prevSlide();
        });

    pageIndicatorText = navBar->createChild<UI::Text>("bold", 16)
        ->setString("Slide 1 / 3")
        ->setFillColor(sf::Color(180, 180, 200));

    nextButton = navBar->createChild<UI::Button>("Next >", "regular", 18)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(200.f)
        ->setFixedHeight(46.f)
        ->setOnClick([this]() {
            nextSlide();
        });
}

void StoryIntroState::updateSlideContent() {
    if (currentSlideIndex >= slides.size()) return;

    const auto& currentSlide = slides[currentSlideIndex];

    if (slideTagText) slideTagText->setString(currentSlide.tag);
    if (slideTitleText) slideTitleText->setString(currentSlide.title);
    if (slideSubtitleText) slideSubtitleText->setString(currentSlide.subtitle);
    if (slideBodyText) slideBodyText->setString(currentSlide.body);

    if (pageIndicatorText) {
        pageIndicatorText->setString("Part " + std::to_string(currentSlideIndex + 1) + " of " + std::to_string(slides.size()));
    }

    if (prevButton) {
        prevButton->setEnabled(currentSlideIndex > 0);
    }

    if (nextButton) {
        if (currentSlideIndex + 1 == slides.size()) {
            nextButton->setLabelText("Enter the Vault");
            nextButton->setFixedWidth(220.f);
        } else {
            nextButton->setLabelText("Next >");
            nextButton->setFixedWidth(160.f);
        }
    }
}

void StoryIntroState::nextSlide() {
    SoundManager::getInstance().playSound("menu-nav");
    if (currentSlideIndex + 1 < slides.size()) {
        currentSlideIndex++;
        updateSlideContent();
    } else {
        startGame();
    }
}

void StoryIntroState::prevSlide() {
    if (currentSlideIndex > 0) {
        SoundManager::getInstance().playSound("menu-nav");
        currentSlideIndex--;
        updateSlideContent();
    }
}

void StoryIntroState::startGame() {
    SoundManager::getInstance().playSound("echo-collect");
    stateManager.changeState(std::make_unique<GameplayState>(stateManager));
}

void StoryIntroState::update(float deltaTime) {
    GameState::update(deltaTime);
}

void StoryIntroState::draw(sf::RenderWindow& window) const {
    // Atmospheric dark backdrop
    sf::RectangleShape backdrop(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
    backdrop.setFillColor(sf::Color(8, 6, 14, 255));
    window.draw(backdrop);

    GameState::draw(window);
}

void StoryIntroState::handleEvents(sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == sf::Keyboard::Scancode::Space || 
            keyEvent->scancode == sf::Keyboard::Scancode::Enter ||
            keyEvent->scancode == sf::Keyboard::Scancode::Right) {
            nextSlide();
            return;
        } else if (keyEvent->scancode == sf::Keyboard::Scancode::Left) {
            prevSlide();
            return;
        } else if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
            startGame();
            return;
        }
    }

    GameState::handleEvents(event);
}
