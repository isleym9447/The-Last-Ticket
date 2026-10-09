#include "CarScene.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
float clamp01(float v) { return std::max(0.f, std::min(1.f, v)); }
}

CarScene::CarScene(sf::Vector2u windowSize) {
    if (!carTexture.loadFromFile("assets/images/backgrounds/morgan'scarinterior.png"))
        throw std::runtime_error("Could not load Morgan's car interior image");
    carTexture.setSmooth(true);
    carSprite.setTexture(carTexture);

    const sf::Vector2u imageSize = carTexture.getSize();
    const float scaleX = static_cast<float>(windowSize.x) / imageSize.x;
    const float scaleY = static_cast<float>(windowSize.y) / imageSize.y;
    const float scale = std::max(scaleX, scaleY) * 1.02f;
    carSprite.setScale(scale, scale);
    const sf::FloatRect bounds = carSprite.getGlobalBounds();
    basePosition = sf::Vector2f((windowSize.x - bounds.width) / 2.f,
                                (windowSize.y - bounds.height) / 2.f);
    carSprite.setPosition(basePosition);

    fadeOverlay.setSize(sf::Vector2f(static_cast<float>(windowSize.x),
                                     static_cast<float>(windowSize.y)));
    fadeOverlay.setFillColor(sf::Color::Black);

    // Only this scene loads audio. Missing files produce a warning, not a crash.
    insideCarLoaded = insideCar.openFromFile("assets/audio/ambience/insidecar.mp3");
    carMusicLoaded = carMusic.openFromFile("assets/audio/ambience/carmusic.mp3");
    if (!insideCarLoaded) std::cerr << "Missing or unsupported: assets/audio/ambience/insidecar.mp3\n";
    if (!carMusicLoaded) std::cerr << "Missing or unsupported: assets/audio/ambience/carmusic.mp3\n";
    if (insideCarLoaded) { insideCar.setLoop(true); insideCar.setVolume(0.f); }
    if (carMusicLoaded) { carMusic.setLoop(true); carMusic.setVolume(0.f); }
}

void CarScene::start() {
    elapsed = 0.f;
    audioStarted = false;
    if (insideCarLoaded) { insideCar.stop(); insideCar.setVolume(0.f); }
    if (carMusicLoaded) { carMusic.stop(); carMusic.setVolume(0.f); }
    carSprite.setPosition(basePosition);
    fadeOverlay.setFillColor(sf::Color::Black);
}

void CarScene::update(float deltaTime) {
    elapsed += std::max(0.f, deltaTime);

    // Gentle driving movement (independent of frame rate).
    const float dx = std::sin(elapsed * 8.f) * 1.2f + std::sin(elapsed * 13.f) * 0.5f;
    const float dy = std::sin(elapsed * 11.f) * 1.5f + std::sin(elapsed * 17.f) * 0.7f;
    carSprite.setPosition(basePosition.x + dx, basePosition.y + dy);

    const float imageProgress = clamp01(elapsed / fadeDuration);
    fadeOverlay.setFillColor(sf::Color(0, 0, 0,
        static_cast<sf::Uint8>((1.f - imageProgress) * 255.f)));

    if (!audioStarted && elapsed >= audioDelay) {
        audioStarted = true;
        if (insideCarLoaded) insideCar.play();
        if (carMusicLoaded) carMusic.play();
    }
    if (audioStarted) {
        const float volumeProgress = clamp01((elapsed - audioDelay) / audioFadeDuration);
        if (insideCarLoaded) insideCar.setVolume(ambienceVolume * volumeProgress);
        if (carMusicLoaded) carMusic.setVolume(musicVolume * volumeProgress);
    }
}

void CarScene::draw(sf::RenderWindow& window) const {
    window.draw(carSprite);
    window.draw(fadeOverlay);
}
