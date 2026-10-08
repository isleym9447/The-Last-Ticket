#include "IntroSequence.h"
#include <algorithm>
#include <stdexcept>

namespace {
float clamp01(float value) { return std::max(0.f, std::min(1.f, value)); }
}

IntroSequence::IntroSequence(sf::Vector2u windowSize) {
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        throw std::runtime_error("Could not load assets/fonts/arial.ttf");
    const float cx = windowSize.x / 2.f;
    const float cy = windowSize.y / 2.f;
    warningTitle.setFont(font);
    warningTitle.setString("WARNING");
    warningTitle.setCharacterSize(52);
    warningTitle.setFillColor(sf::Color(215, 177, 125));
    centerText(warningTitle, cx, cy - 170.f);

    warningBody.setFont(font);
    warningBody.setString("This game contains sudden jumpscares, scary imagery,\nintense audio, and themes of psychological horror.");
    warningBody.setCharacterSize(23);
    warningBody.setLineSpacing(1.5f);
    warningBody.setFillColor(sf::Color(230, 220, 215));
    centerText(warningBody, cx, cy - 45.f);

    warningAdvice.setFont(font);
    warningAdvice.setString("Player discretion is advised.");
    warningAdvice.setCharacterSize(23);
    warningAdvice.setFillColor(sf::Color(230, 220, 215));
    centerText(warningAdvice, cx, cy + 70.f);

    continuePrompt.setFont(font);
    continuePrompt.setString("Press any key to continue.");
    continuePrompt.setCharacterSize(19);
    continuePrompt.setFillColor(sf::Color(165, 145, 140));
    centerText(continuePrompt, cx, cy + 190.f);

    omenFirst.setFont(font);
    omenFirst.setString("Admission is free, leaving costs");
    omenFirst.setCharacterSize(29);
    omenFirst.setFillColor(sf::Color(230, 220, 215));
    centerText(omenFirst, cx, cy - 20.f);

    omenLast.setFont(font);
    omenLast.setString("everything.");
    omenLast.setCharacterSize(32);
    omenLast.setFillColor(sf::Color(215, 177, 125));
    centerText(omenLast, cx, cy + 35.f);
    start();
}

void IntroSequence::centerText(sf::Text& text, float x, float y) {
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    text.setPosition(x, y);
}

void IntroSequence::setAlpha(sf::Text& text, float alpha) const {
    sf::Color c = text.getFillColor();
    c.a = static_cast<sf::Uint8>(clamp01(alpha) * 255.f);
    text.setFillColor(c);
}

void IntroSequence::start() {
    state = State::WarningFadeIn;
    timer = 0.f;
    opacity = 0.f;
    lastWordOpacity = 0.f;
}

void IntroSequence::handleEvent(const sf::Event& event) {
    if (state == State::WarningWaiting && event.type == sf::Event::KeyPressed) {
        state = State::WarningFadeOut;
        timer = 0.f;
    }
}

void IntroSequence::update(float deltaTime) {
    timer += deltaTime;
    switch (state) {
        case State::WarningFadeIn:
            opacity = clamp01(timer / fadeDuration);
            if (timer >= fadeDuration) {
                opacity = 1.f;
                timer = 0.f;
                state = State::WarningWaiting;
            }
            break;
        case State::WarningWaiting:
            opacity = 1.f;
            break;
        case State::WarningFadeOut:
            opacity = 1.f - clamp01(timer / fadeDuration);
            if (timer >= fadeDuration) {
                opacity = 0.f;
                timer = 0.f;
                state = State::OmenFadeIn;
            }
            break;
        case State::OmenFadeIn:
            opacity = clamp01(timer / fadeDuration);
            lastWordOpacity = opacity;
            if (timer >= fadeDuration) {
                opacity = 1.f;
                lastWordOpacity = 1.f;
                timer = 0.f;
                state = State::OmenHold;
            }
            break;
        case State::OmenHold:
            if (timer >= omenHoldDuration) {
                timer = 0.f;
                state = State::OmenFadeOut;
            }
            break;
        case State::OmenFadeOut:
            opacity = 1.f - clamp01(timer / omenFadeDuration);
            lastWordOpacity = 1.f - clamp01(timer / lastWordFadeDuration);
            if (timer >= lastWordFadeDuration) {
                opacity = 0.f;
                lastWordOpacity = 0.f;
                state = State::Finished;
            }
            break;
        case State::Finished:
            break;
    }
}

void IntroSequence::draw(sf::RenderWindow& window) const {
    if (state == State::Finished) return;
    window.clear(sf::Color(8, 5, 8));
    if (state == State::WarningFadeIn || state == State::WarningWaiting || state == State::WarningFadeOut) {
        sf::Text title = warningTitle;
        sf::Text body = warningBody;
        sf::Text advice = warningAdvice;
        sf::Text prompt = continuePrompt;
        setAlpha(title, opacity);
        setAlpha(body, opacity);
        setAlpha(advice, opacity);
        setAlpha(prompt, opacity);
        window.draw(title);
        window.draw(body);
        window.draw(advice);
        window.draw(prompt);
    } else {
        sf::Text first = omenFirst;
        sf::Text last = omenLast;
        setAlpha(first, opacity);
        setAlpha(last, lastWordOpacity);
        window.draw(first);
        window.draw(last);
    }
}

bool IntroSequence::isFinished() const { return state == State::Finished; }
