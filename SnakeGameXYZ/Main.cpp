#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <unordered_set>

//misc
void loadRecords();

// Константы
const int GRID_SIZE = 20;
const int CELL_SIZE = 30;
const int WINDOW_WIDTH = GRID_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_SIZE * CELL_SIZE + 50; // +50 для панели счета
const float BASE_SPEED = 0.2f;
std::vector<std::string> levelMenuItems = {
      "Easy",
      "Harder than Easy",
      "Medium",
      "Easier than Hard",
      "Hard",
      "Back"
};

std::vector<std::string> mainMenuItems = {
    "Start Game",
    "Level Select",
    "Records",
    "Settings",
    "Exit"
};

// Перечисление для выравнивания
enum class TextAlignment {
    Left,
    Center,
    Right
};

// Глобальные переменные
sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game");
sf::Font font;
sf::SoundBuffer eatSoundBuffer, crashSoundBuffer, hoverSoundBuffer, startSoundBuffer;
sf::Sound eatSound, crashSound, hoverSound, startSound;
sf::Music bgMusic;
sf::Texture appleTexture, snakeHeadTexture, snakeBodyTexture, wallTexture;
sf::Sprite appleSprite, snakeHeadSprite, snakeBodySprite, wallSprite;
std::string playerName = "XYZ";
bool nameEntryActive = false;
sf::Clock cursorBlinkClock;
bool cursorVisible = true;

enum GameState {
    MAIN_MENU,
    LEVEL_SELECT,
    IN_GAME,
    PAUSE,
    GAME_OVER,
    RECORDS_SCREEN,
    SETTINGS_MENU,
    NEW_RECORD_PROMPT,
    NAME_ENTRY
};
GameState gameState = MAIN_MENU;

struct GameData {
    int score = 0;
    int level = 1;
    float speed = BASE_SPEED;
    int pointsPerApple = 2;
    bool gameOver = false;
    bool gameWon = false;
    bool soundEnabled = true;
    bool musicEnabled = true;
    bool dynamicWalls = false;
    float startDelayTimer = 5.0f;
    bool isGameStarted = false;
    bool isPaused = false;
    int selectedDifficulty = 0;
} game;

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

// Игровые объекты
std::vector<SnakeSegment> snake;
std::vector<Wall> walls;
sf::Vector2i apple;
sf::Vector2i direction(1, 0);
sf::Vector2i nextDirection(1, 0);
std::vector<Record> records(10); // Изначально 10 записей XYZ с 0 очков

// Глобальные переменные меню
int menuSelection = 0;
int levelSelection = 0;
int settingsSelection = 0;
int gameOverSelection = 1;
int promptSelection = 1;
int pauseSelection = 0;



// Функции
void loadResources() {
    if (!font.loadFromFile("Resources/Fonts/PoetsenOne-Regular.ttf")) {
        throw std::runtime_error("Failed to load font");
    }

    if (!eatSoundBuffer.loadFromFile("Resources/Sounds/AppleEatSound.wav") ||
        !crashSoundBuffer.loadFromFile("Resources/Sounds/GameOverSound.wav") ||
        !hoverSoundBuffer.loadFromFile("Resources/Sounds/MenuHoverSound.wav") ||
        !startSoundBuffer.loadFromFile("Resources/Sounds/GameStartSound.wav")) {
        throw std::runtime_error("Failed to load sound files");
    }

    eatSound.setBuffer(eatSoundBuffer);
    crashSound.setBuffer(crashSoundBuffer);
    hoverSound.setBuffer(hoverSoundBuffer);
    startSound.setBuffer(startSoundBuffer);


    if (!bgMusic.openFromFile("Resources/Sounds/BackgroundMusic.wav")) {
        throw std::runtime_error("Failed to load music file");
    }
    bgMusic.setLoop(true);

    // Загрузка текстур
    if (!appleTexture.loadFromFile("Resources/Apple/Apple.png") ||
        !wallTexture.loadFromFile("Resources/Border/BrickLightGrey.png") ||
        !snakeHeadTexture.loadFromFile("Resources/Snake/SnakeHead.png") ||
        !snakeBodyTexture.loadFromFile("Resources/Snake/SnakeBody.png")) {
        throw std::runtime_error("Failed to load texture files");
    }
}

void generateNewApplePosition() {
    std::unordered_set<int> occupiedCells;

    // Добавляем позиции стен
    for (const auto& wall : walls) {
        occupiedCells.insert(wall.y * GRID_SIZE + wall.x);
    }

    // Добавляем позиции змейки
    for (const auto& segment : snake) {
        occupiedCells.insert(segment.y * GRID_SIZE + segment.x);
    }

    // Ищем свободную позицию
    std::vector<sf::Vector2i> freeCells;
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (occupiedCells.count(y * GRID_SIZE + x) == 0) {
                freeCells.emplace_back(x, y);
            }
        }
    }

    // Если есть свободные клетки, выбираем случайную
    if (!freeCells.empty()) {
        int index = rand() % freeCells.size();
        apple = freeCells[index];
    }
    else {
        // Если нет свободных клеток - победа!
        game.gameWon = true;
        game.gameOver = true;
    }
}

bool checkCollision(int x, int y) {
    // 1. Проверка выхода за границы
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
        return true;

    // 2. Проверка стен
    for (const auto& wall : walls) {
        if (x == wall.x && y == wall.y)
            return true;
    }

    // 3. Проверка тела (начиная с 4-го сегмента)
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
    // Добавляем начальные сегменты тела (если нужно)
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
    gameState = MAIN_MENU;
    loadRecords();

    game.score = 0;
    game.gameOver = false;
    game.gameWon = false;
    game.isGameStarted = false;
    game.startDelayTimer = 5.0f; 

    initGameObjects();

    // Настройки в зависимости от уровня
    switch (game.level) {
    case 0: // Простой
        game.speed = BASE_SPEED * 1.0f;
        game.pointsPerApple = 2;
        break;

    case 1: // Тяжелее простого
        game.speed = BASE_SPEED * 0.8f;
        game.pointsPerApple = 4;
        break;

    case 2: // Средний
        game.speed = BASE_SPEED * 0.6f;
        game.pointsPerApple = 6;
        break;

    case 3: // Легче тяжелого
        game.speed = BASE_SPEED * 0.4f;
        game.pointsPerApple = 8;
        break;

    case 4: // Тяжелый
        game.speed = BASE_SPEED * 0.2f;
        game.pointsPerApple = 10;
        break;
    }

    direction = { 1, 0 };
    nextDirection = { 1, 0 };
}

void moveSnake() {
    // Обновляем направление
    direction = nextDirection;

    // Сохраняем старую голову
    SnakeSegment newHead = snake.front();
    newHead.isHead = false; // Сбрасываем флаг для старой головы

    // Перемещаем голову
    newHead.x += direction.x;
    newHead.y += direction.y;
    newHead.isHead = true; // Устанавливаем флаг для новой головы

    // Проверка коллизий
    if (checkCollision(newHead.x, newHead.y)) {
        game.gameOver = true;
        if (game.soundEnabled) crashSound.play();
        return;
    }

    // Проверка яблока (тоже по клеткам!)
    bool ateApple = (newHead.x == apple.x && newHead.y == apple.y);

    // Вставляем новую голову
    snake.insert(snake.begin(), newHead);

    // У старой головы сбрасываем флаг isHead
    if (snake.size() > 1) {
        snake[1].isHead = false;
    }

    // Если не съели яблоко, удаляем последний сегмент
    if (!ateApple) {
        snake.pop_back();
    }
    else {
        game.score += game.pointsPerApple;
        if (game.soundEnabled) eatSound.play();
        generateNewApplePosition();
    }
}

void loadRecords() {
    records.clear();
    std::ifstream file("records.dat");
    if (file.is_open()) {
        Record record;
        while (file >> record.name >> record.score) {
            records.push_back(record);
        }
        file.close();
    }

    // Если записей меньше 10, дополняем дефолтными
    while (records.size() < 10) {
        records.push_back({ "XYZ", 0 });
    }

    // Сортируем записи по убыванию очков
    std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
        return a.score > b.score;
        });
}

void saveRecords() {
    std::ofstream file("records.dat");
    if (file.is_open()) {
        for (const auto& record : records) {
            file << record.name << " " << record.score << " " << "\n";
        }
        file.close();
    }
}

void addRecord(const std::string& name, int score) {
    records.push_back({ name, score });
    std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
        return a.score > b.score;
        });

    if (records.size() > 10) {
        records.pop_back();
    }

    saveRecords();
}

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

void drawGameObjects(){
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

    // Если игра ожидает старта (таймер обратного отсчета)
    if (!game.isGameStarted) {
        int secondsLeft = static_cast<int>(std::ceil(game.startDelayTimer));
        drawText("Game starts in: " + std::to_string(secondsLeft) + "s",
			WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50, 30, sf::Color::Yellow, TextAlignment::Center);

        // Затемнение фона
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
    }

    // Рисуем счет и уровень
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

    // Добавляем "current" к выбранному уровню
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
    window.clear(sf::Color(30, 30, 30)); // Темно-серый фон

 	drawText("HIGH SCORES", WINDOW_WIDTH / 2, 40, 50, sf::Color::Green, TextAlignment::Center);
	drawText("NAME", 150, 120, 26, sf::Color::Yellow, TextAlignment::Left);
	drawText("SCORE", 400, 120, 26, sf::Color::Yellow, TextAlignment::Left);

    // Разделительная линия
    sf::RectangleShape line(sf::Vector2f(500, 2));
    line.setPosition(100, 160);
    line.setFillColor(sf::Color(80, 80, 80));
    window.draw(line);

    // Вывод рекордов
    for (int i = 0; i < 10; i++) {
        float yPos = 180 + i * 40;
        // Порядковый номер
		drawText(std::to_string(i + 1) + ".", 100, yPos, 24, sf::Color::White, TextAlignment::Left);
        // Имя игрока
 		drawText(records[i].name, 150, yPos, 24, sf::Color::White, TextAlignment::Left);
        // Очки 
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

    // Проверка на рекорд
    bool isHighScore = records.size() < 10 || game.score > records.back().score;

    // Вывод 5 лучших рекордов
    int recordsToShow = std::min(5, (int)records.size());
    for (int i = 0; i < recordsToShow; i++) {
        float yPos = 230 + i * 35;
        // Имя игрока
		drawText(records[i].name, WINDOW_WIDTH / 2 - 120, yPos, 24, sf::Color::White, TextAlignment::Left);
        // Очки
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
    // Анимация курсора (мигание каждые 0.5 секунды)
    if (cursorBlinkClock.getElapsedTime().asSeconds() > 0.5f) {
        cursorVisible = !cursorVisible;
        cursorBlinkClock.restart();
    }
    // Добавляем курсор если нужно
    std::string displayText = playerName;
    if (cursorVisible) {
        displayText += "_";
    }
	drawText("Enter your name (max 10 characters):", WINDOW_WIDTH / 2, 250, 28, sf::Color::White, TextAlignment::Center);
	drawText(displayText, WINDOW_WIDTH / 2, 300, 28, sf::Color::Green, TextAlignment::Center);
	drawText("Press Enter to confirm", WINDOW_WIDTH / 2, 400, 20, sf::Color(180, 180, 180), TextAlignment::Center);

    window.display();
}

void handleGameInput(sf::Event& event) {
    if (game.isGameStarted) {
        if (event.type == sf::Event::KeyPressed) {
            if (gameState == IN_GAME) {
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
                    gameState = PAUSE;
                    game.isPaused = true; // Устанавливаем флаг паузы
                    pauseSelection = 0; // Сброс выбора при открытии паузы
                }
            }
        }
    }
}

void handleMainMenuInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::W) {
            menuSelection = (menuSelection - 1 + 5) % 5;
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::S) {
            menuSelection = (menuSelection + 1) % 5;
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            // Обработка выбора меню
            switch (menuSelection) {
            case 0: // Start Game
                game.level = game.selectedDifficulty;
                initGame();
                gameState = IN_GAME;
                if (game.soundEnabled) startSound.play();
                break;
            case 1: // Level Select
                levelSelection = game.level - 1;
                gameState = LEVEL_SELECT;
                levelSelection = 0;
                break;
            case 2: // Records
                loadRecords();
                gameState = RECORDS_SCREEN;
                break;
            case 3:
                settingsSelection = 0;  // Сбрасываем выбор в настройках
                gameState = SETTINGS_MENU;
                settingsSelection = 0;
                break;
            case 4: // Exit
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
            if (levelSelection == 5) { // Back
                gameState = MAIN_MENU;
            }
            else {
                game.selectedDifficulty = levelSelection;
            }
        }
        else if (event.key.code == sf::Keyboard::B) {
            gameState = MAIN_MENU;
        }
    }
}

void handleRecordsInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::B) {
            gameState = MAIN_MENU;
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
            if (settingsSelection == 0) { // Sound
                game.soundEnabled = !game.soundEnabled;
            }
            else if (settingsSelection == 1) { // Music
                game.musicEnabled = !game.musicEnabled;
                if (game.musicEnabled) bgMusic.play();
                else bgMusic.stop();
            }
            else if (settingsSelection == 2) { // Back
                gameState = MAIN_MENU;
            }
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::B) {
            gameState = MAIN_MENU;
        }
    }
}

void handleGameOverInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D) {
            gameOverSelection = 1 - gameOverSelection; // Переключаем между 0 и 1
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (gameOverSelection == 0) {
                initGame();
                gameState = IN_GAME;
                game.startDelayTimer = 5.0f; // Устанавливаем таймер ожидания перед стартом игры
            }
            else {
                gameState = MAIN_MENU;
            }
        }
    }
}

void handlePauseInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::S) {
            pauseSelection = 1 - pauseSelection; // Переключаем между 0 и 1
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (pauseSelection == 0) {
                game.startDelayTimer = 5.0f;
                game.isPaused = false;
                game.isGameStarted = false;
                gameState = IN_GAME; // Продолжить игру
            }
            else {
                gameState = MAIN_MENU; // Выход в меню
            }
            if (game.soundEnabled) hoverSound.play();
        }
    }
}

void handleNewRecordPromptInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::S) {
            promptSelection = 1 - promptSelection; // Переключаем между 0 и 1
            if (game.soundEnabled) hoverSound.play();
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (promptSelection == 0) { // Yes
                gameState = NAME_ENTRY;
                nameEntryActive = true; // Активируем ввод имени
            }
            else { // No
                gameState = GAME_OVER;
            }
        }
    }
}

void handleNameEntryInput(sf::Event& event) {

    if (event.type == sf::Event::TextEntered && nameEntryActive) {
        cursorBlinkClock.restart(); // Сброс таймера мигания курсора
        cursorVisible = true; // Показываем курсор
        if (event.text.unicode == '\b') {
            if (!playerName.empty()) playerName.pop_back();
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 128 && playerName.length() < 10) {
            playerName += static_cast<char>(event.text.unicode);
        }
    }

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        if (playerName.empty()) playerName = "XYZ";
        addRecord(playerName, game.score);
        gameState = GAME_OVER;
        nameEntryActive = false; // Деактивируем ввод имени
    }
}

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

            switch (gameState) {
            case MAIN_MENU:
                handleMainMenuInput(event);
                break;
            case LEVEL_SELECT:
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

        // Обновление игры
        if (gameState == IN_GAME && !game.gameOver) {
            float deltaTime = clock.restart().asSeconds();
            if (deltaTime > 0.1f) deltaTime = 0.1f; // Ограничиваем слишком большие значения

            if (!game.isGameStarted && gameState == IN_GAME) {
                game.startDelayTimer -= deltaTime;
                if (game.startDelayTimer <= 0) {
                    game.isGameStarted = true;
                    game.startDelayTimer = 0;
                }
            }
            else {
                timer += deltaTime;

                if (timer > game.speed) {
                    timer = 0;
                    moveSnake();

                    if (game.gameOver) {
                        gameState = GAME_OVER;
                        bool isNewRecord = records.size() < 10 || game.score > records.back().score;
                        if (isNewRecord) {
                            gameState = isNewRecord ? NEW_RECORD_PROMPT : NAME_ENTRY;
                            promptSelection = 1;
                        }
                    }
                }
            }
        }

        // Отрисовка
        switch (gameState) {
        case MAIN_MENU:
            drawMainMenu();
            break;
        case LEVEL_SELECT:
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
