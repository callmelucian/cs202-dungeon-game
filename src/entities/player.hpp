#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "character.hpp"
#include "player-form.hpp"
#include "playable-character.hpp"
#include "../core/enums.hpp"
#include <SFML/Window/Event.hpp>
#include <memory>
#include <map>

class Player : public Character {
public:
    Player(PlayableCharacter& character);
    virtual ~Player() = default;

    void handleInput(const sf::Event& event);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow &window) override;
    void takeDamage(float rawAmount) override;

    bool switchForm(FormType newForm);
    void gainMomentum(float amount, FormType form);
    void triggerSpecial(int abilityIndex, class Chamber& chamber);
    void attack(sf::Vector2f targetDir, class Chamber& chamber);

    float getMomentum(FormType form) const;
    void setSwitchCooldownEnabled(bool enabled);
    float getSwitchCooldownTimer() const;

    FormType getActiveFormType() const;
    const PlayableCharacter& getCharacter() const;
    PlayerCombatStateMachine& getStateMachine();

    Stats getEffectiveStats() const override;

private:
    PlayableCharacter* character;
    std::map<FormType, std::unique_ptr<PlayerForm>> forms;
    PlayerForm* activeForm;
    std::map<FormType, float> formMomentum;
    PlayerCombatStateMachine stateMachine;

    float switchCooldownTimer;
    bool isSwitchCooldownEnabled;
};

#endif // PLAYER_HPP
