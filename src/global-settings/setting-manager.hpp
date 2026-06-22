#ifndef SETTING_MANAGER
#define SETTING_MANAGER

class SettingManager {
public:
    static SettingManager& getInstance() {
        static SettingManager instance;
        return instance;
    }

    unsigned int getWidth() const { return screenWidth; }
    unsigned int getHeight() const { return screenHeight; }

private:
    const unsigned int screenWidth, screenHeight;
    
    SettingManager();
    SettingManager (const SettingManager&) = delete;
    SettingManager& operator=(const SettingManager&) = delete;
};

#endif // SETTING_MANAGER