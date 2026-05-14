#include "RoboCatPCH.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::unique_ptr<LevelManager> LevelManager::sInstance;

void LevelManager::StaticInit() {
    sInstance.reset(new LevelManager());
}

LevelManager::LevelManager() {
    std::string filename = "../Assets/Levels/Level.csv";

    std::ifstream file(filename);

    if (!file.is_open()) {
        Logging::Log("LevelManager", "NO FILE");
    }

    std::string line;
    std::vector<std::vector<std::string>> data;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;

        while (std::getline(ss, cell, ',')) {
            row.emplace_back(cell);
        }

        data.emplace_back(row);
    }


    int x = 0;
    int y = 0;
    int tile_size = 16;

    file.close();

    level_tiles_.clear();

    for (int y = 0; y < data.size(); ++y) {
        const auto& row = data[y];
        for (int x = 0; x < row.size(); ++x) {
            const auto& cell = row[x];
            if (std::stoi(cell) == 0) {
                level_tiles_.emplace_back(sf::FloatRect({ ((x * 80) - 2560) * 1.f, ((y * 80) - 2560) * 1.f }, { 16 * 5.f, 16 * 5.f }));
            }
        }
    }
}

bool LevelManager::IsCollidingWithLevel(sf::FloatRect collider) {

    //Logging::Log("LevelManager", "Level tiles: "+std::to_string(level_tiles_.size()));
    for (const sf::FloatRect& rect : level_tiles_) {
        auto intersection = rect.intersects(collider);
        //Logging::Log("LevelManager", "Checking Collision");
        //Logging::Log("LevelManager", "BOX: " + std::to_string(rect.left) + ", " + std::to_string(rect.top));
        //Logging::Log("LevelManager", "COL: " + std::to_string(collider.left) + ", " + std::to_string(collider.top));
        if (intersection) {
            return true;
        }
    }

    return false;
}