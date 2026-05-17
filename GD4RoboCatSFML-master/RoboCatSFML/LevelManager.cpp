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

    wall_tiles_.clear();

    for (int y = 0; y < data.size(); ++y) {
        const auto& row = data[y];
        for (int x = 0; x < row.size(); ++x) {
            const auto& cell = row[x];
            if (std::stoi(cell) == 0) {
                wall_tiles_.emplace_back(sf::FloatRect({ ((x * 80) - 2560) * 1.f, ((y * 80) - 2560) * 1.f }, { 80.f, 80.f }));
            }
            if (std::stoi(cell) == 1) {
                // I love hardcoded values mmmmmmmm
                grass_tiles_.emplace_back(sf::FloatRect({ (((x * 80) - 2560) * 1.f) + 30, (((y * 80) - 2560) * 1.f) + 30 }, { 20.f, 20.f }));
            }
        }
    }
}

bool LevelManager::IsCollidingWithWalls(sf::FloatRect collider) {

    for (const sf::FloatRect& rect : wall_tiles_) {
        auto intersection = rect.intersects(collider);
        if (intersection) {
            return true;
        }
    }

    return false;
}
bool LevelManager::IsCollidingWithOffRoad(sf::FloatRect collider) {

    for (const sf::FloatRect& rect : grass_tiles_) {
        auto intersection = rect.intersects(collider);
        if (intersection) {
            return true;
        }
    }

    return false;
}