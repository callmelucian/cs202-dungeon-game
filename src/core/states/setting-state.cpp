#include "setting-state.hpp"
#include "../../global-settings/sound-manager.hpp"

static std::string scancodeToString(sf::Keyboard::Scancode sc) {
    sf::String desc = sf::Keyboard::getDescription(sc);
    std::string str = desc.toAnsiString();
    if (str.empty()) {
        switch (sc) {
            case sf::Keyboard::Scancode::W: return "W";
            case sf::Keyboard::Scancode::A: return "A";
            case sf::Keyboard::Scancode::S: return "S";
            case sf::Keyboard::Scancode::D: return "D";
            case sf::Keyboard::Scancode::J: return "J";
            case sf::Keyboard::Scancode::Q: return "Q";
            case sf::Keyboard::Scancode::E: return "E";
            case sf::Keyboard::Scancode::Num1: return "1";
            case sf::Keyboard::Scancode::Num2: return "2";
            case sf::Keyboard::Scancode::Num3: return "3";
            case sf::Keyboard::Scancode::Space: return "Space";
            case sf::Keyboard::Scancode::LShift: return "LShift";
            default: return "Key " + std::to_string(static_cast<int>(sc));
        }
    }
    return str;
}

static std::string bindingToString(const ActionBinding& binding) {
    if (binding.isMouseButton) {
        switch (binding.mouseButton) {
            case sf::Mouse::Button::Left: return "L-Mouse";
            case sf::Mouse::Button::Right: return "R-Mouse";
            case sf::Mouse::Button::Middle: return "M-Mouse";
            case sf::Mouse::Button::Extra1: return "Mouse 4";
            case sf::Mouse::Button::Extra2: return "Mouse 5";
            default: return "Mouse";
        }
    }
    return scancodeToString(binding.scancode);
}

static std::string getActionDisplayName(const std::string& action) {
    if (action == "MoveUp") return "Move Up";
    if (action == "MoveDown") return "Move Down";
    if (action == "MoveLeft") return "Move Left";
    if (action == "MoveRight") return "Move Right";
    if (action == "Dash") return "Dash (Wraith)";
    if (action == "Attack") return "Attack";
    if (action == "Special1") return "Special 1";
    if (action == "Special2") return "Special 2";
    if (action == "SwitchForm1") return "Form 1 (Wraith)";
    if (action == "SwitchForm2") return "Form 2 (Void)";
    if (action == "SwitchForm3") return "Form 3 (Ironshell)";
    return action;
}

SettingState::SettingState(StateManager& manager) : GameState(manager), pendingRebindAction("") {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text
    titleText = layoutBox->createChild<UI::Text>("header", 36)
        ->setString("Settings");

    // Horizontal box for Audio (Left) and Keybinds (Right)
    columnsBox = layoutBox->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(60.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Left Column: Audio Settings
    audioBox = columnsBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(15.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    audioBox->createChild<UI::Text>("header", 20)
        ->setString("Audio Options")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    musicLabel = audioBox->createChild<UI::Text>("regular", 18)
        ->setString("Music Volume")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    musicSlider = audioBox->createChild<UI::Slider>(0.f, 100.f, settings.getMusicVolume())
        ->setFixedWidth(240.f)
        ->setFixedHeight(30.f)
        ->setOnValueChanged([](float val) {
            SettingManager::getInstance().setMusicVolume(val);
            SoundManager::getInstance().setMusicVolume(val);
        });

    sfxLabel = audioBox->createChild<UI::Text>("regular", 18)
        ->setString("SFX Volume")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    sfxSlider = audioBox->createChild<UI::Slider>(0.f, 100.f, settings.getSfxVolume())
        ->setFixedWidth(240.f)
        ->setFixedHeight(30.f)
        ->setOnValueChanged([](float val) {
            SettingManager::getInstance().setSfxVolume(val);
            SoundManager::getInstance().setSfxVolume(val);
        });

    backButton = audioBox->createChild<UI::Button>("Back", "regular", 22.f)
        ->setFixedWidth(240.f)
        ->setFixedHeight(45.f)
        ->setOnClick([this]() {
            SettingManager::getInstance().saveSettings("settings.json");
            stateManager.popState();
        });

    // Right Column: Controls
    keybindBox = columnsBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(10.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    keybindBox->createChild<UI::Text>("header", 20)
        ->setString("Controls")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    const std::vector<std::string> actions = {
        "MoveUp", "MoveDown", "MoveLeft", "MoveRight",
        "Dash",
        "Attack", "Special1", "Special2",
        "SwitchForm1", "SwitchForm2", "SwitchForm3"
    };

    for (const auto& action : actions) {
        auto row = keybindBox->createChild<UI::HorizontalBox>()
            ->setModeX(UI::SizeMode::Contained)
            ->setModeY(UI::SizeMode::Contained)
            ->setSpacing(15.f);

        row->createChild<UI::Text>("regular", 16)
            ->setString(getActionDisplayName(action))
            ->setFixedWidth(180.f)
            ->setFixedHeight(30.f);

        ActionBinding b = settings.getActionBinding(action);
        std::string btnText = "[" + bindingToString(b) + "]";

        auto btn = row->createChild<UI::Button>(btnText, "regular", 16.f)
            ->setFixedWidth(140.f)
            ->setFixedHeight(30.f);

        btn->setOnClick([this, action, btn]() {
            pendingRebindAction = action;
            btn->setLabelText("[ Press Key ]");
        });

        keyButtons[action] = btn;
    }
}

void SettingState::refreshKeyBindingLabels() {
    SettingManager& settings = SettingManager::getInstance();
    for (auto& [action, btn] : keyButtons) {
        if (btn) {
            ActionBinding b = settings.getActionBinding(action);
            btn->setLabelText("[" + bindingToString(b) + "]");
        }
    }
}

void SettingState::handleEvents(sf::Event& event) {
    if (!pendingRebindAction.empty()) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode != sf::Keyboard::Scancode::Unknown && keyPressed->scancode != sf::Keyboard::Scancode::Escape) {
                SettingManager::getInstance().setActionScancode(pendingRebindAction, keyPressed->scancode);
                SettingManager::getInstance().saveSettings("settings.json");
            }
            pendingRebindAction = "";
            refreshKeyBindingLabels();
            return;
        } else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            SettingManager::getInstance().setActionMouseButton(pendingRebindAction, mousePressed->button);
            SettingManager::getInstance().saveSettings("settings.json");
            pendingRebindAction = "";
            refreshKeyBindingLabels();
            return;
        }
    }
    GameState::handleEvents(event);
}
