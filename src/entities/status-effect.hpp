#ifndef STATUS_EFFECT_HPP
#define STATUS_EFFECT_HPP

class Character;

class StatusEffect {
public:
    StatusEffect (float duration);
    virtual ~StatusEffect() = default;

    virtual void apply (Character& character) = 0;
    virtual void remove (Character& character) = 0;
    virtual bool update (float dt, Character& character);

    void refresh(const StatusEffect& other);

    float getDuration() const;
    float getTimer() const;

protected:
    float duration, timer;
};

#endif // STATUS_EFFECT_HPP