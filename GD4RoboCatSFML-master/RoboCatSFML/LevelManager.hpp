#pragma once
// Ruby White - D00255322 - Entire class
class LevelManager {
public:
    static void StaticInit();
    static std::unique_ptr<LevelManager> sInstance;

    LevelManager();
    bool IsCollidingWithLevel(sf::FloatRect collider);
    ~LevelManager() = default;

    

//private:
    std::vector<sf::FloatRect> level_tiles_;
};

