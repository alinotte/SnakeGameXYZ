#pragma once
#include <string>

namespace SnakeGame{
	void loadRecords();
	void saveRecords();
	void addRecord(const std::string& name, int score);
}