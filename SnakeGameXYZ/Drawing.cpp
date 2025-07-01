#include "Drawing.h"
#include "GameData.h"
#include "Resources.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace SnakeGame {
    void drawSnake() {
        for (size_t i = 0; i < snake.size(); i++) {
            sf::Sprite segment;
            if (snake[i].isHead) {
                segment.setTexture(snakeHeadTexture);
                segment.setOrigin(snakeHeadTexture.getSize().x / 2, snakeHeadTexture.getSize().y / 2);

                if (direction.x == 1) segment.setRotation(0);
                else if (direction.x == -1) segment.setRotation(180);
                else if (direction.y == -1) segment.setRotation(270);
                else segment.setRotation(90);
            }
            else {
                segment.setTexture(snakeBodyTexture);
                segment.setOrigin(snakeBodyTexture.getSize().x / 2, snakeBodyTexture.getSize().y / 2);

                sf::Vector2i seg_direction;
                if (i < snake.size() - 1) {
                    seg_direction = sf::Vector2i(
                        snake[i + 1].x - snake[i].x,
                        snake[i + 1].y - snake[i].y
                    );
                }
                else {
                    seg_direction = sf::Vector2i(
                        snake[i].x - snake[i - 1].x,
                        snake[i].y - snake[i - 1].y
                    );
                }

                if (seg_direction.x == 1) segment.setRotation(0);
                else if (seg_direction.x == -1) segment.setRotation(180);
                else if (seg_direction.y == -1) segment.setRotation(270);
                else segment.setRotation(90);
            }

            segment.setScale(
                static_cast<float>(CELL_SIZE) / snakeHeadTexture.getSize().x * 0.9f,
                static_cast<float>(CELL_SIZE) / snakeHeadTexture.getSize().y * 0.9f
            );
            segment.setPosition(
                snake[i].x * CELL_SIZE + CELL_SIZE / 2,
                snake[i].y * CELL_SIZE + 50 + CELL_SIZE / 2
            );
            window.draw(segment);
        }
    }

    void drawWalls() {
        wallSprite.setTexture(wallTexture);
        wallSprite.setOrigin(wallTexture.getSize().x / 2, wallTexture.getSize().y / 2);
        wallSprite.setScale(
            static_cast<float>(CELL_SIZE) / wallTexture.getSize().x,
            static_cast<float>(CELL_SIZE) / wallTexture.getSize().y
        );
        for (const auto& wall : walls) {
            wallSprite.setPosition(
                wall.x * CELL_SIZE + CELL_SIZE / 2,
                wall.y * CELL_SIZE + 50 + CELL_SIZE / 2
            );
            window.draw(wallSprite);
        }
    }

    void drawApple() {
        appleSprite.setTexture(appleTexture);
        appleSprite.setOrigin(appleTexture.getSize().x / 2.f, appleTexture.getSize().y / 2.f);
        appleSprite.setScale(
            static_cast<float>(CELL_SIZE) / appleTexture.getSize().x * 0.9f,
            static_cast<float>(CELL_SIZE) / appleTexture.getSize().y * 0.9f
        );
        appleSprite.setPosition(
            apple.x * CELL_SIZE + CELL_SIZE / 2,
            apple.y * CELL_SIZE + 50 + CELL_SIZE / 2
        );
        window.draw(appleSprite);
    }

    void drawGameObjects() {
        drawSnake();
        drawWalls();
        drawApple();
    }

    void drawGameArea() {
        sf::RectangleShape gameArea(sf::Vector2f(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE));
        gameArea.setFillColor(sf::Color::Black);
        gameArea.setPosition(0, 50);
        window.draw(gameArea);
    }

    void drawText(const std::string& str, float x, float y,
        unsigned int size = 30, sf::Color color = sf::Color::White,
        TextAlignment align = TextAlignment::Left,
        sf::Text::Style style = sf::Text::Regular) {
        sf::Text text;
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setStyle(style);

        sf::FloatRect bounds = text.getLocalBounds();

        switch (align)
        {
        case TextAlignment::Left:
            break;
        case TextAlignment::Center:
            x -= bounds.width / 2;
            break;
        case TextAlignment::Right:
            x -= bounds.width;
            break;
        }

        text.setPosition(x, y);
        window.draw(text);
    }

    void drawGame() {
        window.clear(sf::Color(30, 30, 30));

        drawGameArea();
        drawGameObjects();

        if (!game.isGameStarted) {
            int secondsLeft = static_cast<int>(std::ceil(game.startDelayTimer));
            drawText("Game starts in: " + std::to_string(secondsLeft) + "s",
                WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50, 30, sf::Color::Yellow, TextAlignment::Center);

            sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
        }

        std::string scoreText = "Score: " + std::to_string(game.score) +
            " | Level: " + levelMenuItems[game.level];
        drawText(scoreText, WINDOW_WIDTH - 10, 10, 24, sf::Color::White, TextAlignment::Right);

        window.display();
    }

    void drawMainMenu() {
        window.clear(sf::Color(30, 30, 30));

        drawText("Snake Game", WINDOW_WIDTH / 2, 50, 60, sf::Color::Green, TextAlignment::Center);

        for (size_t i = 0; i < mainMenuItems.size(); i++) {
            drawText(mainMenuItems[i], WINDOW_WIDTH / 2, 180 + i * 40, 30,
                i == menuSelection ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        }

        drawText("WSAD: Navigate  Enter: Select  P: Pause  B: Back",
            WINDOW_WIDTH / 2, WINDOW_HEIGHT - 40, 20, sf::Color(150, 150, 150), TextAlignment::Center);

        window.display();
    }

    void drawLevelSelect() {
        window.clear(sf::Color(30, 30, 30));

        drawText("SELECT LEVEL", WINDOW_WIDTH / 2, 50, 50, sf::Color::White, TextAlignment::Center);

        std::vector<std::string> displayNames = levelMenuItems;
        for (size_t i = 0; i < levelMenuItems.size(); i++) {
            if (i == game.selectedDifficulty) {
                displayNames[i] += " (current)";
            }
        }

        for (size_t i = 0; i < levelMenuItems.size(); i++) {
            drawText(displayNames[i], WINDOW_WIDTH / 2, 150 + i * 40, 24,
                i == levelSelection ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        }

        window.display();
    }

    void drawRecordsScreen() {
        window.clear(sf::Color(30, 30, 30));

        drawText("HIGH SCORES", WINDOW_WIDTH / 2, 40, 50, sf::Color::Green, TextAlignment::Center);
        drawText("NAME", 150, 120, 26, sf::Color::Yellow, TextAlignment::Left);
        drawText("SCORE", 400, 120, 26, sf::Color::Yellow, TextAlignment::Left);

        sf::RectangleShape line(sf::Vector2f(500, 2));
        line.setPosition(100, 160);
        line.setFillColor(sf::Color(80, 80, 80));
        window.draw(line);

        for (int i = 0; i < records.size(); i++) {
            float yPos = 180 + i * 40;
            drawText(std::to_string(i + 1) + ".", 100, yPos, 24, sf::Color::White, TextAlignment::Left);
            drawText(records[i].name, 150, yPos, 24, sf::Color::White, TextAlignment::Left);
            drawText(std::to_string(records[i].score), 450, yPos, 24, sf::Color::White, TextAlignment::Right);
        }

        drawText("Press B to return to menu", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 60, 22, sf::Color(150, 150, 150), TextAlignment::Center);

        window.display();
    }

    void drawSettingsMenu() {
        window.clear(sf::Color(30, 30, 30));

        drawText("SETTINGS", WINDOW_WIDTH / 2, 50, 50, sf::Color::Green, TextAlignment::Center);
        drawText("Sound: " + std::string(game.soundEnabled ? "ON" : "OFF"), WINDOW_WIDTH / 2, 150, 30,
            settingsSelection == 0 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        drawText("Music: " + std::string(game.musicEnabled ? "ON" : "OFF"), WINDOW_WIDTH / 2, 200, 30,
            settingsSelection == 1 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        drawText("Back", WINDOW_WIDTH / 2, 350, 30,
            settingsSelection == 2 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);

        window.display();
    }

    void drawGameOverScreen() {
        window.clear(sf::Color(30, 30, 30));

        drawText(game.gameWon ? "YOU WIN!" : "GAME OVER", WINDOW_WIDTH / 2, 150, 30, game.gameWon ? sf::Color::Green : sf::Color::Red, TextAlignment::Center);
        drawText("Your score: " + std::to_string(game.score), WINDOW_WIDTH / 2, 180, 30, sf::Color::White, TextAlignment::Center);

        bool isHighScore = records.size() < 10 || game.score > records.back().score;

        int recordsToShow = std::min(5, (int)records.size());
        for (int i = 0; i < recordsToShow; i++) {
            float yPos = 230 + i * 35;
            drawText(records[i].name, WINDOW_WIDTH / 2 - 120, yPos, 24, sf::Color::White, TextAlignment::Left);
            drawText(std::to_string(records[i].score), WINDOW_WIDTH / 2 + 120, yPos, 24, sf::Color::White, TextAlignment::Right);
        }
        drawText("RESTART", WINDOW_WIDTH / 2 - 220 + 100, WINDOW_HEIGHT - 110, 28,
            gameOverSelection == 0 ? sf::Color::White : sf::Color(180, 180, 180), TextAlignment::Left);
        drawText("MENU", WINDOW_WIDTH / 2 + 20 + 100, WINDOW_HEIGHT - 110, 28,
            gameOverSelection == 1 ? sf::Color::White : sf::Color(180, 180, 180), TextAlignment::Right);

        window.display();
    }

    void drawPause() {
        window.clear(sf::Color(30, 30, 30));

        drawText("PAUSED", WINDOW_WIDTH / 2, 100, 50, sf::Color::White, TextAlignment::Center);
        drawText("CONTINUE", WINDOW_WIDTH / 2, 250, 35,
            pauseSelection == 0 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        drawText("EXIT TO MENU", WINDOW_WIDTH / 2, 300, 35,
            pauseSelection == 1 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);

        window.display();
    }

    void drawNewRecordPrompt() {
        window.clear(sf::Color(30, 30, 30));

        drawText("NEW RECORD!", WINDOW_WIDTH / 2, 100, 50, sf::Color::White, TextAlignment::Center);
        drawText("Score: " + std::to_string(game.score), WINDOW_WIDTH / 2, 180, 30, sf::Color::White, TextAlignment::Center);
        drawText("Enter your name?", WINDOW_WIDTH / 2, 250, 24, sf::Color::White, TextAlignment::Center);
        drawText("YES", WINDOW_WIDTH / 2, 300, 28, promptSelection == 0 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);
        drawText("NO", WINDOW_WIDTH / 2, 350, 28, promptSelection == 1 ? sf::Color::Yellow : sf::Color::White, TextAlignment::Center);

        window.display();
    }

    void drawNameEntryScreen() {
        window.clear(sf::Color(30, 30, 30));

        drawText("Score: " + std::to_string(game.score), WINDOW_WIDTH / 2, 180, 30, sf::Color::White, TextAlignment::Center);
        if (cursorBlinkClock.getElapsedTime().asSeconds() > 0.5f) {
            game.cursorVisible = !game.cursorVisible;
            cursorBlinkClock.restart();
        }
        std::string displayText = game.playerName;
        if (game.cursorVisible) {
            displayText += "_";
        }
        drawText("Enter your name (max 10 characters):", WINDOW_WIDTH / 2, 250, 28, sf::Color::White, TextAlignment::Center);
        drawText(displayText, WINDOW_WIDTH / 2, 300, 28, sf::Color::Green, TextAlignment::Center);
        drawText("Press Enter to confirm", WINDOW_WIDTH / 2, 400, 20, sf::Color(180, 180, 180), TextAlignment::Center);

        window.display();
    }
}