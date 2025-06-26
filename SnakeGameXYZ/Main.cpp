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
std::vector<std::string> levelMenuList = {
      "Easy",
      "Harder than Easy",
      "Medium",
      "Easier than Hard",
      "Hard",
      "Back"
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

void spawnApple() {
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

void initGame() {
    gameState = MAIN_MENU;
    loadRecords();

    game.score = 0;
    game.gameOver = false;
    game.gameWon = false;
    game.isGameStarted = false;
    game.startDelayTimer = 5.0f;

    // Инициализация змейки
    snake.clear();
    snake.push_back({ GRID_SIZE / 2, GRID_SIZE / 2, true });

    // Добавляем начальные сегменты тела (если нужно)
    for (int i = 1; i < 3; i++) {
        snake.push_back({ GRID_SIZE / 2 - i, GRID_SIZE / 2, false });
    }

    // Генерация стен
    walls.clear();
    for (int i = 0; i < GRID_SIZE; ++i) {
        walls.push_back({ i, 0 }); // Верхняя стена
        walls.push_back({ i, GRID_SIZE - 1 }); // Нижняя стена
        walls.push_back({ 0, i }); // Левая стена
        walls.push_back({ GRID_SIZE - 1, i }); // Правая стена
    }

    // Генерация яблока
    spawnApple();

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
        spawnApple();
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

void drawGame() {
    window.clear(sf::Color(30, 30, 30));

    // Рисуем игровое поле
    sf::RectangleShape gameArea(sf::Vector2f(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE));
    gameArea.setFillColor(sf::Color::Black);
    gameArea.setPosition(0, 50);
    window.draw(gameArea);

    // Рисуем стены
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

    // Рисуем змейку
    for (size_t i = 0; i < snake.size(); i++) {
        sf::Sprite segment;
        if (snake[i].isHead) {
            segment.setTexture(snakeHeadTexture);
            segment.setOrigin(snakeHeadTexture.getSize().x / 2, snakeHeadTexture.getSize().y / 2);
            // Поворот головы
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

    // Рисуем яблоко
    appleSprite.setTexture(appleTexture);
    appleSprite.setOrigin(appleTexture.getSize().x / 2, appleTexture.getSize().y / 2); // Центрирование
    appleSprite.setScale(
        static_cast<float>(CELL_SIZE) / appleTexture.getSize().x * 0.9f, // Масштаб с небольшим отступом
        static_cast<float>(CELL_SIZE) / appleTexture.getSize().y * 0.9f
    );
    appleSprite.setPosition(
        apple.x * CELL_SIZE + CELL_SIZE / 2,
        apple.y * CELL_SIZE + 50 + CELL_SIZE / 2
    );
    window.draw(appleSprite);

    // Если игра ожидает старта (таймер обратного отсчета)
    if (!game.isGameStarted) {
        sf::Text countdownText;
        countdownText.setFont(font);
        // Округляем вверх, чтобы не показывать "0.0"
        int secondsLeft = static_cast<int>(std::ceil(game.startDelayTimer));
        countdownText.setString("Game starts in: " + std::to_string(secondsLeft) + "s");
        countdownText.setCharacterSize(30);
        countdownText.setFillColor(sf::Color::Yellow);
        countdownText.setStyle(sf::Text::Bold);
        countdownText.setOrigin(countdownText.getLocalBounds().width / 2, countdownText.getLocalBounds().height / 2);
        countdownText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        // Затемнение фона
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);

        window.draw(countdownText);
    }

    // Рисуем счет и уровень
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setString("Score: " + std::to_string(game.score) + " | Level: " + levelMenuList[game.level]);
    infoText.setCharacterSize(24);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition(WINDOW_WIDTH - infoText.getLocalBounds().width - 10, 10);
    window.draw(infoText);

    window.display();
}

void drawMainMenu() {
    window.clear(sf::Color(30, 30, 30));

    sf::Text title;
    title.setFont(font);
    title.setString("SNAKE GAME");
    title.setCharacterSize(60);
    title.setFillColor(sf::Color::Green);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 50);
    window.draw(title);

    std::vector<std::string> menuItems = {
        "Start Game",
        "Level Select",
        "Records",
        "Settings",
        "Exit"
    };

    for (size_t i = 0; i < menuItems.size(); i++) {
        sf::Text item;
        item.setFont(font);
        item.setString(menuItems[i]);
        item.setCharacterSize(30);
        item.setFillColor(i == menuSelection ? sf::Color::Yellow : sf::Color::White);
        item.setPosition(WINDOW_WIDTH / 2 - item.getLocalBounds().width / 2, 180 + i * 40);
        window.draw(item);
    }

    sf::Text controls;
    controls.setFont(font);
    controls.setString("WSAD: Navigate  Enter: Select P: Pause B: Back");
    controls.setCharacterSize(20);
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition(WINDOW_WIDTH / 2 - controls.getLocalBounds().width / 2,
        WINDOW_HEIGHT - 40);
    window.draw(controls);

    window.display();
}

void drawLevelSelect() {
    window.clear(sf::Color(30, 30, 30));

    sf::Text title;
    title.setFont(font);
    title.setString("SELECT DIFFICULTY");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::Green);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 50);
    window.draw(title);

    // Добавляем "current" к выбранному уровню
    std::vector<std::string> displayNames = levelMenuList;
    for (size_t i = 0; i < levelMenuList.size(); i++) {
        if (i == game.selectedDifficulty) {
            displayNames[i] += " (current)";
        }
    }

    for (size_t i = 0; i < levelMenuList.size(); i++) {
        sf::Text item;
        item.setFont(font);
        item.setString(displayNames[i]);
        item.setCharacterSize(24);
        item.setFillColor(i == levelSelection ? sf::Color::Yellow : sf::Color::White);
        item.setPosition(WINDOW_WIDTH / 2 - item.getLocalBounds().width / 2, 150 + i * 40);
        window.draw(item);
    }

    window.display();
}

void drawRecordsScreen() {
    window.clear(sf::Color(30, 30, 30)); // Темно-серый фон

    // Заголовок
    sf::Text title;
    title.setFont(font);
    title.setString("HIGH SCORES");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::Green);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 40);
    window.draw(title);

    // Шапка таблицы
    sf::Text headerName;
    headerName.setFont(font);
    headerName.setString("NAME");
    headerName.setCharacterSize(26);
    headerName.setFillColor(sf::Color::Yellow);
    headerName.setPosition(150, 120);
    window.draw(headerName);

    sf::Text headerScore;
    headerScore.setFont(font);
    headerScore.setString("SCORE");
    headerScore.setCharacterSize(26);
    headerScore.setFillColor(sf::Color::Yellow);
    headerScore.setPosition(400, 120);
    window.draw(headerScore);

    // Разделительная линия
    sf::RectangleShape line(sf::Vector2f(500, 2));
    line.setPosition(100, 160);
    line.setFillColor(sf::Color(80, 80, 80));
    window.draw(line);

    // Вывод рекордов
    for (int i = 0; i < 10; i++) {
        float yPos = 180 + i * 40;

        // Порядковый номер
        sf::Text numText;
        numText.setFont(font);
        numText.setString(std::to_string(i + 1) + ".");
        numText.setCharacterSize(24);
        numText.setFillColor(sf::Color::White);
        numText.setPosition(100, yPos);
        window.draw(numText);

        // Имя игрока
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(records[i].name);
        nameText.setCharacterSize(24);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition(150, yPos);
        window.draw(nameText);

        // Очки (выравнивание по правому краю)
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString(std::to_string(records[i].score));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(400, yPos);
        window.draw(scoreText);
    }

    // Кнопка возврата
    sf::Text backText;
    backText.setFont(font);
    backText.setString("Press B to return to menu");
    backText.setCharacterSize(22);
    backText.setFillColor(sf::Color(150, 150, 150));
    backText.setPosition(WINDOW_WIDTH / 2 - backText.getLocalBounds().width / 2,
        WINDOW_HEIGHT - 60);
    window.draw(backText);

    window.display();
}

void drawSettingsMenu() {
    window.clear(sf::Color(30, 30, 30));

    // Заголовок
    sf::Text title;
    title.setFont(font);
    title.setString("SETTINGS");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::Green);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 50);
    window.draw(title);

    // Настройки звука
    sf::Text soundText;
    soundText.setFont(font);
    soundText.setString("Sound: " + std::string(game.soundEnabled ? "ON" : "OFF"));
    soundText.setCharacterSize(30);
    soundText.setFillColor(settingsSelection == 0 ? sf::Color::Yellow : sf::Color::White);
    soundText.setPosition(WINDOW_WIDTH / 2 - soundText.getLocalBounds().width / 2, 150);
    window.draw(soundText);

    // Настройки музыки
    sf::Text musicText;
    musicText.setFont(font);
    musicText.setString("Music: " + std::string(game.musicEnabled ? "ON" : "OFF"));
    musicText.setCharacterSize(30);
    musicText.setFillColor(settingsSelection == 1 ? sf::Color::Yellow : sf::Color::White);
    musicText.setPosition(WINDOW_WIDTH / 2 - musicText.getLocalBounds().width / 2, 200);
    window.draw(musicText);

    // Кнопка назад
    sf::Text backText;
    backText.setFont(font);
    backText.setString("Back");
    backText.setCharacterSize(30);
    backText.setFillColor(settingsSelection == 2 ? sf::Color::Yellow : sf::Color::White);
    backText.setPosition(WINDOW_WIDTH / 2 - backText.getLocalBounds().width / 2, 280);
    window.draw(backText);

    window.display();
}

void drawGameOverScreen() {
    window.clear(sf::Color(30, 30, 30));

    // Заголовок
    sf::Text title;
    title.setFont(font);
    title.setString(game.gameWon ? "YOU WIN!" : "GAME OVER");
    title.setCharacterSize(50);
    title.setFillColor(game.gameWon ? sf::Color::Green : sf::Color::Red);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 100);
    window.draw(title);

    // Счет
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Your score: " + std::to_string(game.score));
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - scoreText.getLocalBounds().width / 2, 180);
    window.draw(scoreText);

    // Проверка на рекорд
    bool isHighScore = records.size() < 10 || game.score > records.back().score;

    // Вывод 5 лучших рекордов
    int recordsToShow = std::min(5, (int)records.size());
    for (int i = 0; i < recordsToShow; i++) {
        float yPos = 230 + i * 35;

        // Имя игрока
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(records[i].name);
        nameText.setCharacterSize(24);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition(WINDOW_WIDTH / 2 - 120, yPos);
        window.draw(nameText);

        // Очки (выравнивание по правому краю)
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString(std::to_string(records[i].score));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(WINDOW_WIDTH / 2 + 120 - scoreText.getLocalBounds().width, yPos);
        window.draw(scoreText);
    }

    // Кнопка "Restart"
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString("RESTART");
    restartText.setCharacterSize(28);
    restartText.setFillColor(gameOverSelection == 0 ? sf::Color::White : sf::Color(180, 180, 180));
    restartText.setPosition(WINDOW_WIDTH / 2 - 220 + 100 - restartText.getLocalBounds().width / 2,
        WINDOW_HEIGHT - 110);
    window.draw(restartText);

    // Кнопка "Main Menu"
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setString("MENU");
    menuText.setCharacterSize(28);
    menuText.setFillColor(gameOverSelection == 1 ? sf::Color::White : sf::Color(180, 180, 180));
    menuText.setPosition(WINDOW_WIDTH / 2 + 20 + 100 - menuText.getLocalBounds().width / 2,
        WINDOW_HEIGHT - 110);
    window.draw(menuText);

    window.display();
}

void drawPause() {
    window.clear(sf::Color(30, 30, 30));

    // Заголовок
    sf::Text pauseText;
    pauseText.setFont(font);
    pauseText.setString("PAUSED");
    pauseText.setCharacterSize(50);
    pauseText.setFillColor(sf::Color::White);
    pauseText.setPosition(WINDOW_WIDTH / 2 - pauseText.getLocalBounds().width / 2, 150);
    window.draw(pauseText);

    // Кнопка Continue
    sf::Text continueText;
    continueText.setFont(font);
    continueText.setString("CONTINUE");
    continueText.setCharacterSize(35);
    continueText.setFillColor(pauseSelection == 0 ? sf::Color::Yellow : sf::Color::White);
    continueText.setPosition(WINDOW_WIDTH / 2 - continueText.getLocalBounds().width / 2, 250);
    window.draw(continueText);

    // Кнопка Exit to menu
    sf::Text exitText;
    exitText.setFont(font);
    exitText.setString("EXIT TO MENU");
    exitText.setCharacterSize(35);
    exitText.setFillColor(pauseSelection == 1 ? sf::Color::Yellow : sf::Color::White);
    exitText.setPosition(WINDOW_WIDTH / 2 - exitText.getLocalBounds().width / 2, 300);
    window.draw(exitText);

    window.display();
}

void drawNewRecordPrompt() {
    window.clear(sf::Color(30, 30, 30));

    // Заголовок
    sf::Text title;
    title.setFont(font);
    title.setString("NEW RECORD!");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::Yellow);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 100);
    window.draw(title);

    // Информация о рекорде
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + std::to_string(game.score));
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - scoreText.getLocalBounds().width / 2, 180);
    window.draw(scoreText);

    // Вопрос
    sf::Text question;
    question.setFont(font);
    question.setString("Enter your name?");
    question.setCharacterSize(30);
    question.setFillColor(sf::Color::White);
    question.setPosition(WINDOW_WIDTH / 2 - question.getLocalBounds().width / 2, 250);
    window.draw(question);

    // Кнопки Да/Нет
    sf::Text yesText;
    yesText.setFont(font);
    yesText.setString("YES");
    yesText.setCharacterSize(28);
    yesText.setFillColor(promptSelection == 0 ? sf::Color::Green : sf::Color(150, 150, 150));
    yesText.setPosition(WINDOW_WIDTH / 2 - yesText.getLocalBounds().width / 2, 300);
    window.draw(yesText);

    sf::Text noText;
    noText.setFont(font);
    noText.setString("NO");
    noText.setCharacterSize(28);
    noText.setFillColor(promptSelection == 1 ? sf::Color::Red : sf::Color(150, 150, 150));
    noText.setPosition(WINDOW_WIDTH / 2 - noText.getLocalBounds().width / 2, 350);
    window.draw(noText);

    window.display();
}

void drawNameEntryScreen() {
    window.clear(sf::Color(30, 30, 30));

    // Информация о рекорде
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + std::to_string(game.score));
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - scoreText.getLocalBounds().width / 2, 180);
    window.draw(scoreText);

    // Поле ввода имени с курсором
    sf::Text nameText;
    nameText.setFont(font);

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

    nameText.setString("Enter your name: " + displayText);
    nameText.setCharacterSize(28);
    nameText.setFillColor(sf::Color::Green);
    nameText.setPosition(WINDOW_WIDTH / 2 - nameText.getLocalBounds().width / 2, 250);
    window.draw(nameText);

    // Подсказка по вводу
    sf::Text hintText;
    hintText.setFont(font);
    hintText.setString("Press Enter to confirm");
    hintText.setCharacterSize(20);
    hintText.setFillColor(sf::Color(180, 180, 180));
    hintText.setPosition(WINDOW_WIDTH / 2 - hintText.getLocalBounds().width / 2, 320);
    window.draw(hintText);

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
