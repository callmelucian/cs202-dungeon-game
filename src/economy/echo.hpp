#ifndef ECHO_HPP
#define ECHO_HPP

#include <vector>
#include <string>
#include <algorithm>

// Forward declarations
class Echo;

class EchoObserver {
public:
    virtual ~EchoObserver() = default;
    virtual void onEchoPowerChanged(float power) = 0;
};

class Echo {
protected:
    std::string name;
    float echoPower;
    std::vector<EchoObserver*> observers;

public:
    Echo(const std::string& name, float initialPower);
    virtual ~Echo() = default;

    virtual void takeDamage(float penalty);
    virtual void addPower(float amount);
    float getPower() const;
    
    void attach(EchoObserver* observer);
    void notify();
};

class ClarityShard : public Echo {
public:
    ClarityShard(float initialPower = 50.0f) : Echo("Clarity Shard", initialPower) {}
    
    // Stored effect values
    float getCollectTimeReduction() const { return 0.10f; } // 10% reduction
    float getCollectTimeReductionFullyIntact() const { return 0.20f; } // 20% reduction
};

class MarrowEcho : public Echo {
public:
    MarrowEcho(float initialPower = 50.0f) : Echo("Marrow Echo", initialPower) {}

    // Stored stolen effect values
    float getBossRegenRate() const { return 0.02f; } // 2% Max HP/sec
    float getEnemySelfHealRate() const { return 0.03f; } // 3% Max HP/sec
};

class HollowBell : public Echo {
public:
    HollowBell(float initialPower = 50.0f) : Echo("Hollow Bell", initialPower) {}

    // Stored effect values
    float getSpecial1ThresholdReduction() const { return 0.15f; } // 15% reduction
    float getSpecial1ThresholdReductionFullyIntact() const { return 0.30f; } // 30% reduction
    float getReflectWardRatio() const { return 0.20f; } // 20% reflected
};

class ResonanceCore : public Echo {
public:
    ResonanceCore(float initialPower = 50.0f) : Echo("Resonance Core", initialPower) {}

    // Stored effect values
    float getBossTransitionDamageRatio() const { return 0.08f; } // 8% current HP
};

class ObsidianKey : public Echo {
public:
    ObsidianKey(float initialPower = 50.0f) : Echo("Obsidian Key", initialPower) {}

    // Stored stolen effect values
    float getBlinkIntervalMin() const { return 6.0f; }
    float getBlinkIntervalMax() const { return 9.0f; }
    float getPitTelegraphOpacityMultiplier() const { return 0.40f; } // 60% opacity reduction (40% remains)
};

#endif // ECHO_HPP
