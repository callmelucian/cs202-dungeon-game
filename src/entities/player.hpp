#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "character.hpp"
#include "player-form.hpp"
#include "../core/enums.hpp"
#include <memory>

class Player : public Character {
public:
    Player();
    virtual ~Player() = default;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow &window) override;
    void takeDamage(float amount);

    void switchForm(FormType newForm);
    void gainMomentum(float amount, FormType form);
    void triggerSpecial(int abilityIndex, class Chamber& chamber);

    float& getFormMomentumRef(FormType form);
    float getMomentum(FormType form) const;

    void setSwitchCooldownEnabled(bool enabled);
    float getSwitchCooldownTimer() const;

private:
    std::unique_ptr<PlayerForm> activeForm;
    float wraithbladeMomentum;
    float voidcasterMomentum;
    float ironshellMomentum;
    float switchCooldownTimer;
    bool isSwitchCooldownEnabled;
};

#endif // PLAYER_HPP
