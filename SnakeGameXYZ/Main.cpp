#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>

// game window
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// snake
const int SEGMENT_SIZE = 20; // one snake segment size 20x20
struct Segment // position of a segment
{
	float x, y;
};

// food
struct Food
{
	int x, y;
};

// globals
sf::RenderWindow window;
std::vector<Segment> snake; // snake segments in a vector
sf::Color snakeColor = sf::Color::Green; // snake color
Food food; // food position
sf::Color foodColor = sf::Color::Red; // food color


// init snake
void SnakeInit()
{
	snake.clear(); // clear snake vector in case of restart

	// starting position (window center)
	float startX = WINDOW_WIDTH / 2 / SEGMENT_SIZE * SEGMENT_SIZE; // align to grid
	float startY = WINDOW_HEIGHT / 2 / SEGMENT_SIZE * SEGMENT_SIZE;

	// creating starting segments (3 in a row)
	for (int i = 0; i < 3; ++i)
	{
		snake.push_back({ startX - i * SEGMENT_SIZE, startY });
	}
}

// draw snake
void DrawSnake()
{
	sf::RectangleShape segmentShape(sf::Vector2f(SEGMENT_SIZE, SEGMENT_SIZE));
	segmentShape.setFillColor(snakeColor);

	for (const auto& segment : snake)
	{
		segmentShape.setPosition(sf::Vector2f(segment.x, segment.y));
		window.draw(segmentShape);
	}
}

void GenerateFood()
{
	// rng init
	static std::random_device rd;
	static std::mt19937 gen(rd());

	// define field borders (accounting segment size)
	std::uniform_int_distribution<> distX(0, (WINDOW_WIDTH - SEGMENT_SIZE) / SEGMENT_SIZE); // ???
	std::uniform_int_distribution<> distY(0, (WINDOW_HEIGHT - SEGMENT_SIZE) / SEGMENT_SIZE);

	// generate position divisible by segment size
	food.x = distX(gen) * SEGMENT_SIZE;
	food.y = distY(gen) * SEGMENT_SIZE;

	// checking if food generated on snake
	for (const auto& segment : snake)
	{
		if (segment.x == food.x && segment.y == food.y)
		{
			GenerateFood(); // ???
			return;
		}
	}
}

void DrawFood()
{
	sf::RectangleShape foodShape(sf::Vector2f(SEGMENT_SIZE, SEGMENT_SIZE));
	foodShape.setFillColor(foodColor);
	foodShape.setPosition(sf::Vector2f(static_cast<float>(food.x), static_cast<float>(food.y)));
	window.draw(foodShape);
}

// handle input
void HandleInput()
{
	while (auto event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			window.close();
		}

		if (auto keyEvent = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyEvent->code == sf::Keyboard::Key::Escape)
			{
				window.close();
			}
		}
	}
}

// game init
void GameInit()
{
	window.create(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Snake Game");
	window.setFramerateLimit(60);
	SnakeInit();
	GenerateFood();
}

void GameUpdate()
{
	// game logic
}

void DrawGame()
{
	window.clear(sf::Color::Black);

	DrawSnake();
	DrawFood();

	window.display();
}

int main()
{
	// sfml version snippet
	std::cout << "SFML version: "
		<< SFML_VERSION_MAJOR << "."
		<< SFML_VERSION_MINOR << "\n";

	GameInit();
	while (window.isOpen())
	{
		HandleInput();
		GameUpdate();
		DrawGame();
	}

	return 0;
}