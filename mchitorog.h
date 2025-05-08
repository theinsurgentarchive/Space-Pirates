//
//program: mchitorog.h
//author: Mihail Chitorog
//

#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
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

class SpriteSheet;
extern std::unordered_map<std::string, std::shared_ptr<SpriteSheet>> ssheets;
extern const ecs::Entity* player;
extern ecs::Entity* planetPtr;

// Sound effect types
enum SoundType {
    MENU_CLICK,
    PLAYER_SHOOT,
    PLAYER_HIT,
    ENEMY_HIT,
    ENEMY_GRUNT,
    ENEMY_DEATH,
    PLAYER_DEATH,
    GAME_WIN,
    GAME_LOSE,
    POWER_UP,
    FOOTSTEP_GRASS,   
    FOOTSTEP_SNOW,
    FOOTSTEP_LAVA,
    PLAYER_SWORD_SWING,
    PLAYER_GUN_SHOT,
    PLAYER_ROCKET_LAUNCH,
    PLAYER_NEED_AMMO,
    PLAYER_NEED_BIGGER_WEAPON,
    PLAYER_WEAPON_SWITCH,
    PLAYER_HASTA_LA_VISTA,
    PLAYER_NOW_THATS_A_WEAPON
};

// Music types for different game states
enum MusicType {
    MENU_MUSIC,
    MENU_PAUSE_MUSIC,
    GAME_MUSIC,
    SPACE_MUSIC,
    BOSS_MUSIC,
    VICTORY_MUSIC,
    GAME_OVER_MUSIC,
    FOREST_MUSIC,
    DESERT_MUSIC,
    TAIGA_MUSIC,
    MEADOW_MUSIC,
    HELL_MUSIC
};

// Enum for tracking player direction
enum PlayerDirection {
    DIR_DOWN,
    DIR_UP,
    DIR_LEFT,
    DIR_RIGHT
};

// Weapon types
enum WeaponType {
    WEAPON_NONE,
    WEAPON_SWORD,
    WEAPON_HANDGUN,
    WEAPON_ROCKET
};

// Projectile types
enum ProjectileType {
    PROJECTILE_BULLET,
    PROJECTILE_ROCKET
};

// Weapon state tracking
struct WeaponState {
    WeaponType currentWeapon;
    int handgunAmmo;
    int rocketAmmo;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFireTime;
    std::chrono::milliseconds cooldownPeriod;
    bool canFire;
};

// Global weapon state
extern WeaponState g_weaponState;

// Projectile structure to track bullet properties
struct Projectile {
    bool active;
    float posX, posY;
    float velX, velY;
    float speed;
    float lifetime;
    float maxLifetime;
    ProjectileType type;
};

// Structure to track sword attack animations
struct SwordAttackState {
    bool isAttacking;
    std::chrono::time_point<std::chrono::high_resolution_clock> attackStartTime;
    std::chrono::milliseconds attackDuration;
};

extern SwordAttackState g_swordAttackState;

// Maximum number of projectiles that can exist simultaneously
#define MAX_PROJECTILES 5

// Global array to track active projectiles
extern Projectile projectiles[MAX_PROJECTILES];

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

// Structure to track footstep sounds and timing
struct FootstepState {
    bool isWalking{false};
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFootstepTime{};
    std::chrono::milliseconds footstepInterval{400}; 
    bool isSoundPlaying{false};
    std::chrono::milliseconds soundDuration{400}; 
};

// Enemy health and state tracking
struct EnemyState {
    int health;           // Current health
    int maxHealth;        // Maximum health
    bool isDead;          // Is the enemy dead
    bool isHit;           // Is the enemy currently in hit animation
    float hitTimer;       // Timer for hit animation
    float deathTimer;     // Timer for death animation
};

// Global enemy state (will be integrated with your entity system)
extern EnemyState g_enemyState;
extern FootstepState g_footstepState;

// Update footstep sounds based on player movement
void updateFootstepSounds();

// Audio Manager singleton class to handle all game audio
class AudioManager {
private:
#ifdef USE_OPENAL_SOUND
    // OpenAL variables
    ALCdevice* device;
    ALCcontext* context;

    // Sound buffers and sources
    std::unordered_map<SoundType, ALuint> soundBuffers;
    std::unordered_map<SoundType, ALuint> soundSources;

    // Track last time each sound was played to prevent stuttering
    std::unordered_map<SoundType, 
        std::chrono::time_point<std::chrono::high_resolution_clock>> 
        lastSoundPlayed;
    // Minimum time between playing the same sound (in milliseconds)
    std::chrono::milliseconds soundDebounceTime{100};

    // Custom gain multipliers for specific sound effects
    std::unordered_map<SoundType, float> soundGainMultipliers;

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

// Global accessor function for AudioManager singleton
AudioManager* getAudioManager();

// Audio system initialization and shutdown
void initAudioSystem();
void shutdownAudioSystem();
void playGameSound(SoundType sound);
void playGameMusic(MusicType music);
void updateAudioState(GameState state);
void changeGameState(GameState& currentState, GameState newState);

// UI rendering functions
void render_credits_screen(int xres, int yres, GLuint menuBackgroundTexture);
void resetGLState(int xres, int yres);
void render_title_logo(int xres, int yres, GLuint titleTexture);
void playFootstepSound();

// Pause menu functions
void render_pause_menu(int xres, int yres, int selected_option);
int handle_pause_keys(int key, GameState &state, GameState &previous_state, 
                     int &selected_option, MusicType previous_music);
void render_pause_controls_screen(int xres, int yres);

// Player movement and sprite functions
void updatePlayerMovementSprite(const ecs::Entity* player, 
                               PlayerDirection direction);
void updatePlayerIdleSprite(const ecs::Entity* player);
void initializePlayerSprites();
void handlePlayerMovementInput(int key, const ecs::Entity* player);
void handlePlayerKeyRelease(const ecs::Entity* player);

// Music functions
void playBiomeMusic(BiomeType biomeType);
MusicType getBiomeMusicType(BiomeType biomeType);

// Weapon system functions
void initializeWeaponSystem();
void loadWeaponSprites(std::unordered_map<std::string, 
                      std::shared_ptr<SpriteSheet>>& sheets);
void switchWeapon(WeaponType newWeapon, bool playSounds = true);
bool fireWeapon();
void updateWeaponSprites(const ecs::Entity* player);
void handleWeaponInput(int key, const ecs::Entity* player);
void renderAmmoUI(int xres, int yres);
void updateSwordAttackAnimation();

// Projectile system functions
void initializeProjectileSystem();
void createProjectile(float startX, float startY, float dirX, float dirY, 
                     ProjectileType type);
void updateProjectiles(float dt);
void renderProjectiles();

// Enemy collision and combat functions
bool checkProjectileEnemyCollision(const Projectile& projectile, 
                                  const ecs::Entity* enemy);
bool checkSwordEnemyCollision(const ecs::Entity* player, 
                             const ecs::Entity* enemy);
void handleEnemyHit(const ecs::Entity* enemy, int damage);
void updateEnemyState(float deltaTime);
void loadEnemySprites();
void initializeEnemySystem();
void testEnemySpriteChange(const ecs::Entity* enemy);

// Debugging flags
extern bool g_debugCollisions;
extern int g_totalHits;

// Game over screen functions
void render_game_over_screen(int xres, int yres, int& restart_option);
int handle_game_over_keys(int key, GameState &state, int &restart_option);

// Constants for game over actions
const int GAMEOVER_ACTION_NONE = 0;
const int GAMEOVER_ACTION_RESTART = 1;
const int GAMEOVER_ACTION_EXIT = 2;
