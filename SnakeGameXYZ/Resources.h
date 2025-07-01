#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace SnakeGame{
	extern sf::Font font;
	extern sf::SoundBuffer eatSoundBuffer, crashSoundBuffer, hoverSoundBuffer, startSoundBuffer;
	extern sf::Sound eatSound, crashSound, hoverSound, startSound;
	extern sf::Music bgMusic;
	extern sf::Texture appleTexture, snakeHeadTexture, snakeBodyTexture, wallTexture;
	extern sf::Sprite appleSprite, snakeHeadSprite, snakeBodySprite, wallSprite;
	
	void loadResources();
}