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

#endif // ECHO_HPP
