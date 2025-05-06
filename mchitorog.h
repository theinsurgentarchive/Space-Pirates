//
//program: mchitorog.h
//author: Mihail Chitorog
//

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

namespace ecs {
    class Entity;  
}

extern const ecs::Entity* player;
extern ecs::Entity* planetPtr;

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

// Enum for tracking player direction
enum PlayerDirection {
    DIR_DOWN,
    DIR_UP,
    DIR_LEFT,
    DIR_RIGHT
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

struct FootstepState {
    bool isWalking{false};
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFootstepTime{};
    std::chrono::milliseconds footstepInterval{400}; 
    bool isSoundPlaying{false};
    std::chrono::milliseconds soundDuration{400}; 
};

extern FootstepState g_footstepState;
void updateFootstepSounds();

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

// Function to update player sprites when moving
void updatePlayerMovementSprite(const ecs::Entity* player, PlayerDirection direction);

// Function to update player sprites when stopping
void updatePlayerIdleSprite(const ecs::Entity* player);

// Function to initialize player sprites
void initializePlayerSprites();

// Function to detect movement keys and update sprites
void handlePlayerMovementInput(int key, const ecs::Entity* player);

// Function to handle key release for player
void handlePlayerKeyRelease(const ecs::Entity* player);
