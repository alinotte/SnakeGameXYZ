#pragma once
#include <vector>
#include <string>

namespace SnakeGame {
    const int GRID_SIZE = 20;
    const int CELL_SIZE = 30;
    const int WINDOW_WIDTH = GRID_SIZE * CELL_SIZE;
    const int WINDOW_HEIGHT = GRID_SIZE * CELL_SIZE + 50;
    const float BASE_SPEED = 0.2f;

    enum DifficultySettings {
        EASY,
        HARDER_THAN_EASY,
        MEDIUM,
        EASIER_THAN_HARD,
        HARD
    };

    enum GameState {
        MAIN_MENU,
        LEVEL_SELECT_MENU,
        IN_GAME,
        PAUSE,
        GAME_OVER,
        RECORDS_SCREEN,
        SETTINGS_MENU,
        NEW_RECORD_PROMPT,
        NAME_ENTRY
    };

    enum MainMenu {
        START_GAME,
        LEVEL_SELECTION,
        RECORDS,
        SETTINGS,
        EXIT
    };

    enum class TextAlignment {
        Left,
        Center,
        Right
    };

    extern std::vector<std::string> levelMenuItems;
    extern std::vector<std::string> mainMenuItems;
}
