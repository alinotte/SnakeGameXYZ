#include "Resources.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cassert>

namespace SnakeGame {

    sf::Font font;
    sf::SoundBuffer eatSoundBuffer, crashSoundBuffer, hoverSoundBuffer, startSoundBuffer;
    sf::Sound eatSound, crashSound, hoverSound, startSound;
    sf::Music bgMusic;
    sf::Texture appleTexture, snakeHeadTexture, snakeBodyTexture, wallTexture;
    sf::Sprite appleSprite, snakeHeadSprite, snakeBodySprite, wallSprite;

	void loadResources() {
        assert(font.loadFromFile("Resources/Fonts/PoetsenOne-Regular.ttf"));

        assert(eatSoundBuffer.loadFromFile("Resources/Sounds/AppleEatSound.wav"));
        assert(crashSoundBuffer.loadFromFile("Resources/Sounds/GameOverSound.wav"));
        assert(hoverSoundBuffer.loadFromFile("Resources/Sounds/MenuHoverSound.wav"));
        assert(startSoundBuffer.loadFromFile("Resources/Sounds/GameStartSound.wav"));

        eatSound.setBuffer(eatSoundBuffer);
        crashSound.setBuffer(crashSoundBuffer);
        hoverSound.setBuffer(hoverSoundBuffer);
        startSound.setBuffer(startSoundBuffer);


        assert(bgMusic.openFromFile("Resources/Sounds/BackgroundMusic.wav"));
        bgMusic.setLoop(true);

        assert(appleTexture.loadFromFile("Resources/Apple/Apple.png"));
        assert(wallTexture.loadFromFile("Resources/Border/BrickLightGrey.png"));
        assert(snakeHeadTexture.loadFromFile("Resources/Snake/SnakeHead.png"));
        assert(snakeBodyTexture.loadFromFile("Resources/Snake/SnakeBody.png"));
    }
}