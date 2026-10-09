#include "CarScene.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
float clamp01(float v) { return std::max(0.f, std::min(1.f, v)); }
bool loadAudio(sf::Music& music, const char* path, bool loop = false) {
    if (!music.openFromFile(path)) {
        std::cerr << "Missing or unsupported audio: " << path << '\n';
        return false;
    }
    music.setLoop(loop);
    music.setVolume(0.f);
    return true;
}
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

    insideCarLoaded = loadAudio(insideCar, "assets/audio/ambience/insidecar.mp3", true);
    carMusicLoaded = loadAudio(carMusic, "assets/audio/ambience/carmusic.mp3", true);
    djLoaded = loadAudio(dj, "assets/audio/soundfx/dj.mp3");
    organLoaded = loadAudio(carnivalOrgan, "assets/audio/music/carnivalorgan.mp3", true);
    staticLoaded = loadAudio(radioStatic, "assets/audio/soundfx/radiostatic.mp3", true);
    glitchLoaded = loadAudio(radioGlitch, "assets/audio/soundfx/radioglitch.mp3", true);
    calloutLoaded = loadAudio(morganCallout, "assets/audio/dialog/ellie/morgancallout.mp3");
    isThatYouLoaded = loadAudio(morganIsThatYou, "assets/audio/dialog/ellie/morganisthatyou.mp3");
    gaspLoaded = loadAudio(morganGasp, "assets/audio/dialog/morgan/morganintro/morgangasps.mp3");
    helloLoaded = loadAudio(morganHello, "assets/audio/dialog/morgan/morganintro/morganhello.mp3");
    // Morgan is the player: keep her voice anchored to the listener, not the car.
    morganGasp.setRelativeToListener(true);
    morganHello.setRelativeToListener(true);
    carStopLoaded = loadAudio(carStop, "assets/audio/soundfx/carstop.mp3");
    carShiftLoaded = loadAudio(carShift, "assets/audio/soundfx/carshift.mp3");
    carOffLoaded = loadAudio(carOff, "assets/audio/soundfx/caroff.mp3");
}

void CarScene::start() {
    elapsed = phaseTime = voiceGapTime = 0.f;
    phase = Phase::Waiting;
    calloutStarted = secondLineStarted = false;
    gaspStarted = helloStarted = false;
    insideCar.stop(); carMusic.stop(); dj.stop(); carnivalOrgan.stop();
    radioStatic.stop(); radioGlitch.stop(); morganCallout.stop(); morganIsThatYou.stop();
    morganGasp.stop(); morganHello.stop(); carStop.stop(); carShift.stop(); carOff.stop();
    insideCar.setVolume(0.f);
    carMusic.setVolume(0.f);
    carSprite.setPosition(basePosition);
    fadeOverlay.setFillColor(sf::Color::Black);
}

void CarScene::enterPhase(Phase next) {
    phase = next;
    phaseTime = 0.f;
    switch (next) {
        case Phase::DJ:
            if (djLoaded) { dj.setVolume(djVolume); dj.play(); }
            break;
        case Phase::Song:
            if (carMusicLoaded) { carMusic.setVolume(musicVolume); carMusic.play(); }
            break;
        case Phase::FirstStatic:
        case Phase::SecondStatic:
            radioStatic.stop();
            if (staticLoaded) { radioStatic.setVolume(staticVolume); radioStatic.play(); }
            break;
        case Phase::FirstGlitch:
            gaspStarted = false;
            radioStatic.stop();
            if (glitchLoaded) { radioGlitch.setVolume(glitchVolume); radioGlitch.play(); }
            if (organLoaded) { carnivalOrgan.setVolume(organVolume); carnivalOrgan.play(); }
            break;
        case Phase::SecondGlitch:
            radioStatic.stop();
            radioGlitch.stop();
            carnivalOrgan.stop();
            calloutStarted = secondLineStarted = false;
            helloStarted = false;
            voiceGapTime = 0.f;
            if (glitchLoaded) { radioGlitch.setVolume(glitchVolume); radioGlitch.play(); }
            break;
        case Phase::FirstClosingStatic:
        case Phase::SecondClosingStatic:
            radioGlitch.stop();
            carnivalOrgan.stop();
            radioStatic.stop();
            // After the second glitch, the radio song cuts out permanently.
            if (next == Phase::SecondClosingStatic) carMusic.stop();
            if (staticLoaded) { radioStatic.setVolume(staticVolume); radioStatic.play(); }
            break;
        case Phase::Recovery:
            radioStatic.stop();
            if (carMusicLoaded) carMusic.setVolume(musicVolume);
            break;
        case Phase::Recovered:
            radioStatic.stop();
            // Leave carMusic stopped; only the driving ambience remains.
            break;
        case Phase::CarStop:
            // Stop driving ambience BEFORE the car-stop sound.
            insideCar.stop();
            insideCar.setVolume(0.f);

            if (carStopLoaded) {
                carStop.setVolume(carEffectVolume);
                carStop.play();
            }
            break;
        case Phase::CarShift:
            if (carShiftLoaded) { carShift.setVolume(carEffectVolume); carShift.play(); }
            break;
        case Phase::CarOff:
            if (carOffLoaded) { carOff.setVolume(carEffectVolume); carOff.play(); }
            if (carMusicLoaded) carMusic.stop();
            break;
        case Phase::Finished:
            insideCar.stop();
            carMusic.stop();
            break;
        case Phase::Waiting: break;
    }
}

void CarScene::updateEllie() {
    if (!calloutStarted && phaseTime >= calloutDelayInSecondGlitch) {
        calloutStarted = true;
        if (calloutLoaded) { morganCallout.setVolume(calloutVolume); morganCallout.play(); }
    }
    if (calloutStarted && !secondLineStarted &&
        (!calloutLoaded || morganCallout.getStatus() == sf::SoundSource::Stopped) &&
        voiceGapTime >= gapBetweenEllieLines) {
        secondLineStarted = true;
        if (isThatYouLoaded) { morganIsThatYou.setVolume(isThatYouVolume); morganIsThatYou.play(); }
    }
    const bool linesFinished = secondLineStarted &&
        (!isThatYouLoaded || morganIsThatYou.getStatus() == sf::SoundSource::Stopped);
    if (linesFinished && phaseTime >= secondGlitchMinSeconds &&
        (!helloLoaded || (helloStarted && morganHello.getStatus() == sf::SoundSource::Stopped)))
        enterPhase(Phase::SecondClosingStatic);
}

void CarScene::update(float deltaTime) {
    const float dt = std::max(0.f, deltaTime);
    elapsed += dt;
    phaseTime += dt;

    // Preserve the existing gentle camera movement and two-second fade-in.
    const float dx = std::sin(elapsed * 8.f) * 1.2f + std::sin(elapsed * 13.f) * 0.5f;
    const float dy = std::sin(elapsed * 11.f) * 1.5f + std::sin(elapsed * 17.f) * 0.7f;
    if (phase == Phase::CarShift || phase == Phase::CarOff || phase == Phase::Finished)
        carSprite.setPosition(basePosition);
    else
        carSprite.setPosition(basePosition.x + dx, basePosition.y + dy);
    const float imageProgress = clamp01(elapsed / fadeDuration);
    fadeOverlay.setFillColor(sf::Color(0, 0, 0,
        static_cast<sf::Uint8>((1.f - imageProgress) * 255.f)));

    // Driving ambience is only allowed before the car-stop sequence.
    bool isDriving =
        phase != Phase::CarStop &&
        phase != Phase::CarShift &&
        phase != Phase::CarOff &&
        phase != Phase::Finished;

    if (isDriving && elapsed >= audioDelay && insideCarLoaded &&
        insideCar.getStatus() == sf::SoundSource::Stopped)
    {
        insideCar.play();
    }
    if (insideCarLoaded && elapsed >= audioDelay) {
        if (phase == Phase::CarOff)
            insideCar.setVolume(ambienceVolume * (1.f - clamp01(phaseTime / engineFadeSeconds)));
        else if (phase != Phase::Finished)
            insideCar.setVolume(ambienceVolume * clamp01((elapsed - audioDelay) / audioFadeDuration));
    }

    switch (phase) {
        case Phase::Waiting:
            if (elapsed >= audioDelay + audioFadeDuration) enterPhase(Phase::DJ);
            break;
        case Phase::DJ:
            if (!djLoaded || dj.getStatus() == sf::SoundSource::Stopped)
                enterPhase(Phase::Song);
            break;
        case Phase::Song:
            if (phaseTime >= songBeforeStaticSeconds) enterPhase(Phase::FirstStatic);
            break;
        case Phase::FirstStatic:
        case Phase::SecondStatic:
            if (carMusicLoaded)
                carMusic.setVolume(musicVolume * (1.f - 0.72f * clamp01(phaseTime / musicDipSeconds)));
            if (phaseTime >= openingStaticSeconds)
                enterPhase(phase == Phase::FirstStatic ? Phase::FirstGlitch : Phase::SecondGlitch);
            break;
        case Phase::FirstGlitch:
            if (!gaspStarted && phaseTime >= gaspDelayAfterOrgan) {
                gaspStarted = true;
                if (gaspLoaded) { morganGasp.setVolume(morganGaspVolume); morganGasp.play(); }
            }
            if (carMusicLoaded) carMusic.setVolume(musicVolume * 0.28f);
            if (phaseTime >= firstGlitchSeconds) enterPhase(Phase::FirstClosingStatic);
            break;
        case Phase::SecondGlitch:
            if (!helloStarted && phaseTime >= helloDelayInSecondGlitch) {
                helloStarted = true;
                if (helloLoaded) { morganHello.setVolume(morganHelloVolume); morganHello.play(); }
            }
            if (carMusicLoaded) carMusic.setVolume(musicVolume * 0.28f);
            if (calloutStarted && !secondLineStarted &&
                (!calloutLoaded || morganCallout.getStatus() == sf::SoundSource::Stopped))
                voiceGapTime += dt;
            updateEllie();
            break;
        case Phase::FirstClosingStatic:
        case Phase::SecondClosingStatic:
            // Restore the song after the first glitch only.
            if (phase == Phase::FirstClosingStatic && carMusicLoaded)
                carMusic.setVolume(musicVolume * (0.28f + 0.72f *
                    clamp01((phaseTime - (closingStaticSeconds - musicRecoverySeconds)) / musicRecoverySeconds)));
            if (phaseTime >= closingStaticSeconds)
                enterPhase(phase == Phase::FirstClosingStatic ? Phase::Recovery : Phase::Recovered);
            break;
        case Phase::Recovery:
            if (phaseTime >= recoverySeconds) enterPhase(Phase::SecondStatic);
            break;
        case Phase::Recovered:
            if (phaseTime >= pauseBeforeStopping) enterPhase(Phase::CarStop);
            break;
        case Phase::CarStop:
            if (!carStopLoaded || carStop.getStatus() == sf::SoundSource::Stopped)
                enterPhase(Phase::CarShift);
            break;
        case Phase::CarShift:
            if (!carShiftLoaded || carShift.getStatus() == sf::SoundSource::Stopped)
                enterPhase(Phase::CarOff);
            break;
        case Phase::CarOff:
            if ((!carOffLoaded || carOff.getStatus() == sf::SoundSource::Stopped) &&
                phaseTime >= engineFadeSeconds)
                enterPhase(Phase::Finished);
            break;
        case Phase::Finished: break;
    }
}

void CarScene::draw(sf::RenderWindow& window) const {
    window.draw(carSprite);
    window.draw(fadeOverlay);
}
