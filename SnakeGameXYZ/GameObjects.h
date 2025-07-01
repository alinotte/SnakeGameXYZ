#pragma once

namespace SnakeGame {
	void initApple();
	void initSnake();
	void initWalls();
	void initGameObjects();
	void initGame();

	void moveSnake();
	void generateNewApplePosition();
	bool checkCollision(int x, int y);
}