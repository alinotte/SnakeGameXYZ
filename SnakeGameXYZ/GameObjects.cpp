#include "GameObjects.h"
#include "GameData.h"
#include "Resources.h"
#include "Records.h"
#include <unordered_set>

namespace SnakeGame {
    void moveSnake() {
        direction = nextDirection;

        SnakeSegment newHead = snake.front();
        newHead.isHead = false;

        newHead.x += direction.x;
        newHead.y += direction.y;
        newHead.isHead = true;

        if (checkCollision(newHead.x, newHead.y)) {
            game.gameOver = true;
            if (game.soundEnabled) crashSound.play();
            return;
        }

        bool ateApple = (newHead.x == apple.x && newHead.y == apple.y);

        snake.insert(snake.begin(), newHead);

        if (snake.size() > 1) {
            snake[1].isHead = false;
        }

        if (!ateApple) {
            snake.pop_back();
        }
        else {
            game.score += game.pointsPerApple;
            if (game.soundEnabled) eatSound.play();
            generateNewApplePosition();
        }
    }

    void generateNewApplePosition() {
        std::unordered_set<int> occupiedCells;
        
        for (const auto& wall : walls) {
            occupiedCells.insert(wall.y * GRID_SIZE + wall.x);
        }

        for (const auto& segment : snake) {
            occupiedCells.insert(segment.y * GRID_SIZE + segment.x);
        }

        std::vector<sf::Vector2i> freeCells;
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                if (occupiedCells.count(y * GRID_SIZE + x) == 0) {
                    freeCells.emplace_back(x, y);
                }
            }
        }

        if (!freeCells.empty()) {
            int index = rand() % freeCells.size();
            apple = freeCells[index];
        }
        else {
            game.gameWon = true;
            game.gameOver = true;
        }
    }

    bool checkCollision(int x, int y) {
        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
            return true;

        for (const auto& wall : walls) {
            if (x == wall.x && y == wall.y)
                return true;
        }

        for (size_t i = 3; i < snake.size(); i++) {
            if (x == snake[i].x && y == snake[i].y)
                return true;
        }

        return false;
    }

    void initApple() {
        generateNewApplePosition();
    }

    void initSnake() {
        snake.clear();
        snake.push_back({ GRID_SIZE / 2, GRID_SIZE / 2, true });
        for (int i = 1; i < 3; i++) {
            snake.push_back({ GRID_SIZE / 2 - i, GRID_SIZE / 2, false });
        }
    }

    void initWalls() {
        walls.clear();
        for (int i = 0; i < GRID_SIZE; ++i) {
            walls.push_back({ i, 0 });
            walls.push_back({ i, GRID_SIZE - 1 });
            walls.push_back({ 0, i });
            walls.push_back({ GRID_SIZE - 1, i });
        }
    }

    void initGameObjects() {
        initApple();
        initSnake();
        initWalls();
    }

    void initGame() {
        game.gameState = MAIN_MENU;
        loadRecords();

        game.score = 0;
        game.gameOver = false;
        game.gameWon = false;
        game.isGameStarted = false;
        game.startDelayTimer = 5.0f;

        initGameObjects();

        switch (game.level) {
        case EASY:
            game.timePerCell = BASE_SPEED * 1.0f;
            game.pointsPerApple = 2;
            break;

        case HARDER_THAN_EASY:
            game.timePerCell = BASE_SPEED * 0.8f;
            game.pointsPerApple = 4;
            break;

        case MEDIUM:
            game.timePerCell = BASE_SPEED * 0.6f;
            game.pointsPerApple = 6;
            break;

        case EASIER_THAN_HARD:
            game.timePerCell = BASE_SPEED * 0.4f;
            game.pointsPerApple = 8;
            break;

        case HARD:
            game.timePerCell = BASE_SPEED * 0.2f;
            game.pointsPerApple = 10;
            break;
        }

        direction = { 1, 0 };
        nextDirection = { 1, 0 };
    }
}