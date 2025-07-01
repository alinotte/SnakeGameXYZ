#pragma once
#include "GameConstants.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace SnakeGame {
    void drawSnake();
    void drawWalls();
    void drawApple();
    void drawGameObjects();
    void drawGameArea();
    void drawText(const std::string& str, float x, float y, unsigned int size,
        sf::Color color, TextAlignment align,
        sf::Text::Style style);
    void drawGame();
    void drawMainMenu();
    void drawLevelSelect();
    void drawRecordsScreen();
    void drawSettingsMenu();
    void drawGameOverScreen();
    void drawPause();
    void drawNewRecordPrompt();
    void drawNameEntryScreen();
}