#include "color-palette-manager.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

static sf::Color hexToColor(const std::string& hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') {
        s = s.substr(1);
    }
    if (s.length() == 6) {
        unsigned int r = 0, g = 0, b = 0;
        std::stringstream ss;
        ss << std::hex << s.substr(0, 2);
        ss >> r;
        ss.clear();
        ss << std::hex << s.substr(2, 2);
        ss >> g;
        ss.clear();
        ss << std::hex << s.substr(4, 2);
        ss >> b;
        return sf::Color(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
    }
    return sf::Color::White;
}

static std::string colorToHex(const sf::Color& color) {
    std::stringstream ss;
    ss << "#"
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.r)
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.g)
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.b);
    return ss.str();
}

ColorPaletteManager::ColorPaletteManager() {
    // currentPalette is automatically initialized with defaults from struct definition
}

ColorPaletteManager& ColorPaletteManager::getInstance() {
    static ColorPaletteManager instance;
    return instance;
}

const ColorPalette& ColorPaletteManager::getPalette() const {
    return currentPalette;
}

void ColorPaletteManager::setPalette(const ColorPalette& palette) {
    currentPalette = palette;
    notifyObservers();
}

void ColorPaletteManager::addObserver(ColorPaletteObserver* observer) {
    if (observer && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void ColorPaletteManager::removeObserver(ColorPaletteObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void ColorPaletteManager::notifyObservers() {
    for (auto* observer : observers) {
        if (observer) {
            observer->onColorPaletteChanged(currentPalette);
        }
    }
}

void ColorPaletteManager::load(const nlohmann::json& pal) {
    if (pal.contains("primary")) currentPalette.primary = hexToColor(pal["primary"].get<std::string>());
    if (pal.contains("secondary")) currentPalette.secondary = hexToColor(pal["secondary"].get<std::string>());
    if (pal.contains("background")) currentPalette.background = hexToColor(pal["background"].get<std::string>());
    if (pal.contains("text")) currentPalette.text = hexToColor(pal["text"].get<std::string>());
    if (pal.contains("accent")) currentPalette.accent = hexToColor(pal["accent"].get<std::string>());
    if (pal.contains("success")) currentPalette.success = hexToColor(pal["success"].get<std::string>());
    if (pal.contains("warning")) currentPalette.warning = hexToColor(pal["warning"].get<std::string>());
    if (pal.contains("disabled")) currentPalette.disabled = hexToColor(pal["disabled"].get<std::string>());
    
    notifyObservers();
}

void ColorPaletteManager::save(nlohmann::json& pal) const {
    pal["primary"] = colorToHex(currentPalette.primary);
    pal["secondary"] = colorToHex(currentPalette.secondary);
    pal["background"] = colorToHex(currentPalette.background);
    pal["text"] = colorToHex(currentPalette.text);
    pal["accent"] = colorToHex(currentPalette.accent);
    pal["success"] = colorToHex(currentPalette.success);
    pal["warning"] = colorToHex(currentPalette.warning);
    pal["disabled"] = colorToHex(currentPalette.disabled);
}
