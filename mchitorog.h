#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "balrowhany.h"
#include "jlo.h"

// For OpenAL audio system
#ifdef USE_OPENAL_SOUND
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#endif

// Sound effect types
enum SoundType {
    MENU_CLICK,
    PLAYER_SHOOT,
    PLAYER_HIT,
    ENEMY_HIT,
    PLAYER_DEATH,
    GAME_WIN,
    GAME_LOSE,
    POWER_UP,
    FOOTSTEP_GRASS,   
    FOOTSTEP_SNOW,
    FOOTSTEP_LAVA
};

// Music types for different game states
enum MusicType {
    MENU_MUSIC,
    MENU_PAUSE_MUSIC,
    GAME_MUSIC,
    SPACE_MUSIC,
    BOSS_MUSIC,
    VICTORY_MUSIC,
    GAME_OVER_MUSIC
};

// Constants for menu actions
const int MENU_ACTION_NONE = 0;
const int MENU_ACTION_EXIT = 1;
const int MENU_ACTION_NAVIGATE = 2;
const int MENU_ACTION_SELECT = 3;

// Pause menu actions
const int PAUSE_ACTION_NONE = 0;
const int PAUSE_ACTION_RESUME = 30;
const int PAUSE_ACTION_CONTROLS = 21;
const int PAUSE_ACTION_MAINMENU = 22;

class AudioManager {
private:
    #ifdef USE_OPENAL_SOUND
    // OpenAL variables
    ALCdevice* device;
    ALCcontext* context;
    
    // Sound buffers and sources
    std::unordered_map<SoundType, ALuint> soundBuffers;
    std::unordered_map<SoundType, ALuint> soundSources;
    
    // Music buffers and sources
    std::unordered_map<MusicType, ALuint> musicBuffers;
    std::unordered_map<MusicType, ALuint> musicSources;
    #endif
    
    // Audio status
    bool audioInitialized;
    bool musicEnabled;
    bool soundEnabled;
    int musicVolume;  // 0-100
    int soundVolume;  // 0-100
    MusicType currentMusic;
    
    // Constructor is private for singleton pattern
    AudioManager();
    
    // Helper methods
    bool loadSound(SoundType type, const std::string& filename);
    bool loadMusic(MusicType type, const std::string& filename);
    
public:
    // Static instance pointer
    static AudioManager* instance;
    
    // Destructor
    ~AudioManager();
    
    // Get singleton instance
    static AudioManager* getInstance();
    
    // Initialize and shutdown audio system
    bool initAudio();
    void shutdown();
    
    // Load resources
    bool loadSoundEffects();
    bool loadMusic();
    
    // Sound effect controls
    void playSound(SoundType sound);
    void stopSound(SoundType sound);
    void stopAllSounds();
    
    // Music controls
    void playMusic(MusicType music);
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    
    // Volume controls
    void setMusicVolume(int volume);
    void setSoundVolume(int volume);
    int getMusicVolume() const;
    int getSoundVolume() const;
    
    // Toggle audio
    void toggleMusic();
    void toggleSound();
    bool isMusicEnabled() const;
    bool isSoundEnabled() const;
    
    // Update function to be called each frame
    void update();
    MusicType getCurrentMusic() const;
    void setCurrentMusic(MusicType music) { currentMusic = music; }
};

// Global accessor function
AudioManager* getAudioManager();

// Helper functions
void initAudioSystem();
void shutdownAudioSystem();
void playGameSound(SoundType sound);
void playGameMusic(MusicType music);
void updateAudioState(GameState state);
void changeGameState(GameState& currentState, GameState newState);
void render_credits_screen(int xres, int yres, GLuint menuBackgroundTexture);
void resetGLState(int xres, int yres);
void render_title_logo(int xres, int yres, GLuint titleTexture);
void playFootstepSound();
// Pause menu functions
void render_pause_menu(int xres, int yres, int selected_option);
int handle_pause_keys(int key, GameState &state, GameState &previous_state, int &selected_option, MusicType previous_music);
void render_pause_controls_screen(int xres, int yres);
