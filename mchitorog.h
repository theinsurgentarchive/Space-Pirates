#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <vector>
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

class AudioManager {
private:
    static AudioManager* instance;
    
    // Maps to store loaded sound effects and music
    std::unordered_map<SoundType, Mix_Chunk*> soundEffects;
    std::unordered_map<MusicType, Mix_Music*> musicTracks;
    
    // Current music track being played
    MusicType currentMusic;
    
    // Volume levels
    int musicVolume;
    int soundVolume;
    
    // Audio status
    bool audioInitialized;
    bool musicEnabled;
    bool soundEnabled;
    
    // Constructor is private for singleton pattern
    AudioManager();
    
public:
    // Destructor
    ~AudioManager();
    
    // Get singleton instance
    static AudioManager* getInstance();
    
    // Initialize and shutdown audio system
    bool initAudio();
    void shutdown();
    
    // Load resources
    bool loadSoundEffects();
    bool loadMusicTracks();
    
    // Sound effect controls
    void playSound(SoundType sound);
    void stopAllSounds();
    
    // Music controls
    void playMusic(MusicType music, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void fadeOutMusic(int ms = 1000);
    
    // Volume controls
    void setMusicVolume(int volume); // 0-128
    void setSoundVolume(int volume); // 0-128
    int getMusicVolume() const;
    int getSoundVolume() const;
    
    // Toggle audio
    void toggleMusic();
    void toggleSound();
    bool isMusicEnabled() const;
    bool isSoundEnabled() const;
    
    // Update function to be called each frame
    void update();
};

// Global accessor function
AudioManager* getAudioManager();

// Helper functions
void initAudioSystem();
void shutdownAudioSystem();
void playGameSound(SoundType sound);
void playGameMusic(MusicType music);
void updateAudioState(GameState state);
