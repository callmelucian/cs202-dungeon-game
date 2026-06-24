#ifndef COLOR_PALETTE_MANAGER_HPP
#define COLOR_PALETTE_MANAGER_HPP

#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

struct ColorPalette {
    sf::Color primary = sf::Color(0x1F, 0x8A, 0x70);    // Emerald / teal
    sf::Color secondary = sf::Color(0x2D, 0xE1, 0xFC);  // Cyan
    sf::Color background = sf::Color(0x18, 0x1A, 0x1B); // Dark grey
    sf::Color text = sf::Color(0xF0, 0xF0, 0xF0);       // Off-white
    sf::Color accent = sf::Color(0xFF, 0xAE, 0x19);     // Amber / orange
    sf::Color success = sf::Color(0x4E, 0x9F, 0x3D);    // Green
    sf::Color warning = sf::Color(0xD8, 0x3A, 0x56);    // Red
    sf::Color disabled = sf::Color(0x55, 0x55, 0x55);   // Darker grey
};

class ColorPaletteObserver {
public:
    virtual ~ColorPaletteObserver() = default;
    virtual void onColorPaletteChanged(const ColorPalette& newPalette) = 0;
};

class ColorPaletteManager {
public:
    static ColorPaletteManager& getInstance();

    const ColorPalette& getPalette() const;
    void setPalette(const ColorPalette& palette);

    void addObserver(ColorPaletteObserver* observer);
    void removeObserver(ColorPaletteObserver* observer);

    // Serialization / Deserialization helpers
    void load(const nlohmann::json& pal);
    void save(nlohmann::json& pal) const;

private:
    ColorPalette currentPalette;
    std::vector<ColorPaletteObserver*> observers;

    ColorPaletteManager();
    ~ColorPaletteManager() = default;
    ColorPaletteManager(const ColorPaletteManager&) = delete;
    ColorPaletteManager& operator=(const ColorPaletteManager&) = delete;

    void notifyObservers();
};

#endif // COLOR_PALETTE_MANAGER_HPP
