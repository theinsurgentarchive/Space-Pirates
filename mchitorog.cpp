#include "mchitorog.h"
#include <iostream>
#include <cstdlib>

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
    AudioManager::getInstance()->loadMusicTracks();
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

// AudioManager implementation

AudioManager::AudioManager() 
    : currentMusic(MENU_MUSIC), 
      musicVolume(MIX_MAX_VOLUME), 
      soundVolume(MIX_MAX_VOLUME),
      audioInitialized(false),
      musicEnabled(true),
      soundEnabled(true) {
}

AudioManager::~AudioManager() {
    shutdown();
}

AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

bool AudioManager::initAudio() {
    // Initialize SDL audio subsystem
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL audio initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Allocate channels for sound effects
    Mix_AllocateChannels(16);
    
    audioInitialized = true;
    return true;
}

void AudioManager::shutdown() {
    // Clean up sound effects
    for (auto& sound : soundEffects) {
        if (sound.second) {
            Mix_FreeChunk(sound.second);
            sound.second = nullptr;
        }
    }
    soundEffects.clear();
    
    // Clean up music tracks
    for (auto& music : musicTracks) {
        if (music.second) {
            Mix_FreeMusic(music.second);
            music.second = nullptr;
        }
    }
    musicTracks.clear();
    
    // Close SDL_mixer and SDL
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    
    audioInitialized = false;
}

bool AudioManager::loadSoundEffects() {
    if (!audioInitialized) return false;
    
    // Define paths to sound effects
    std::unordered_map<SoundType, std::string> soundPaths = {
        {MENU_CLICK, "./sounds/menu_click.wav"},
        {PLAYER_SHOOT, "./sounds/player_shoot.wav"},
        {PLAYER_HIT, "./sounds/player_hit.wav"},
        {ENEMY_HIT, "./sounds/enemy_hit.wav"},
        {PLAYER_DEATH, "./sounds/player_death.wav"},
        {GAME_WIN, "./sounds/game_win.wav"},
        {GAME_LOSE, "./sounds/game_lose.wav"},
        {POWER_UP, "./sounds/power_up.wav"}
    };
    
    // Load each sound effect
    for (const auto& sound : soundPaths) {
        Mix_Chunk* chunk = Mix_LoadWAV(sound.second.c_str());
        if (!chunk) {
            std::cerr << "Failed to load sound effect: " << sound.second << " - " << Mix_GetError() << std::endl;
            // Continue loading other sounds even if one fails
        } else {
            soundEffects[sound.first] = chunk;
        }
    }
    
    return !soundEffects.empty();
}

bool AudioManager::loadMusicTracks() {
    if (!audioInitialized) return false;
    
    // Define paths to music tracks
    std::unordered_map<MusicType, std::string> musicPaths = {
        {MENU_MUSIC, "./music/menu_music.mp3"},
        {GAME_MUSIC, "./music/game_music.mp3"},
        {BOSS_MUSIC, "./music/boss_music.mp3"},
        {VICTORY_MUSIC, "./music/victory_music.mp3"},
        {GAME_OVER_MUSIC, "./music/game_over_music.mp3"}
    };
    
    // Load each music track
    for (const auto& music : musicPaths) {
        Mix_Music* track = Mix_LoadMUS(music.second.c_str());
        if (!track) {
            std::cerr << "Failed to load music track: " << music.second << " - " << Mix_GetError() << std::endl;
            // Continue loading other tracks even if one fails
        } else {
            musicTracks[music.first] = track;
        }
    }
    
    return !musicTracks.empty();
}

void AudioManager::playSound(SoundType sound) {
    if (!audioInitialized || !soundEnabled) return;
    
    auto it = soundEffects.find(sound);
    if (it != soundEffects.end() && it->second) {
        Mix_VolumeChunk(it->second, soundVolume);
        Mix_PlayChannel(-1, it->second, 0);
    }
}

void AudioManager::stopAllSounds() {
    if (!audioInitialized) return;
    
    Mix_HaltChannel(-1);
}

void AudioManager::playMusic(MusicType music, bool loop) {
    if (!audioInitialized || !musicEnabled) return;
    
    auto it = musicTracks.find(music);
    if (it != musicTracks.end() && it->second) {
        // Stop current music if playing
        if (Mix_PlayingMusic()) {
            Mix_HaltMusic();
        }
        
        // Set the volume and play the new track
        Mix_VolumeMusic(musicVolume);
        Mix_PlayMusic(it->second, loop ? -1 : 0);
        currentMusic = music;
    }
}

void AudioManager::stopMusic() {
    if (!audioInitialized) return;
    
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}

void AudioManager::pauseMusic() {
    if (!audioInitialized) return;
    
    if (Mix_PlayingMusic() && !Mix_PausedMusic()) {
        Mix_PauseMusic();
    }
}

void AudioManager::resumeMusic() {
    if (!audioInitialized) return;
    
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void AudioManager::fadeOutMusic(int ms) {
    if (!audioInitialized) return;
    
    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(ms);
    }
}

void AudioManager::setMusicVolume(int volume) {
    // Clamp volume to valid range
    if (volume < 0) volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    
    musicVolume = volume;
    
    if (audioInitialized && Mix_PlayingMusic()) {
        Mix_VolumeMusic(musicVolume);
    }
}

void AudioManager::setSoundVolume(int volume) {
    // Clamp volume to valid range
    if (volume < 0) volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    
    soundVolume = volume;
}

int AudioManager::getMusicVolume() const {
    return musicVolume;
}

int AudioManager::getSoundVolume() const {
    return soundVolume;
}

void AudioManager::toggleMusic() {
    musicEnabled = !musicEnabled;
    
    if (!musicEnabled) {
        stopMusic();
    } else if (audioInitialized) {
        // Resume current music
        playMusic(currentMusic);
    }
}

void AudioManager::toggleSound() {
    soundEnabled = !soundEnabled;
}

bool AudioManager::isMusicEnabled() const {
    return musicEnabled;
}

bool AudioManager::isSoundEnabled() const {
    return soundEnabled;
}

void AudioManager::update() {
    // This function can be used for any per-frame audio updates
    // For example, handling dynamic music transitions
}
