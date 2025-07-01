#pragma once
#include <SFML/Window/Event.hpp>

namespace SnakeGame {
	void handleGameInput(sf::Event& event);
	void handleMainMenuInput(sf::Event& event);
	void handleLevelSelectInput(sf::Event& event);
	void handleRecordsInput(sf::Event& event);
	void handleSettingsInput(sf::Event& event);
	void handleGameOverInput(sf::Event& event);
	void handlePauseInput(sf::Event& event);
	void handleNewRecordPromptInput(sf::Event& event);
	void handleNameEntryInput(sf::Event& event);
}