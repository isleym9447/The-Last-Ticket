
#pragma once

#include <SFML/Graphics.hpp>

class IntroSequence
{
public:
    enum class State
    {
        WarningFadeIn,
        WarningWaiting,
        WarningFadeOut,
        OmenFadeIn,
        OmenHold,
        OmenFadeOut,
        Finished
    };

    explicit IntroSequence(sf::Vector2u windowSize);

    void start();
    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

    bool isFinished() const;

private:
    sf::Font font;

    sf::Text warningTitle;
    sf::Text warningBody;
    sf::Text warningAdvice;
    sf::Text continuePrompt;

    sf::Text omenFirst;
    sf::Text omenLast;

    State state = State::WarningFadeIn;

    float timer = 0.f;
    float opacity = 0.f;
    float lastWordOpacity = 0.f;

    static constexpr float fadeDuration = 1.5f;
    static constexpr float omenHoldDuration = 2.5f;
    static constexpr float omenFadeDuration = 2.f;
    static constexpr float lastWordFadeDuration = 3.5f;

    void centerText(sf::Text& text, float x, float y);
    void setAlpha(sf::Text& text, float alpha) const;
};
