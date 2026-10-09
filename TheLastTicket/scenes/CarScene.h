#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class CarScene {
public:
    explicit CarScene(sf::Vector2u windowSize);
    void start();
    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

private:
    sf::Texture carTexture;
    sf::Sprite carSprite;
    sf::RectangleShape fadeOverlay;
    sf::Vector2f basePosition;
    sf::Music insideCar;
    sf::Music carMusic;
    bool insideCarLoaded = false;
    bool carMusicLoaded = false;
    bool audioStarted = false;
    float elapsed = 0.f;
    static constexpr float fadeDuration = 2.f;
    static constexpr float audioDelay = 1.f;
    static constexpr float audioFadeDuration = 1.5f;
    static constexpr float ambienceVolume = 65.f;
    static constexpr float musicVolume = 28.f;
};
