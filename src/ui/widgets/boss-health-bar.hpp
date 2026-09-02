#ifndef BOSS_HEALTH_BAR_HPP
#define BOSS_HEALTH_BAR_HPP

#include "../base/component.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class BossMalachar;

namespace UI {

class BossHealthBar : public Component {
public:
    BossHealthBar();
    virtual ~BossHealthBar() = default;

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override {}
    void computeSize(sf::Vector2f availableSize) override {}
    void setPosition(sf::Vector2f pos) override {}

    void updateBossState(const BossMalachar& boss);

private:
    float currentHp;
    float maxHp;
    float displayedHpRatio;
    float displayedPhaseRatio;
    float ghostPhaseRatio;
    float ghostLagTimer;
    int currentPhase;
    std::string bossName;
    float lerpSpeed;
    float animTime;
};

} // namespace UI

#endif // BOSS_HEALTH_BAR_HPP
