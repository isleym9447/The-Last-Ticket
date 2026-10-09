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
    enum class Phase {
        Waiting, DJ, Song, FirstStatic, FirstGlitch, FirstClosingStatic,
        Recovery, SecondStatic, SecondGlitch, SecondClosingStatic, Recovered, CarStop, CarShift, CarOff, Finished
    };
    void enterPhase(Phase next);
    void updateEllie();

    sf::Texture carTexture;
    sf::Sprite carSprite;
    sf::RectangleShape fadeOverlay;
    sf::Vector2f basePosition;

    sf::Music insideCar, carMusic, dj, carnivalOrgan, radioStatic, radioGlitch;
    sf::Music morganCallout, morganIsThatYou;
    sf::Music morganGasp, morganHello, carStop, carShift, carOff;
    bool insideCarLoaded = false, carMusicLoaded = false, djLoaded = false;
    bool organLoaded = false, staticLoaded = false, glitchLoaded = false;
    bool calloutLoaded = false, isThatYouLoaded = false;
    bool calloutStarted = false, secondLineStarted = false;
    bool gaspStarted = false, helloStarted = false;
    bool gaspLoaded = false, helloLoaded = false;
    bool carStopLoaded = false, carShiftLoaded = false, carOffLoaded = false;
    float elapsed = 0.f, phaseTime = 0.f, voiceGapTime = 0.f;
    Phase phase = Phase::Waiting;

    static constexpr float fadeDuration = 2.f;
    static constexpr float audioDelay = 1.f;
    static constexpr float audioFadeDuration = 1.5f;
    static constexpr float ambienceVolume = 25.f;
    static constexpr float musicVolume = 20.f;
    static constexpr float djVolume = 35.f;
    static constexpr float organVolume = 30.f;
    static constexpr float staticVolume = 30.f;
    static constexpr float glitchVolume = 30.f;
    static constexpr float calloutVolume = 7.f;
    static constexpr float isThatYouVolume = 7.f;
    static constexpr float morganGaspVolume = 80.f;
    static constexpr float morganHelloVolume = 100.f;
    static constexpr float carEffectVolume = 70.f;
    static constexpr float gaspDelayAfterOrgan = 0.5f;
    static constexpr float helloDelayInSecondGlitch = 2.6f;
    static constexpr float pauseBeforeStopping = 1.5f;
    static constexpr float engineFadeSeconds = 1.5f;

    // Adjust these to fine-tune the diagram's timing (seconds).
    static constexpr float songBeforeStaticSeconds = 8.f;
    static constexpr float openingStaticSeconds = 1.0f;
    static constexpr float firstGlitchSeconds = 4.0f;
    static constexpr float closingStaticSeconds = 1.0f;
    static constexpr float recoverySeconds = 5.0f;
    static constexpr float secondGlitchMinSeconds = 5.0f;
    static constexpr float calloutDelayInSecondGlitch = 1.8f;
    static constexpr float gapBetweenEllieLines = 0.4f;
    static constexpr float musicDipSeconds = 0.5f;
    static constexpr float musicRecoverySeconds = 0.65f;
};
