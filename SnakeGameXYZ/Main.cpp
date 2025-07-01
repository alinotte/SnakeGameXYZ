#include "GameData.h"
#include "Resources.h"
#include "InputHandling.h"
#include "Drawing.h"
#include "GameObjects.h"
#include "Records.h"

using namespace SnakeGame;

int main() {

    srand(static_cast<unsigned>(time(nullptr)));
    loadResources();
    loadRecords();

    if (game.musicEnabled) {
        bgMusic.play();
    }

    sf::Clock clock;
    float timer = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            switch (game.gameState) {
            case MAIN_MENU:
                handleMainMenuInput(event);
                break;
            case LEVEL_SELECT_MENU:
                handleLevelSelectInput(event);
                break;
            case IN_GAME:
                handleGameInput(event);
                break;
            case PAUSE:
                handlePauseInput(event);
                break;
            case GAME_OVER:
                handleGameOverInput(event);
                break;
            case RECORDS_SCREEN:
                handleRecordsInput(event);
                break;
            case SETTINGS_MENU:
                handleSettingsInput(event);
                break;
            case NEW_RECORD_PROMPT:
                handleNewRecordPromptInput(event);
                break;
            case NAME_ENTRY:
                handleNameEntryInput(event);
                break;
            }
        }

        if (game.gameState == IN_GAME && !game.gameOver) {
            float deltaTime = clock.restart().asSeconds();
            if (deltaTime > 0.1f) deltaTime = 0.1f; // Ограничиваем слишком большие значения

            if (!game.isGameStarted && game.gameState == IN_GAME) {
                game.startDelayTimer -= deltaTime;
                if (game.startDelayTimer <= 0) {
                    game.isGameStarted = true;
                    game.startDelayTimer = 0;
                }
            }
            else {
                timer += deltaTime;

                if (timer > game.timePerCell) {
                    timer = 0;
                    moveSnake();

                    if (game.gameOver) {
                        game.gameState = GAME_OVER;
                        bool isNewRecord = records.size() < 10 || game.score > records.back().score;
                        if (isNewRecord) {
                            game.gameState = isNewRecord ? NEW_RECORD_PROMPT : NAME_ENTRY;
                            promptSelection = 1;
                        }
                    }
                }
            }
        }

        switch (game.gameState) {
        case MAIN_MENU:
            drawMainMenu();
            break;
        case LEVEL_SELECT_MENU:
            drawLevelSelect();
            break;
        case IN_GAME:
            drawGame();
            break;
        case PAUSE:
            drawPause();
            break;
        case GAME_OVER:
            drawGameOverScreen();
            break;
        case RECORDS_SCREEN:
            drawRecordsScreen();
            break;
        case SETTINGS_MENU:
            drawSettingsMenu();
            break;
        case NEW_RECORD_PROMPT:
            drawNewRecordPrompt();
            break;
        case NAME_ENTRY:
            drawNameEntryScreen();
            break;
        }
    }

    return 0;
}
