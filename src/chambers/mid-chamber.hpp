#ifndef MID_CHAMBER_HPP
#define MID_CHAMBER_HPP

#include "chamber.hpp"

class MidChamber : public Chamber {
public:
    MidChamber(Player& player);
    virtual ~MidChamber(); // handles onExit if destroyed
    
    void update(float dt) override;
    
protected:
    void completeChamber() override;
};

#endif // MID_CHAMBER_HPP
