#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "balrowhany.h"

// Sound effect types
enum SoundType {
    MENU_CLICK,
    PLAYER_SHOOT,
    PLAYER_HIT,
    ENEMY_HIT,
    PLAYER_DEATH,
    GAME_WIN,
    GAME_LOSE,
    POWER_UP
};

// Music types for different game states
enum MusicType {
    MENU_MUSIC,
    GAME_MUSIC,
    BOSS_MUSIC,
    VICTORY_MUSIC,
    GAME_OVER_MUSIC
};

// Simple fallback audio manager that doesn't actually play sounds
// but tracks the sound state so the game can integrate sound logic
class AudioManager {
private:
    // Audio status
    bool audioInitialized;
    bool musicEnabled;
    bool soundEnabled;
    int musicVolume;  // 0-100
    int soundVolume;  // 0-100
    MusicType currentMusic;
    
    // Constructor is private for singleton pattern
    AudioManager() 
        : audioInitialized(false),
          musicEnabled(true),
          soundEnabled(true),
          musicVolume(100),
          soundVolume(100),
          currentMusic(MENU_MUSIC) {
    }
    
public:
    // Static instance pointer - moved to public for access
    static AudioManager* instance;
    
    // Destructor
    ~AudioManager() {
        shutdown();
    }
    
    // Get singleton instance
    static AudioManager* getInstance() {
        if (instance == nullptr) {
            instance = new AudioManager();
        }
        return instance;
    }
    
    // Initialize and shutdown audio system
    bool initAudio() {
        // Always succeed in fallback mode
        audioInitialized = true;
        return true;
    }
    
    void shutdown() {
        audioInitialized = false;
    }
    
    // Load resources (no-op in fallback mode)
    bool loadSoundEffects() {
        return true;
    }
    
    // Sound effect controls (no-op but log the actions)
    void playSound(SoundType sound) {
        if (!audioInitialized || !soundEnabled) return;
        
        // Just log the sound that would be played
        #ifdef DEBUG
        std::cout << "Playing sound: " << static_cast<int>(sound) << std::endl;
        #endif
    }
    
    void stopAllSounds() {
        // No-op in fallback mode
    }
    
    // Music controls (no-op but log the actions)
    void playMusic(MusicType music) {
        if (!audioInitialized || !musicEnabled) return;
        
        currentMusic = music;
        
        // Just log the music that would be played
        #ifdef DEBUG
        std::cout << "Playing music: " << static_cast<int>(music) << std::endl;
        #endif
    }
    
    void stopMusic() {
        // No-op in fallback mode
        #ifdef DEBUG
        std::cout << "Stopping music" << std::endl;
        #endif
    }
    
    // Volume controls
    void setMusicVolume(int volume) {
        // Clamp volume to valid range
        if (volume < 0) volume = 0;
        if (volume > 100) volume = 100;
        
        musicVolume = volume;
    }
    
    void setSoundVolume(int volume) {
        // Clamp volume to valid range
        if (volume < 0) volume = 0;
        if (volume > 100) volume = 100;
        
        soundVolume = volume;
    }
    
    int getMusicVolume() const {
        return musicVolume;
    }
    
    int getSoundVolume() const {
        return soundVolume;
    }
    
    // Toggle audio
    void toggleMusic() {
        musicEnabled = !musicEnabled;
        
        if (!musicEnabled) {
            stopMusic();
        } else {
            playMusic(currentMusic);
        }
    }
    
    void toggleSound() {
        soundEnabled = !soundEnabled;
    }
    
    bool isMusicEnabled() const {
        return musicEnabled;
    }
    
    bool isSoundEnabled() const {
        return soundEnabled;
    }
    
    // Update function to be called each frame (no-op in fallback mode)
    void update() {
    }
};

// Global accessor function
AudioManager* getAudioManager();

// Helper functions
void initAudioSystem();
void shutdownAudioSystem();
void playGameSound(SoundType sound);
void playGameMusic(MusicType music);
void updateAudioState(GameState state);
