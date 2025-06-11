#include "space_invaders/HighScoreManager.hpp"
#include "space_invaders/Constants.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>

namespace SpaceInvaders {

HighScoreManager::HighScoreManager() {
    // Get user's home directory
    std::string homeDir;
    
    #ifdef _WIN32
    homeDir = std::getenv("USERPROFILE");
    #else
    homeDir = std::getenv("HOME");
    #endif
    
    highScoreFilePath = homeDir + "/" + HIGH_SCORE_FILE;
    loadScores();
}

void HighScoreManager::loadScores() {
    highScores.clear();
    
    try {
        if (std::filesystem::exists(highScoreFilePath)) {
            std::ifstream file(highScoreFilePath);
            if (file.is_open()) {
                // Simple file format: each line is name,score,level
                std::string line;
                while (std::getline(file, line)) {
                    HighScoreEntry entry;
                    size_t commaPos1 = line.find(',');
                    size_t commaPos2 = line.find(',', commaPos1 + 1);
                    
                    if (commaPos1 != std::string::npos && commaPos2 != std::string::npos) {
                        entry.name = line.substr(0, commaPos1);
                        entry.score = std::stoi(line.substr(commaPos1 + 1, commaPos2 - commaPos1 - 1));
                        entry.level = std::stoi(line.substr(commaPos2 + 1));
                        highScores.push_back(entry);
                    }
                }
                file.close();
            }
        } else {
            // Create default high scores if file doesn't exist
            highScores = {
                {"CLAUDE", 1000, 3},
                {"IAN", 800, 2},
                {"CPU", 600, 2},
                {"AI", 400, 1},
                {"ML", 200, 1},
                {"GPT", 150, 1},
                {"HAL", 100, 1},
                {"R2D2", 75, 1},
                {"C3PO", 50, 1},
                {"WALLE", 25, 1}
            };
            saveScores();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading high scores: " << e.what() << std::endl;
        highScores.clear();
    }
}

void HighScoreManager::saveScores() {
    try {
        std::ofstream file(highScoreFilePath);
        if (file.is_open()) {
            for (const auto& entry : highScores) {
                file << entry.name << "," << entry.score << "," << entry.level << std::endl;
            }
            file.close();
        } else {
            std::cerr << "Error: Unable to open high score file for writing." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving high scores: " << e.what() << std::endl;
    }
}

bool HighScoreManager::isHighScore(int score) const {
    if (highScores.size() < HIGH_SCORE_COUNT) {
        return true;
    }
    
    int lowestScore = std::numeric_limits<int>::max();
    for (const auto& entry : highScores) {
        lowestScore = std::min(lowestScore, entry.score);
    }
    
    return score > lowestScore;
}

void HighScoreManager::addScore(const std::string& name, int score, int level) {
    HighScoreEntry newEntry{name, score, level};
    highScores.push_back(newEntry);
    
    // Sort by score (highest first)
    std::sort(highScores.begin(), highScores.end(), 
        [](const HighScoreEntry& a, const HighScoreEntry& b) {
            return a.score > b.score;
        });
    
    // Keep only top scores
    if (highScores.size() > HIGH_SCORE_COUNT) {
        highScores.resize(HIGH_SCORE_COUNT);
    }
    
    saveScores();
}

const std::vector<HighScoreEntry>& HighScoreManager::getHighScores() const {
    return highScores;
}

void HighScoreManager::resetScores() {
    if (std::filesystem::exists(highScoreFilePath)) {
        std::filesystem::remove(highScoreFilePath);
    }
    loadScores();
}

} // namespace SpaceInvaders
