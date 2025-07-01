#include "GameData.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace SnakeGame {
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game");
	GameData game;
	std::vector<SnakeSegment> snake;
	sf::Vector2i direction(1, 0);
	sf::Vector2i nextDirection(1, 0);
	std::vector<Wall> walls;
	sf::Vector2i apple;
	std::vector<Record> records(10);
	MainMenu menuSelection = START_GAME;

	int levelSelection = 0;
	int settingsSelection = 0;
	int gameOverSelection = 1;
	int promptSelection = 1;
	int pauseSelection = 0;

	sf::Clock cursorBlinkClock;
}