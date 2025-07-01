#pragma once
#include "GameConstants.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace SnakeGame{
    struct SnakeSegment {
        int x, y;
        bool isHead = false;
    };

    struct Wall {
        int x, y;
    };

    struct Record {
        std::string name = "XYZ";
        int score = 0;
    };

    struct GameData {
        int score = 0;
        DifficultySettings level = EASY;
        float timePerCell = BASE_SPEED;
        int pointsPerApple = 2;
        bool gameOver = false;
        bool gameWon = false;
        bool soundEnabled = true;
        bool musicEnabled = true;
        float startDelayTimer = 5.0f;
        bool isGameStarted = false;
        bool isPaused = false;
        int selectedDifficulty = 0;
        GameState gameState = MAIN_MENU;
        std::string playerName = "XYZ";
        bool nameEntryActive = false;
        bool cursorVisible = true;
    };

	extern sf::RenderWindow window;
	extern GameData game;
    extern std::vector<SnakeSegment> snake;
    extern sf::Vector2i direction;
    extern sf::Vector2i nextDirection;
    extern std::vector<Wall> walls;
    extern sf::Vector2i apple;
    extern std::vector<Record> records;
    extern MainMenu menuSelection;
    extern int levelSelection;
    extern int settingsSelection;
    extern int gameOverSelection;
    extern int promptSelection;
    extern int pauseSelection;

    extern sf::Clock cursorBlinkClock;
}