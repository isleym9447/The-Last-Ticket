#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <string>

class MainMenu {
public:
    enum class Action { None, Start, Resume, Exit };

    explicit MainMenu(sf::Vector2u windowSize);
    Action handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window) const;
    Action takeCompletedAction();

private:
    enum class Phase { Idle, FadeButtons, Flicker, Lit, FadeBlack, Complete };

    sf::Font font;
    sf::Texture logoOffTexture;
    sf::Texture logoOneBulbTexture;
    sf::Texture logoOnTexture;
    sf::Sprite logoSprite;
    std::array<sf::Text, 4> buttons;
    sf::RectangleShape blackOverlay;
    bool showingCredits = false;
    bool hasSaveFile = false;
    Phase phase = Phase::Idle;
    Action pendingAction = Action::None;
    sf::Clock phaseClock;
    sf::Music lightFlicker;
    sf::Music lightBuzz;
    sf::Vector2u size;

    static void centerText(sf::Text& text, float x, float y);
    void setupButtons();
    void startTransition(Action action);
    void enterPhase(Phase next);
    void setLogo(const sf::Texture& texture);
    void setButtonsAlpha(sf::Uint8 alpha);
};
