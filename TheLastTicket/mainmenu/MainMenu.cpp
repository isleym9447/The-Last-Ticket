#include "MainMenu.h"
#include <algorithm>
#include <stdexcept>

namespace {
    constexpr float FadeButtonsSeconds = 0.60f;
    constexpr float FlickerSeconds = 0.85f;
    constexpr float LitSeconds = 1.10f;
    constexpr float FadeBlackSeconds = 0.80f;
}

MainMenu::MainMenu(sf::Vector2u windowSize) : size(windowSize) {
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        throw std::runtime_error("Could not load assets/fonts/arial.ttf");

    if (!logoOffTexture.loadFromFile("assets/images/logos/logooff.png") ||
        !logoOneBulbTexture.loadFromFile("assets/images/logos/logoononebulb.png") ||
        !logoOnTexture.loadFromFile("assets/images/logos/logoon.png"))
        throw std::runtime_error("Could not load one or more menu logo images.");

    logoOffTexture.setSmooth(true);
    logoOneBulbTexture.setSmooth(true);
    logoOnTexture.setSmooth(true);
    setLogo(logoOffTexture);

    setupButtons();
    blackOverlay.setSize(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    blackOverlay.setFillColor(sf::Color(0, 0, 0, 0));

}

void MainMenu::centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(x, y);
}

void MainMenu::setLogo(const sf::Texture& texture) {
    // Keep the sprite's on-screen size and center identical for all three images.
    logoSprite.setTexture(texture, true);
    const sf::Vector2u imageSize = texture.getSize();
    logoSprite.setOrigin(imageSize.x / 2.f, imageSize.y / 2.f);
    const float maxWidth = static_cast<float>(size.x) * 0.62f;
    const float maxHeight = static_cast<float>(size.y) * 0.34f;
    const float scale = std::min(maxWidth / imageSize.x, maxHeight / imageSize.y);
    logoSprite.setScale(scale, scale);
    logoSprite.setPosition(size.x / 2.f, size.y * 0.23f);
}

void MainMenu::setupButtons() {
    const std::array<std::string, 4> labels = {"START", "RESUME", "CREDITS", "EXIT GAME"};
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setFont(font);
        buttons[i].setString(labels[i]);
        buttons[i].setCharacterSize(28);
        buttons[i].setFillColor(sf::Color(220, 198, 163));
        centerText(buttons[i], size.x / 2.f, size.y * 0.48f + static_cast<float>(i) * 65.f);
    }
}

void MainMenu::setButtonsAlpha(sf::Uint8 alpha) {
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        sf::Color color = buttons[i].getFillColor();
        color.a = alpha;
        buttons[i].setFillColor(color);
    }
}

void MainMenu::enterPhase(Phase next) {
    phase = next;
    phaseClock.restart();
    if (next == Phase::Flicker) setLogo(logoOffTexture);
    if (next == Phase::Lit) {
        setLogo(logoOnTexture);
    }
    if (next == Phase::FadeBlack) blackOverlay.setFillColor(sf::Color(0, 0, 0, 0));
}

void MainMenu::startTransition(Action action) {
    if (phase != Phase::Idle) return;
    pendingAction = action;
    enterPhase(Phase::FadeButtons);
}

void MainMenu::update(const sf::RenderWindow& window) {
    if (phase == Phase::Idle) {
        if (showingCredits) return;
        const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            if (i == 1 && !hasSaveFile) {
                buttons[i].setFillColor(sf::Color(90, 80, 85));
            } else {
                const bool hover = buttons[i].getGlobalBounds().contains(mouse);
                buttons[i].setFillColor(hover ? sf::Color(255, 221, 160) : sf::Color(220, 198, 163));
            }
        }
        return;
    }

    const float t = phaseClock.getElapsedTime().asSeconds();
    switch (phase) {
        case Phase::FadeButtons: {
            const float p = std::min(t / FadeButtonsSeconds, 1.f);
            setButtonsAlpha(static_cast<sf::Uint8>(255.f * (1.f - p)));
            if (p >= 1.f) enterPhase(Phase::Flicker);
            break;
        }
        case Phase::Flicker:
            // Three uneven sparks from the single bulb, then darkness.
            if ((t >= 0.12f && t < 0.20f) ||
                (t >= 0.34f && t < 0.42f) ||
                (t >= 0.54f && t < 0.72f))
                setLogo(logoOneBulbTexture);
            else
                setLogo(logoOffTexture);
            if (t >= FlickerSeconds) enterPhase(Phase::Lit);
            break;
        case Phase::Lit:
            if (t >= LitSeconds) enterPhase(Phase::FadeBlack);
            break;
        case Phase::FadeBlack: {
            const float p = std::min(t / FadeBlackSeconds, 1.f);
            blackOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(255.f * p)));
            if (p >= 1.f) enterPhase(Phase::Complete);
            break;
        }
        case Phase::Idle:
        case Phase::Complete:
            break;
    }
}

MainMenu::Action MainMenu::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (phase != Phase::Idle) return Action::None;

    if (showingCredits) {
        if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) ||
            (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left))
            showingCredits = false;
        return Action::None;
    }
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return Action::None;

    const sf::Vector2f mouse = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (!buttons[i].getGlobalBounds().contains(mouse)) continue;
        switch (i) {
            case 0: startTransition(Action::Start); return Action::None;
            case 1:
                if (hasSaveFile) startTransition(Action::Resume);
                return Action::None;
            case 2: showingCredits = true; return Action::None;
            case 3: return Action::Exit;
        }
    }
    return Action::None;
}

void MainMenu::draw(sf::RenderWindow& window) const {
    if (showingCredits && phase == Phase::Idle) {
        sf::Text credits;
        credits.setFont(font);
        credits.setString("THE LAST TICKET\n\nCreated by The Last Ticket Team\n\nClick anywhere or press ESC to return");
        credits.setCharacterSize(26);
        credits.setFillColor(sf::Color(214, 183, 123));
        credits.setLineSpacing(1.5f);
        centerText(credits, size.x / 2.f, size.y / 2.f);
        window.draw(credits);
        return;
    }
    window.draw(logoSprite);
    for (const auto& button : buttons) window.draw(button);
    window.draw(blackOverlay);
}

MainMenu::Action MainMenu::takeCompletedAction() {
    if (phase != Phase::Complete) return Action::None;
    const Action result = pendingAction;
    pendingAction = Action::None;
    return result;
}
