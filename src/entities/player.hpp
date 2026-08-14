#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "character.hpp"
#include "player-form.hpp"
#include "playable-character.hpp"
#include "player-movement-controller.hpp"
#include "player-animation-controller.hpp"
#include "../core/enums.hpp"
#include "../ui/widgets/player-health-bar.hpp"
#include <SFML/Window/Event.hpp>
#include <memory>
#include <map>

class Player : public Character {
public:
    Player(PlayableCharacter& character);
    virtual ~Player() = default;

    void handleInput(const sf::Event& event);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void takeDamage(float rawAmount) override;
    void onWallCollision() override;

    /// Public wrapper over the protected notifyStateChanged — lets collaborators
    /// (e.g. PlayerAnimationController) fire animation transitions.
    void triggerAnimation(const std::string& key);

    bool switchForm(FormType newForm);
    float getMomentum(FormType form) const;
    void gainMomentum(float amount, FormType form);
    void setMomentum(float amount, FormType form);
    void triggerSpecial(int abilityIndex, class Chamber& chamber);
    void attack(sf::Vector2f targetDir, class Chamber& chamber);

    void setChamber(class Chamber* chamber);
    class Chamber* getChamber() const { return currentChamber; }
    void applySlowAura(std::vector<class Enemy*>& enemies);
    
    void setInMidChamber(bool value);
    bool getInMidChamber() const;

    void setSwitchCooldownEnabled(bool enabled);
    float getSwitchCooldownTimer() const;
    bool getIsFacingRight() const;
    
    void setSpecial1Threshold(float threshold);
    float getSpecial1Threshold() const;

    FormType getActiveFormType() const;
    PlayerForm* getActiveForm() const;
    const PlayableCharacter& getCharacter() const;
    PlayerCombatStateMachine& getStateMachine();

    /// Returns true when the currently playing animation clip has finished.
    bool isAnimationFinished() const;

    Stats getEffectiveStats() const override;

    UI::PlayerHealthBar* getPlayerHealthBar();
    const UI::PlayerHealthBar* getPlayerHealthBar() const;

private:
    PlayableCharacter* character;
    class Chamber* currentChamber = nullptr;
    std::map<FormType, std::unique_ptr<PlayerForm>> forms;
    PlayerForm* activeForm;
    std::map<FormType, float> formMomentum;
    PlayerCombatStateMachine stateMachine;

    PlayerMovementController movementController;
    PlayerAnimationController animController;

    float switchCooldownTimer;
    bool isSwitchCooldownEnabled;
    bool inMidChamber = false;
    float special1Threshold = 50.0f;
};

#endif // PLAYER_HPP
