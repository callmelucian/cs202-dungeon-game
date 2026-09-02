/**
 * @file story-intro-state.hpp
 * @brief Narrative prologue state introducing Serin, Malachar, the Vault, and Echo mechanics.
 */

#ifndef STORY_INTRO_STATE_HPP
#define STORY_INTRO_STATE_HPP

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"
#include <vector>
#include <string>

struct StorySlide {
    std::string tag;
    std::string title;
    std::string subtitle;
    std::string body;
};

class StoryIntroState : public GameState {
public:
    explicit StoryIntroState(StateManager& manager);
    virtual ~StoryIntroState() override = default;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void handleEvents(sf::Event& event) override;

private:
    void setupUI();
    void updateSlideContent();
    void nextSlide();
    void prevSlide();
    void startGame();

    std::vector<StorySlide> slides;
    size_t currentSlideIndex;

    // UI Structure
    UI::VerticalBox* layoutBox;
    UI::VerticalBox* contentCard;
    UI::Text* slideTagText;
    UI::Text* slideTitleText;
    UI::Text* slideSubtitleText;
    UI::Text* slideBodyText;
    UI::Text* pageIndicatorText;

    UI::HorizontalBox* navBar;
    UI::Button* prevButton;
    UI::Button* nextButton;
    UI::Button* skipButton;
};

#endif // STORY_INTRO_STATE_HPP
