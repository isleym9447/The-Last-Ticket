#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include "mainmenu/MainMenu.h"
#include "intro/IntroSequence.h"
#include "scenes/CarScene.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "THE LAST TICKET", sf::Style::Close);
    window.setFramerateLimit(60);

    MainMenu mainMenu(window.getSize());
    IntroSequence introSequence(window.getSize());
    CarScene carScene(window.getSize());

    enum class GameScreen { MainMenu, Intro, Car };
    GameScreen currentScreen = GameScreen::MainMenu;
    sf::Clock clock;

    while (window.isOpen()) {
        const float deltaTime = std::min(clock.restart().asSeconds(), 0.1f);
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                continue;
            }
            if (currentScreen == GameScreen::MainMenu) {
                if (mainMenu.handleEvent(event, window) == MainMenu::Action::Exit)
                    window.close();
            } else if (currentScreen == GameScreen::Intro) {
                introSequence.handleEvent(event);
            }
        }
        if (!window.isOpen()) break;

        switch (currentScreen) {
            case GameScreen::MainMenu: {
                mainMenu.update(window);
                const MainMenu::Action completed = mainMenu.takeCompletedAction();
                if (completed == MainMenu::Action::Start) {
                    introSequence.start();
                    currentScreen = GameScreen::Intro;
                    std::cout << "Starting warning sequence...\n";
                } else if (completed == MainMenu::Action::Resume) {
                    std::cout << "Resume not implemented yet.\n";
                } else if (completed == MainMenu::Action::Exit) {
                    window.close();
                }
                break;
            }
            case GameScreen::Intro:
                introSequence.update(deltaTime);
                if (introSequence.isFinished()) {
                    carScene.start();
                    currentScreen = GameScreen::Car;
                    std::cout << "Morgan's car scene begins...\n";
                }
                break;
            case GameScreen::Car:
                carScene.update(deltaTime);
                break;
        }
        if (!window.isOpen()) break;

        window.clear(sf::Color(8, 5, 8));
        switch (currentScreen) {
            case GameScreen::MainMenu: mainMenu.draw(window); break;
            case GameScreen::Intro: introSequence.draw(window); break;
            case GameScreen::Car: carScene.draw(window); break;
        }
        window.display();
    }
    return 0;
}
