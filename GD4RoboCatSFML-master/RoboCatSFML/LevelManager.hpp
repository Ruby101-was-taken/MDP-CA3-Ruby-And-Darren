#pragma once
// Ruby White - D00255322 - Entire class
class LevelManager {
public:
    static void StaticInit();
    static std::unique_ptr<LevelManager> sInstance;

    LevelManager();
    bool IsCollidingWithWalls(sf::FloatRect collider);
    bool IsCollidingWithOffRoad(sf::FloatRect collider);
    ~LevelManager() = default;

    

//private:
    std::vector<sf::FloatRect> wall_tiles_;
    std::vector<sf::FloatRect> grass_tiles_;
};

