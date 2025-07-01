#include "InputHandling.h"
#include "GameData.h"
#include "Resources.h"
#include "Records.h"
#include "GameObjects.h"
#include <SFML/Window/Event.hpp>

namespace SnakeGame {
    void handleGameInput(sf::Event& event) {
        if (game.isGameStarted) {
            if (event.type == sf::Event::KeyPressed) {
                if (game.gameState == IN_GAME) {
                    if (event.key.code == sf::Keyboard::W && direction.y == 0) {
                        nextDirection = { 0, -1 };
                    }
                    else if (event.key.code == sf::Keyboard::S && direction.y == 0) {
                        nextDirection = { 0, 1 };
                    }
                    else if (event.key.code == sf::Keyboard::A && direction.x == 0) {
                        nextDirection = { -1, 0 };
                    }
                    else if (event.key.code == sf::Keyboard::D && direction.x == 0) {
                        nextDirection = { 1, 0 };
                    }
                    else if (event.key.code == sf::Keyboard::P) {
                        game.gameState = PAUSE;
                        game.isPaused = true;
                        pauseSelection = 0;
                    }
                }
            }
        }
    }

    void handleMainMenuInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::W) {
                menuSelection = static_cast<MainMenu>((menuSelection - 1 + 5) % 5);
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::S) {
                menuSelection = static_cast<MainMenu>((menuSelection + 1) % 5);
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                // Обработка выбора меню
                switch (menuSelection) {
                case START_GAME:
                    game.level = static_cast<DifficultySettings>(game.selectedDifficulty);
                    initGame();
                    game.gameState = IN_GAME;
                    if (game.soundEnabled) startSound.play();
                    break;
                case LEVEL_SELECTION:
                    levelSelection = game.level - 1;
                    game.gameState = LEVEL_SELECT_MENU;
                    levelSelection = 0;
                    break;
                case RECORDS:
                    loadRecords();
                    game.gameState = RECORDS_SCREEN;
                    break;
                case SETTINGS:
                    settingsSelection = 0;
                    game.gameState = SETTINGS_MENU;
                    settingsSelection = 0;
                    break;
                case EXIT:
                    window.close();
                    return;
                }
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Escape) {
                window.close();
                return;
            }
        }
    }

    void handleLevelSelectInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::W) {
                levelSelection = (levelSelection - 1 + 6) % 6;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::S) {
                levelSelection = (levelSelection + 1) % 6;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (levelSelection == 5) {
                    game.gameState = MAIN_MENU;
                }
                else {
                    game.selectedDifficulty = levelSelection;
                }
            }
            else if (event.key.code == sf::Keyboard::B) {
                game.gameState = MAIN_MENU;
            }
        }
    }

    void handleRecordsInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::B) {
                game.gameState = MAIN_MENU;
            }
        }
    }

    void handleSettingsInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::W) {
                settingsSelection = (settingsSelection - 1 + 3) % 3;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::S) {
                settingsSelection = (settingsSelection + 1) % 3;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (settingsSelection == 0) {
                    game.soundEnabled = !game.soundEnabled;
                }
                else if (settingsSelection == 1) {
                    game.musicEnabled = !game.musicEnabled;
                    if (game.musicEnabled) bgMusic.play();
                    else bgMusic.stop();
                }
                else if (settingsSelection == 2) {
                    game.gameState = MAIN_MENU;
                }
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::B) {
                game.gameState = MAIN_MENU;
            }
        }
    }

    void handleGameOverInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D) {
                gameOverSelection = 1 - gameOverSelection;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (gameOverSelection == 0) {
                    initGame();
                    game.gameState = IN_GAME;
                    game.startDelayTimer = 5.0f;
                }
                else {
                    game.gameState = MAIN_MENU;
                }
            }
        }
    }

    void handlePauseInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::S) {
                pauseSelection = 1 - pauseSelection;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (pauseSelection == 0) {
                    game.startDelayTimer = 5.0f;
                    game.isPaused = false;
                    game.isGameStarted = false;
                    game.gameState = IN_GAME;
                }
                else {
                    game.gameState = MAIN_MENU;
                }
                if (game.soundEnabled) hoverSound.play();
            }
        }
    }

    void handleNewRecordPromptInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::S) {
                promptSelection = 1 - promptSelection;
                if (game.soundEnabled) hoverSound.play();
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (promptSelection == 0) {
                    game.gameState = NAME_ENTRY;
                    game.nameEntryActive = true;
                }
                else {
                    game.gameState = GAME_OVER;
                }
            }
        }
    }

    void handleNameEntryInput(sf::Event& event) {

        if (event.type == sf::Event::TextEntered && game.nameEntryActive) {
            cursorBlinkClock.restart();
            game.cursorVisible = true;
            if (event.text.unicode == '\b') {
                if (!game.playerName.empty()) game.playerName.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 128 && game.playerName.length() < 10) {
                game.playerName += static_cast<char>(event.text.unicode);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            if (game.playerName.empty()) game.playerName = "XYZ";
            addRecord(game.playerName, game.score);
            game.gameState = GAME_OVER;
            game.nameEntryActive = false;
        }
    }
}