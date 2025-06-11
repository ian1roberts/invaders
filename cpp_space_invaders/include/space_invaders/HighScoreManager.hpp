#pragma once

#include <string>
#include <vector>

namespace SpaceInvaders {

struct HighScoreEntry {
    std::string name;
    int score;
    int level;
};

class HighScoreManager {
public:
    HighScoreManager();
    ~HighScoreManager() = default;
    
    void loadScores();
    void saveScores();
    bool isHighScore(int score) const;
    void addScore(const std::string& name, int score, int level);
    const std::vector<HighScoreEntry>& getHighScores() const;
    void resetScores();
    
private:
    std::vector<HighScoreEntry> highScores;
    std::string highScoreFilePath;
};

} // namespace SpaceInvaders
