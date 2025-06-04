#include <SFML/Graphics.hpp>
#include <iostream>

// game window
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// snake
const int SEGMENT_SIZE = 20; // one snake segment size 20x20
struct Segment // position of a segment
{
	float x, y;
};

// globals
sf::RenderWindow window;
std::vector<Segment> snake; // snake segments in a vector
sf::Color snakeColor = sf::Color::Green; // snake color

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
}

void GameUpdate()
{
	// game logic
}

void DrawGame()
{
	window.clear(sf::Color::Black);

	DrawSnake();

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