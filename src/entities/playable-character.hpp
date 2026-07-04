#ifndef PLAYABLE_CHARACTER_HPP
#define PLAYABLE_CHARACTER_HPP

#include <string>
#include <memory>

class PlayerForm;

class PlayableCharacter {
public:
    PlayableCharacter(const std::string& name, const std::string& description);
    virtual ~PlayableCharacter() = default;

    virtual std::unique_ptr<PlayerForm> createForm1() = 0;
    virtual std::unique_ptr<PlayerForm> createForm2() = 0;
    virtual std::unique_ptr<PlayerForm> createForm3() = 0;

    const std::string& getName() const;
    const std::string& getDescription() const;

protected:
    std::string name;
    std::string description;
};

class Serin : public PlayableCharacter {
public:
    Serin();
    std::unique_ptr<PlayerForm> createForm1() override;
    std::unique_ptr<PlayerForm> createForm2() override;
    std::unique_ptr<PlayerForm> createForm3() override;
};

#endif // PLAYABLE_CHARACTER_HPP
