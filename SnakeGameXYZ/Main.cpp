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

enum class Direction
{
	Up,
	Down,
	Left,
	Right
};

Direction currentDirection = Direction::Right; // starting direction
bool directionChanged = false; // flag to prevent sudden change in direction
const float SPEED = 1.0f; // update intervals
sf::Clock moveClock; // movement timer

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

void UpdateSnake()
{
	// move snake only by intervals
	if (moveClock.getElapsedTime().asSeconds() >= SPEED)
	{
		// saving previous snake head
		Segment newHead = snake[0];

		// update snake head position
		switch (currentDirection)
		{
		case Direction::Up:
			newHead.y -= SEGMENT_SIZE;
			break;
		case Direction::Down:
			newHead.y += SEGMENT_SIZE;
			break;
		case Direction::Left:
			newHead.x -= SEGMENT_SIZE;
			break;
		case Direction::Right:
			newHead.x += SEGMENT_SIZE;
			break;
		}

		// adding new snake head
		snake.insert(snake.begin(), newHead);

		// deleting snake tail
		snake.pop_back();

		moveClock.restart(); // reset timer
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
			GenerateFood(); // checking through recursion
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
			// change movement direction if it hasnt changed in the current frame
			if (!directionChanged)
			{
				switch (keyEvent->code)
				{
				case sf::Keyboard::Key::Up:
					if (currentDirection != Direction::Down)
					{
						currentDirection = Direction::Up;
						directionChanged = true;
					}
					break;
				case sf::Keyboard::Key::Down:
					if (currentDirection != Direction::Up)
					{
						currentDirection = Direction::Down;
						directionChanged = true;
					}
					break;
				case sf::Keyboard::Key::Left:
					if (currentDirection != Direction::Right)
					{
						currentDirection = Direction::Left;
						directionChanged = true;
					}
					break;
				case sf::Keyboard::Key::Right:
					if (currentDirection != Direction::Left)
					{
						currentDirection = Direction::Right;
						directionChanged = true;
					}
					break;
				case sf::Keyboard::Key::Escape:
					window.close();
					break;
				}
			}
		}
	}

	directionChanged = false; // reset flag after all events processed
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
	UpdateSnake();
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