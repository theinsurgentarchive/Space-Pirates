#include "mchitorog.h"
#include "balrowhany.h"
#include "jsandoval.h"
#include <cstring>
#include <unistd.h>

// Enable OpenAL sound by default
#ifndef USE_OPENAL_SOUND
#define USE_OPENAL_SOUND
#endif

// File paths for audio resources
const std::string AUDIO_PATH = "./resources/audio/";
const std::string SFX_MENU_PATH = AUDIO_PATH + "sfx/menu/";
const std::string SFX_GAMEPLAY_PATH = AUDIO_PATH + "sfx/gameplay/";
const std::string MUSIC_MENU_PATH = AUDIO_PATH + "music/menu/";
const std::string MUSIC_GAMEPLAY_PATH = AUDIO_PATH + "music/gameplay/";

// Initialize static instance pointer
AudioManager* AudioManager::instance = nullptr;

// AudioManager constructor
AudioManager::AudioManager() 
    : 
    #ifdef USE_OPENAL_SOUND
    device(nullptr),
    context(nullptr),
    #endif
    audioInitialized(false),
    musicEnabled(true),
    soundEnabled(true),
    musicVolume(100),
    soundVolume(100),
    currentMusic(MENU_MUSIC) {
}

// AudioManager destructor
AudioManager::~AudioManager() {
    shutdown();
}

// Get singleton instance
AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

// Global accessor function
AudioManager* getAudioManager() {
    return AudioManager::getInstance();
}

// Initialize audio system
bool AudioManager::initAudio() {
    #ifdef USE_OPENAL_SOUND
    // Initialize ALUT
    if (alutInit(NULL, NULL) != AL_TRUE) {
        DWARNF("Failed to initialize ALUT: %s\n", alutGetErrorString(alutGetError()));
        return false;
    }
    
    // Clear any AL errors
    alGetError();
    
    // Set up listener
    float orientation[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListenerfv(AL_ORIENTATION, orientation);
    alListenerf(AL_GAIN, 1.0f);
    
    audioInitialized = (alGetError() == AL_NO_ERROR);
    DINFOF("Audio system initialized: %s\n", audioInitialized ? "true" : "false");
    #else
    DINFO("OpenAL support not compiled in.\n");
    audioInitialized = false;
    #endif
    
    return audioInitialized;
}

// Shutdown audio system
void AudioManager::shutdown() {
    if (!audioInitialized) return;
    
    #ifdef USE_OPENAL_SOUND
    // Stop all sounds and music
    stopAllSounds();
    stopMusic();
    
    // Delete sound sources and buffers
    for (auto& pair : soundSources) {
        alDeleteSources(1, &pair.second);
    }
    soundSources.clear();
    
    for (auto& pair : soundBuffers) {
        alDeleteBuffers(1, &pair.second);
    }
    soundBuffers.clear();
    
    // Delete music sources and buffers
    for (auto& pair : musicSources) {
        alDeleteSources(1, &pair.second);
    }
    musicSources.clear();
    
    for (auto& pair : musicBuffers) {
        alDeleteBuffers(1, &pair.second);
    }
    musicBuffers.clear();
    
    // Close OpenAL
    ALCcontext* context = alcGetCurrentContext();
    if (context) {
        ALCdevice* device = alcGetContextsDevice(context);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        if (device) {
            alcCloseDevice(device);
        }
    }
    
    // Close ALUT
    alutExit();
    #endif
    
    audioInitialized = false;
    DINFO("Audio system shutdown.\n");
}

// Load a sound effect
bool AudioManager::loadSound(SoundType type, const std::string& filename) {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return false;
    
    // Create buffer
    ALuint buffer = alutCreateBufferFromFile(filename.c_str());
    if (buffer == AL_NONE) {
        DWARNF("Failed to load sound: %s\n", filename.c_str());
        return false;
    }
    
    // Create source
    ALuint source;
    alGenSources(1, &source);
    
    // Configure source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, soundVolume / 100.0f);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    
    // Store buffer and source
    soundBuffers[type] = buffer;
    soundSources[type] = source;
    
    return (alGetError() == AL_NO_ERROR);
    #else
    return false;
    #endif
}

// Load a music track
bool AudioManager::loadMusic(MusicType type, const std::string& filename) {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return false;
    
    // Create buffer
    ALuint buffer = alutCreateBufferFromFile(filename.c_str());
    if (buffer == AL_NONE) {
        DWARNF("Failed to load music: %s\n", filename.c_str());
        return false;
    }
    
    // Create source
    ALuint source;
    alGenSources(1, &source);
    
    // Configure source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, musicVolume / 100.0f);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcei(source, AL_LOOPING, AL_TRUE);  // Music loops by default
    
    // Store buffer and source
    musicBuffers[type] = buffer;
    musicSources[type] = source;
    
    return (alGetError() == AL_NO_ERROR);
    #else
    return false;
    #endif
}

// Load all sound effects
bool AudioManager::loadSoundEffects() {
    bool success = true;
    
    // Load menu sound effects
    success &= loadSound(MENU_CLICK, SFX_MENU_PATH + "menu_click.wav");
    
    // Load gameplay sound effects
    success &= loadSound(PLAYER_SHOOT, SFX_GAMEPLAY_PATH + "shoot.wav");
    success &= loadSound(PLAYER_HIT, SFX_GAMEPLAY_PATH + "hit.wav");
    success &= loadSound(ENEMY_HIT, SFX_GAMEPLAY_PATH + "enemy_hit.wav");
    success &= loadSound(PLAYER_DEATH, SFX_GAMEPLAY_PATH + "death.wav");
    success &= loadSound(GAME_WIN, SFX_GAMEPLAY_PATH + "win.wav");
    success &= loadSound(GAME_LOSE, SFX_GAMEPLAY_PATH + "lose.wav");
    success &= loadSound(POWER_UP, SFX_GAMEPLAY_PATH + "powerup.wav");
    
    return success;
}

// Load all music tracks
bool AudioManager::loadMusic() {
    bool success = true;
    
    // Load menu music
    success &= loadMusic(MENU_MUSIC, MUSIC_MENU_PATH + "menu_start_music.wav");
    success &= loadMusic(MENU_PAUSE_MUSIC, MUSIC_MENU_PATH + "menu_pause_music.wav");
    
    // Load gameplay music
    success &= loadMusic(GAME_MUSIC, MUSIC_GAMEPLAY_PATH + "background_4.wav");
    
    return success;
}

// Play a sound effect
void AudioManager::playSound(SoundType sound) {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized || !soundEnabled) return;
    
    auto it = soundSources.find(sound);
    if (it != soundSources.end()) {
        alSourcePlay(it->second);
        DINFOF("Playing sound: %d\n", sound);
    }
    #endif
}

// Stop a sound effect
void AudioManager::stopSound(SoundType sound) {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return;
    
    auto it = soundSources.find(sound);
    if (it != soundSources.end()) {
        alSourceStop(it->second);
    }
    #endif
}

// Stop all sound effects
void AudioManager::stopAllSounds() {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return;
    
    for (auto& pair : soundSources) {
        alSourceStop(pair.second);
    }
    #endif
}

// Play music
void AudioManager::playMusic(MusicType music) {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized || !musicEnabled) return;
    
    // Stop current music if any
    stopMusic();
    
    auto it = musicSources.find(music);
    if (it != musicSources.end()) {
        alSourcePlay(it->second);
        currentMusic = music;
        DINFOF("Playing music: %d\n", music);
    }
    #endif
}

// Pause current music
void AudioManager::pauseMusic() {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return;
    
    auto it = musicSources.find(currentMusic);
    if (it != musicSources.end()) {
        alSourcePause(it->second);
    }
    #endif
}

// Resume current music
void AudioManager::resumeMusic() {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized || !musicEnabled) return;
    
    auto it = musicSources.find(currentMusic);
    if (it != musicSources.end()) {
        ALint state;
        alGetSourcei(it->second, AL_SOURCE_STATE, &state);
        if (state == AL_PAUSED) {
            alSourcePlay(it->second);
        }
    }
    #endif
}

// Stop current music
void AudioManager::stopMusic() {
    #ifdef USE_OPENAL_SOUND
    if (!audioInitialized) return;
    
    for (auto& pair : musicSources) {
        alSourceStop(pair.second);
    }
    #endif
}

// Set music volume
void AudioManager::setMusicVolume(int volume) {
    #ifdef USE_OPENAL_SOUND
    // Clamp volume to valid range
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    musicVolume = volume;
    
    if (audioInitialized) {
        for (auto& pair : musicSources) {
            alSourcef(pair.second, AL_GAIN, musicVolume / 100.0f);
        }
    }
    #endif
}

// Set sound effects volume
void AudioManager::setSoundVolume(int volume) {
    #ifdef USE_OPENAL_SOUND
    // Clamp volume to valid range
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    soundVolume = volume;
    
    if (audioInitialized) {
        for (auto& pair : soundSources) {
            alSourcef(pair.second, AL_GAIN, soundVolume / 100.0f);
        }
    }
    #endif
}

// Get music volume
int AudioManager::getMusicVolume() const {
    return musicVolume;
}

// Get sound effects volume
int AudioManager::getSoundVolume() const {
    return soundVolume;
}

// Toggle music on/off
void AudioManager::toggleMusic() {
    musicEnabled = !musicEnabled;
    
    if (!musicEnabled) {
        stopMusic();
    } else {
        playMusic(currentMusic);
    }
}

// Toggle sound effects on/off
void AudioManager::toggleSound() {
    soundEnabled = !soundEnabled;
}

// Check if music is enabled
bool AudioManager::isMusicEnabled() const {
    return musicEnabled;
}

// Check if sound effects are enabled
bool AudioManager::isSoundEnabled() const {
    return soundEnabled;
}

// Update function to be called each frame
void AudioManager::update() {
    // Nothing to do in update for now
}

// Initialize audio system
void initAudioSystem() {
    AudioManager::getInstance()->initAudio();
    AudioManager::getInstance()->loadSoundEffects();
    AudioManager::getInstance()->loadMusic();
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
        case CREDITS:
            playGameMusic(MENU_PAUSE_MUSIC);
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

void render_credits_screen(int xres, int yres, GLuint menuBackgroundTexture)
{
    DisableFor2D();

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();

    // Determine font size based on window dimensions
    int titleSize = 40;
    int headingSize = 17;
    int textSize = 14;
    int returnSize = 13;

    if (xres < 800 || yres < 600) {
        titleSize = 30;
        headingSize = 14;
        textSize = 12;
        returnSize = 10;
    }

    // Credits title - position at 10% from top
    Rect r;
    r.left = xres/2;
    r.bot = yres - (yres * 0.1);
    r.center = 1;

    ggprint40(&r, titleSize, 0xFF00FF00, "CREDITS");

    // Team member credits - position relative to window height
    r.bot = yres - (yres * 0.15);
    ggprint17(&r, headingSize, 0xFFFFFFFF, "DEVELOPMENT TEAM");

    // Calculate vertical spacing based on window height and number of entries
    float totalEntriesHeight = yres * 0.5;  // Use 50% of screen height for entries
    int numEntries = 5;  // 5 team members
    int entryHeight = totalEntriesHeight / numEntries;

    // Starting position for first entry (after heading)
    r.bot = yres - (yres * 0.25);

    // Billie
    ggprint17(&r, textSize, 0xFF00FF00, "Billie");
    r.bot -= 10;
    ggprint17(&r, textSize, 0xFF7DF9FF, "Resource Management");

    // David
    r.bot -= entryHeight;
    ggprint17(&r, textSize, 0xFF00FF00, "David C.");
    r.bot -= 10;
    ggprint17(&r, textSize, 0xFF7DF9FF, "Combat & Navigation Systems");

    // Juancarlos
    r.bot -= entryHeight;
    ggprint17(&r, textSize, 0xFF00FF00, "Juancarlos");
    r.bot -= 10;
    ggprint17(&r, textSize, 0xFF7DF9FF, "Planet System Implementation");

    // Justin
    r.bot -= entryHeight;
    ggprint17(&r, textSize, 0xFF00FF00, "Justin");
    r.bot -= 10;
    ggprint17(&r, textSize, 0xFF7DF9FF, "Technical Framework");

    // Mihail
    r.bot -= entryHeight;
    ggprint17(&r, textSize, 0xFF00FF00, "Mihail");
    r.bot -= 10;
    ggprint17(&r, textSize, 0xFF7DF9FF, "Audio Implementation");

    // Instructions to return - position at 5% from bottom
    r.bot = yres * 0.05;
    ggprint13(&r, returnSize, 0xFFFFFFFF, "Press ESC or ENTER to return to the menu");

    EnableFor3D();
}

void resetGLState(int xres, int yres) {
    // Reset matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xres, 0, yres, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Reset other common OpenGL state
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    // Clear any errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        DINFOF("OpenGL error cleared: %d\n", error);
    }
}
