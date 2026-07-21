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
    void draw(sf::RenderWindow &window) const override;
    void takeDamage(float rawAmount) override;

    bool switchForm(FormType newForm);
    void gainMomentum(float amount, FormType form);
    void triggerSpecial(int abilityIndex, class Chamber& chamber);
    void attack(sf::Vector2f targetDir, class Chamber& chamber);

    void setChamber(class Chamber* chamber);
    void applySlowAura(std::vector<class Enemy*>& enemies);
    
    void setInMidChamber(bool value);
    bool getInMidChamber() const;

    float getMomentum(FormType form) const;
    void setSwitchCooldownEnabled(bool enabled);
    float getSwitchCooldownTimer() const;
    bool getIsFacingRight() const;
    
    void setSpecial1Threshold(float threshold);
    float getSpecial1Threshold() const;

    FormType getActiveFormType() const;
    const PlayableCharacter& getCharacter() const;
    PlayerCombatStateMachine& getStateMachine();

    Stats getEffectiveStats() const override;

private:
    PlayableCharacter* character;
    class Chamber* currentChamber = nullptr;
    std::map<FormType, std::unique_ptr<PlayerForm>> forms;
    PlayerForm* activeForm;
    std::map<FormType, float> formMomentum;
    PlayerCombatStateMachine stateMachine;

    float switchCooldownTimer;
    bool isSwitchCooldownEnabled;
    bool isFacingRight;
    bool isAttacking;
    bool inMidChamber = false;
    float special1Threshold = 50.0f;
};

#endif // PLAYER_HPP
