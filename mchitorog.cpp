#include "mchitorog.h"

// Initialize static instance pointer
AudioManager* AudioManager::instance = nullptr;

// Global accessor function
AudioManager* getAudioManager() {
    return AudioManager::getInstance();
}

// Initialize audio system
void initAudioSystem() {
    AudioManager::getInstance()->initAudio();
    AudioManager::getInstance()->loadSoundEffects();
}

// Shutdown audio system
void shutdownAudioSystem() {
    if (AudioManager::instance != nullptr) {
        AudioManager::getInstance()->shutdown();
    }
}

// Play sound wrapper
void playGameSound(SoundType sound) {
    AudioManager::getInstance()->playSound(sound);
}

// Play music wrapper
void playGameMusic(MusicType music) {
    AudioManager::getInstance()->playMusic(music);
}

// Update audio based on game state
void updateAudioState(GameState state) {
    switch (state) {
        case MENU:
            playGameMusic(MENU_MUSIC);
            break;
        case PLAYING:
            playGameMusic(GAME_MUSIC);
            break;
        case CONTROLS:
            // Use the same music as the menu
            playGameMusic(MENU_MUSIC);
            break;
        case EXIT:
            // Stop all audio
            AudioManager::getInstance()->stopMusic();
            AudioManager::getInstance()->stopAllSounds();
            break;
        default:
            break;
    }
}
