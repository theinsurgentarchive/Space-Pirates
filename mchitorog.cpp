//
//program: mchitorog.cpp
//author: Mihail Chitorog
//
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

FootstepState g_footstepState = {
	false, // isWalking
	std::chrono::high_resolution_clock::now(), // lastFootstepTime
	std::chrono::milliseconds(600), // footstepInterval
	false, // isSoundPlaying
	std::chrono::milliseconds(600) 
};

// Initialize the global weapon state
WeaponState g_weaponState = {
	WEAPON_NONE,     // Start with no weapon (will switch to sword on init)
	10,              // Initial handgun ammo
	5,               // Initial rocket ammo
	std::chrono::high_resolution_clock::now(),
	std::chrono::milliseconds(300),  // Default cooldown period
	true             // Can fire initially
};


// Initialize global enemy state
EnemyState g_enemyState = {
	15,     // Initial health (can be adjusted)
	15,     // Max health
	false,  // Not dead initially
	false,  // Not hit initially
	0.0f,   // Hit timer
	0.0f    // Death timer
};

SwordAttackState g_swordAttackState = {
    false,
    std::chrono::high_resolution_clock::now(),
    std::chrono::milliseconds(300)
};

bool g_debugCollisions = true; // Set to true to enable detailed debug output
int g_totalHits = 0;           // Count total hits across the game

MusicType AudioManager::getCurrentMusic() const {
	return currentMusic;
}

// Initialize global projectile array
Projectile projectiles[MAX_PROJECTILES];

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

			soundGainMultipliers[PLAYER_GUN_SHOT] = 0.5f;
			soundGainMultipliers[PLAYER_ROCKET_LAUNCH] = 0.6f;
			soundGainMultipliers[PLAYER_NEED_AMMO] = 0.6f;
			soundGainMultipliers[PLAYER_NEED_BIGGER_WEAPON] = 0.6f;
			soundGainMultipliers[PLAYER_HASTA_LA_VISTA] = 0.6f;
			soundGainMultipliers[PLAYER_NOW_THATS_A_WEAPON] = 0.6f;

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
	success &= loadSound(FOOTSTEP_GRASS, SFX_GAMEPLAY_PATH + "footstep_grass.wav");
	success &= loadSound(FOOTSTEP_SNOW, SFX_GAMEPLAY_PATH + "footstep_snow.wav");
	success &= loadSound(FOOTSTEP_LAVA, SFX_GAMEPLAY_PATH + "footstep_lava.wav");
	success &= loadSound(PLAYER_SWORD_SWING, SFX_GAMEPLAY_PATH + "sword_swing.wav");
	success &= loadSound(PLAYER_NEED_AMMO, SFX_GAMEPLAY_PATH + "jlo_i_need_more_bullets.wav");
	success &= loadSound(PLAYER_NOW_THATS_A_WEAPON, SFX_GAMEPLAY_PATH + "jlo_now_that_is_a_weapon.wav");
	success &= loadSound(PLAYER_NEED_BIGGER_WEAPON, SFX_GAMEPLAY_PATH + "jlo_bigger_weapons.wav");
	success &= loadSound(PLAYER_WEAPON_SWITCH, SFX_GAMEPLAY_PATH + "weapon_switch.wav");
	success &= loadSound(PLAYER_GUN_SHOT, SFX_GAMEPLAY_PATH + "gun_shot.wav");
	success &= loadSound(PLAYER_ROCKET_LAUNCH, SFX_GAMEPLAY_PATH + "rocket_launch.wav");
	success &= loadSound(PLAYER_HASTA_LA_VISTA, SFX_GAMEPLAY_PATH + "jlo_hasta_la_vista_baby.wav");
	success &= loadSound(ENEMY_GRUNT, SFX_GAMEPLAY_PATH + "gobo_grunt.wav");
	success &= loadSound(ENEMY_HIT, SFX_GAMEPLAY_PATH + "gobo_hit.wav");
	success &= loadSound(ENEMY_DEATH, SFX_GAMEPLAY_PATH + "gobo_death.wav");


	return success;
}

// Load all music tracks
bool AudioManager::loadMusic() {
	bool success = true;

	// Load menu music
	success &= loadMusic(MENU_MUSIC, MUSIC_MENU_PATH + "menu_start_music.wav");
	success &= loadMusic(MENU_PAUSE_MUSIC, MUSIC_MENU_PATH + "menu_pause_music.wav");

	// Add Space Music
	success &= loadMusic(SPACE_MUSIC, MUSIC_GAMEPLAY_PATH + "background_space.wav");

	// Add biome-specific music
	success &= loadMusic(FOREST_MUSIC, MUSIC_GAMEPLAY_PATH + "background_forest_planet.wav");
	success &= loadMusic(DESERT_MUSIC, MUSIC_GAMEPLAY_PATH + "background_desert_planet.wav");
	success &= loadMusic(TAIGA_MUSIC, MUSIC_GAMEPLAY_PATH + "background_taiga_planet.wav");
	success &= loadMusic(MEADOW_MUSIC, MUSIC_GAMEPLAY_PATH + "background_meadow_planet.wav");
	success &= loadMusic(HELL_MUSIC, MUSIC_GAMEPLAY_PATH + "background_hell_planet.wav");

	return success;
}

// Play a sound effect
void AudioManager::playSound(SoundType sound) {
#ifdef USE_OPENAL_SOUND
	if (!audioInitialized || !soundEnabled) return;

	// Check if this sound was recently played to avoid stuttering
	auto now = std::chrono::high_resolution_clock::now();
	auto it = lastSoundPlayed.find(sound);

	if (it != lastSoundPlayed.end()) {
		auto timeSinceLastPlay = std::chrono::duration_cast<std::chrono::milliseconds>(
				now - it->second);

		// If the same sound was played too recently, skip it
		if (timeSinceLastPlay < soundDebounceTime) {
			DINFOF("Skipping sound %d - too soon since last played (%lld ms)\n", 
					sound, timeSinceLastPlay.count());
			return;
		}
	}

	// Update the last played time for this sound
	lastSoundPlayed[sound] = now;

	auto sourceIt = soundSources.find(sound);
	if (sourceIt != soundSources.end()) {
		ALint state;
		alGetSourcei(sourceIt->second, AL_SOURCE_STATE, &state);
		// Calculate the gain/volume to use
		float gain = soundVolume / 100.0f;
		// Apply custom gain multiplier if one exists for this sound
		auto gainIt = soundGainMultipliers.find(sound);
		if (gainIt != soundGainMultipliers.end()) {
			gain *= gainIt->second;
		}
		// For footstep sounds specifically, handle differently
		if (sound == FOOTSTEP_GRASS || sound == FOOTSTEP_SNOW || sound == FOOTSTEP_LAVA) {
			// If already playing, don't interrupt it
			if (state == AL_PLAYING) {
				return;
			}

			// Set footstep-specific properties
			alSourcef(sourceIt->second, AL_GAIN, soundVolume / 100.0f * 1.2f);
			alSourcef(sourceIt->second, AL_PITCH, 0.9f + ((float)rand() /
			 RAND_MAX) * 0.2f); // Random pitch variation
		}

		alSourcef(sourceIt->second, AL_GAIN, gain);
		alSourcePlay(sourceIt->second);
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

	// stop current music if it's different from the new music
	if (currentMusic != music) {
		stopMusic();
	}

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
		ALint state;
		alGetSourcei(pair.second, AL_SOURCE_STATE, &state);
		if (state == AL_PLAYING) {
			alSourceStop(pair.second);
		}
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

	initializePlayerSprites();
	initializeWeaponSystem(); 
	loadWeaponSprites(ssheets);
	initializeProjectileSystem();
	initializeEnemySystem();
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

// Get the corresponding music type for a biome type
MusicType getBiomeMusicType(BiomeType biomeType) {
	switch (biomeType) {
		case FOREST:
			return FOREST_MUSIC;
		case DESERT:
			return DESERT_MUSIC;
		case TAIGA:
			return TAIGA_MUSIC;
		case MEADOW:
			return MEADOW_MUSIC;
		case HELL:
			return HELL_MUSIC;
		default:
			return GAME_MUSIC; // Default game music as fallback
	}
}

// Play biome-specific music based on planet type
void playBiomeMusic(BiomeType biomeType) {
	MusicType musicType = getBiomeMusicType(biomeType);
	playGameMusic(musicType);
	DINFOF("Playing biome music for: %d (Music type: %d)\n", biomeType, musicType);
}

// Update audio based on game state
void updateAudioState(GameState state) {
	switch (state) {
		case SPLASH:
		case MENU:
			playGameMusic(MENU_MUSIC);
			break;
		case PLAYING:
			// For PLAYING state, determine the appropriate biome music
			if (planetPtr) {
				auto [traits] = ecs::ecs.component().fetch<ecs::Planet>(planetPtr);
				if (traits) {
					// Determine biome type based on temperature and humidity
					BiomeType biomeType;
					float temp = traits->temperature;
					float humidity = traits->humidity;

					// Simple biome determination logic
					if (temp >= 30.0f && temp <= 85.0f && humidity >= 0.0f && 
						humidity <= 0.8f) {
						biomeType = DESERT;
						DINFO("Playing music for DESERT biome\n");
					} else if (temp >= -40.0f && temp <= 10.0f && humidity >= 
						0.1f && humidity <= 0.8f) {
						biomeType = TAIGA;
						DINFO("Playing music for TAIGA biome\n");
					} else if (temp >= 0.0f && temp <= 20.0f && humidity >= 0.4f 
						&& humidity <= 1.0f) {
						biomeType = MEADOW;
						DINFO("Playing music for MEADOW biome\n");
					} else if (temp >= 85.0f && humidity >= 0.0f && humidity <= 
						0.6f) {
						biomeType = HELL;
						DINFO("Playing music for HELL biome\n");
					} else {
						biomeType = FOREST; // Default biome
						DINFO("Playing music for FOREST biome\n");
					}

					// Play the corresponding music
					playBiomeMusic(biomeType);
				} else {
					playGameMusic(GAME_MUSIC); // Fallback
				}
			} else {
				playGameMusic(GAME_MUSIC); // Fallback
			}
			break;
		case SPACE:  
			playGameMusic(SPACE_MUSIC);
			break;
		case PAUSED:
			// Just pause the current music
			getAudioManager()->pauseMusic();
			break;
		case CONTROLS:
			if (getAudioManager()->getCurrentMusic() != MENU_PAUSE_MUSIC) {
				playGameMusic(MENU_PAUSE_MUSIC);
			}
			break;
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


void changeGameState(GameState& currentState, GameState newState) {
	currentState = newState;
	updateAudioState(newState);
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
	float totalEntriesHeight = yres * 0.5;
	int numEntries = 5;  // 5 team members
	int entryHeight = totalEntriesHeight / numEntries;

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

	r.bot = yres * 0.02;
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

void render_title_logo(int xres, int yres, GLuint titleTexture) {
	int logoWidth = 300;
	int logoHeight = 300;
	int logoX = (xres - logoWidth) / 2;
	int logoY = yres - 150 - logoWidth / 2;  // Position from top

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, titleTexture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(logoX, logoY);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(logoX, logoY + logoHeight);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(logoX + logoWidth, logoY + logoHeight);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(logoX + logoWidth, logoY);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void render_pause_menu(int xres, int yres, int selected_option) {
	// Create a semi-transparent overlay to dim the background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f); // Black with 70% opacity

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, yres);
	glVertex2i(xres, yres);
	glVertex2i(xres, 0);
	glEnd();

	// Draw the pause menu box
	int boxWidth = 300;
	int boxHeight = 200;
	int boxX = (xres - boxWidth) / 2;
	int boxY = (yres - boxHeight) / 2;

	// Draw box background
	glColor4f(0.1f, 0.1f, 0.1f, 0.9f); // Dark grey with 90% opacity
	glBegin(GL_QUADS);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	// Draw box border
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	glDisable(GL_BLEND);

	// Render text
	Rect r;
	r.left = xres / 2;
	r.bot = yres / 2 + 70;
	r.center = 1;

	// Title
	ggprint17(&r, 0, 0xFFFFFFFF, "GAME PAUSED");

	// Menu options
	r.bot = yres / 2 + 20;
	const char* options[] = {"RESUME", "CONTROLS", "MAIN MENU"};

	for (int i = 0; i < 3; i++) {
		int color = (i == selected_option) ? 0xFF00FF00 : 0xFFFFFFFF;
		ggprint13(&r, 35, color, options[i]);
	}
}

int handle_pause_keys(int key, GameState &state, GameState &previous_state, 
	int &selected_option, MusicType previous_music) {
	switch(key) {
		case XK_Up:
			playGameSound(MENU_CLICK);
			selected_option = (selected_option - 1 + 3) % 3;
			break;

		case XK_Down:
			playGameSound(MENU_CLICK);
			selected_option = (selected_option + 1) % 3;
			break;

		case XK_Return:
			playGameSound(MENU_CLICK);
			switch(selected_option) {
				case 0: // Resume
					state = previous_state;
					if (getAudioManager()->isMusicEnabled()) {
						playGameMusic(previous_music);
					}
					break;
				case 1: // Controls
					state = CONTROLS;
					updateAudioState(state);
					break;
				case 2: // Main Menu
					state = MENU;
					updateAudioState(state);
					break;
			}
			break;

		case XK_Escape:
			// Resume game on escape
			playGameSound(MENU_CLICK);
			state = previous_state;
			if (getAudioManager()->isMusicEnabled()) {
				playGameMusic(previous_music);
			}
			break;
	}
	return PAUSE_ACTION_NONE;
}

void render_pause_controls_screen(int xres, int yres) {
	DINFO("=== ENTER render_pause_controls_screen ===\n");
	DINFOF("xres: %d, yres: %d\n", xres, yres);

	// Check for valid resolution
	if (xres <= 0 || yres <= 0) {
		DERROR("Invalid resolution in render_pause_controls_screen!\n");
		return;
	}

	// Semi-transparent overlay (same as pause menu)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, yres);
	glVertex2i(xres, yres);
	glVertex2i(xres, 0);
	glEnd();

	// Draw controls in a box (similar to pause menu style)
	int boxWidth = 400;
	int boxHeight = 300;
	int boxX = (xres - boxWidth) / 2;
	int boxY = (yres - boxHeight) / 2;


	DINFOF("Box dimensions: width=%d, height=%d, x=%d, y=%d\n",
			boxWidth, boxHeight, boxX, boxY);

	// Box background
	glColor4f(0.1f, 0.1f, 0.1f, 0.9f);
	glBegin(GL_QUADS);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	// Box border
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	glDisable(GL_BLEND);

	// Render text
	Rect r;
	r.left = xres / 2;
	r.bot = yres / 2 + 120;
	r.center = 1;

	DINFO("About to render text\n");

	ggprint17(&r, 0, 0xFFFFFFFF, "CONTROLS");

	// Game controls
	r.bot = yres / 2 + 60;
	ggprint13(&r, 20, 0xFF7DF9FF, "<-^-> - Move ship");
	r.bot -= 20;
	ggprint13(&r, 20, 0xFF7DF9FF, "SPACE - Fire");
	r.bot -= 20;
	ggprint13(&r, 20, 0xFF7DF9FF, "E - Interact");
	r.bot -= 20;
	ggprint13(&r, 20, 0xFF7DF9FF, "ESC - Pause/Back");

	// Audio controls
	r.bot -= 40;
	ggprint13(&r, 20, 0xFFFFFFFF, "AUDIO");
	r.bot -= 20;

	DINFO("About to check AudioManager\n");


	AudioManager* audioMgr = getAudioManager();
	if (!audioMgr) {
		DERROR("AudioManager is null!\n");
		return;
	}

	if (audioMgr->isMusicEnabled()) {
		ggprint13(&r, 20, 0xFF7DF9FF, "M - Toggle Music [ON]");
	} else {
		ggprint13(&r, 20, 0xFF999999, "M - Toggle Music [OFF]");
	}

	r.bot -= 20;
	if (audioMgr->isSoundEnabled()) {
		ggprint13(&r, 20, 0xFF7DF9FF, "S - Toggle Sound [ON]");
	} else {
		ggprint13(&r, 20, 0xFF999999, "S - Toggle Sound [OFF]");
	}

	// Return instruction
	r.bot = yres / 2 - 100;
	ggprint13(&r, 0, 0xFFFFFFFF, "Press ESC to return");

	DINFO("=== EXIT render_pause_controls_screen ===\n");

}

// Tracks the last direction the player was moving
static PlayerDirection lastPlayerDirection = DIR_DOWN;

void initializePlayerSprites() {
	// This function simply logs that we're going to use new player sprites
	DINFO("Initializing player directional sprites...\n");
}

void updatePlayerMovementSprite(const ecs::Entity* player, PlayerDirection direction) {
	if (!player) return;

	auto [sprite, physics] = ecs::ecs.component().fetch<SPRITE, PHYSICS>(player);

	if (!sprite || !physics) {
		DWARN("Player missing sprite or physics component\n");
		return;
	}

	// Store the direction for use when stopping
	lastPlayerDirection = direction;

	DINFOF("Setting player direction to: %d\n", direction);

	// Set appropriate movement sprite based on direction
	switch (direction) {
		case DIR_RIGHT:
			sprite->ssheet = "player-right";
			sprite->invert_y = false;
			physics->vel = {75.0f, 0.0f};
			break;

		case DIR_LEFT:
			sprite->ssheet = "player-right";
			sprite->invert_y = true;
			physics->vel = {-75.0f, 0.0f};
			break;

		case DIR_UP:
			sprite->ssheet = "player-back";
			sprite->invert_y = false;
			physics->vel = {0.0f, 75.0f};
			break;

		case DIR_DOWN:
			sprite->ssheet = "player-front";
			sprite->invert_y = false;
			physics->vel = {0.0f, -75.0f};
			break;
	}

	// Update sprites based on current weapon
	updateWeaponSprites(player);
	// Play footstep sound
	playFootstepSound();
}

void updatePlayerIdleSprite(const ecs::Entity* player) {
	if (!player) return;

	auto [sprite, physics] = ecs::ecs.component().fetch<SPRITE, PHYSICS>(player);

	if (!sprite || !physics) {
		DWARN("Player missing sprite or physics component\n");
		return;
	}

	// Stop player movement
	physics->vel = {0.0f, 0.0f};

	DINFOF("Setting player idle direction based on last direction: %d\n", 
		lastPlayerDirection);

	// Set idle sprite based on last direction
	switch (lastPlayerDirection) {
		case DIR_RIGHT:
			sprite->ssheet = "player-right-stand";
			sprite->invert_y = false;
			break;

		case DIR_LEFT:
			sprite->ssheet = "player-right-stand";
			sprite->invert_y = true;
			break;

		case DIR_UP:
			sprite->ssheet = "player-idle-back";
			sprite->invert_y = false;
			break;

		case DIR_DOWN:
			sprite->ssheet = "player-idle";
			sprite->invert_y = false;
			break;
	}

	// Reset animation frame
	sprite->frame = 0;
}

void handlePlayerMovementInput(int key, const ecs::Entity* player) {
	switch(key) {
		case XK_Right:
			updatePlayerMovementSprite(player, DIR_RIGHT);
			break;
		case XK_Left:
			updatePlayerMovementSprite(player, DIR_LEFT);
			break;
		case XK_Up:
			updatePlayerMovementSprite(player, DIR_UP);
			break;
		case XK_Down:
			updatePlayerMovementSprite(player, DIR_DOWN);
			break;
	}
}

void handlePlayerKeyRelease(const ecs::Entity* player) {
	if (!player) return;

	auto [sprite, physics] = ecs::ecs.component().fetch<SPRITE, PHYSICS>(player);

	if (!sprite || !physics) return;

	// Force a specific sprite, bypassing the direction logic for testing
	sprite->ssheet = "player-idle-back";  // Always use back sprite on release
	sprite->invert_y = false;
	physics->vel = {0.0f, 0.0f};

	DINFO("TEST: Force-set player to player-idle-back\n");
}

void playFootstepSound() {
	auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(player);
	if (!transform) return;

	auto [planet] = ecs::ecs.component().fetch<PLANET>(planetPtr);
	if (planet) {
		float temp = planet->temperature;
		float humidity = planet->humidity;

		// Determine sound type based on planet conditions
		SoundType soundType;
		if (temp >= -40.0f && temp <= 10.0f && humidity >= 0.1f && humidity <= 0.8f) {
			soundType = FOOTSTEP_SNOW;
		}
		else if (temp >= 85.0f && humidity >= 0.0f && humidity <= 0.6f) {
			soundType = FOOTSTEP_LAVA;
		}
		else {
			soundType = FOOTSTEP_GRASS;
		}

		// Play the sound
		playGameSound(soundType);
	} else {
		playGameSound(FOOTSTEP_GRASS);
	}
}

void updateFootstepSounds() {
	if (!player) return;

	[[maybe_unused]]static int debugCounter = 0;
	static auto lastDebugTime = std::chrono::high_resolution_clock::now();

	auto [physics, transform] = ecs::ecs.component().fetch<PHYSICS, TRANSFORM>(player);
	if (!physics || !transform) return;

	auto currentTime = std::chrono::high_resolution_clock::now();

	// Only print debug info once per second
	auto timeSinceLastDebug = std::chrono::duration_cast<std::chrono::milliseconds>(
			currentTime - lastDebugTime);
	if (timeSinceLastDebug.count() > 1000) {
		DINFOF("Footstep Debug: isWalking=%d, vel=(%f,%f)\n",
				g_footstepState.isWalking, physics->vel[0], physics->vel[1]);
		lastDebugTime = currentTime;
	}

	// Check if player is moving
	bool isCurrentlyMoving = (fabs(physics->vel[0]) > 0.1f || fabs(physics->vel[1]) > 0.1f);

	auto timeSinceLastFootstep = std::chrono::duration_cast<std::chrono::milliseconds>(
			currentTime - g_footstepState.lastFootstepTime);

	// Update walking state
	if (isCurrentlyMoving != g_footstepState.isWalking) {
		g_footstepState.isWalking = isCurrentlyMoving;
		DINFOF("Walking state changed to: %d\n", g_footstepState.isWalking);

		// If just started walking, reset timer to play sound immediately
		if (isCurrentlyMoving) {
			g_footstepState.lastFootstepTime = currentTime - 
			g_footstepState.footstepInterval;
		}
	}

	// Play footstep sound at regular intervals while walking
	if (g_footstepState.isWalking && timeSinceLastFootstep >= 
		g_footstepState.footstepInterval) {
		DINFOF("Playing footstep! Time since last: %ld ms\n", 
			timeSinceLastFootstep.count());
		playFootstepSound();
		g_footstepState.lastFootstepTime = currentTime;
	}
}

// Initialize the weapon system
void initializeWeaponSystem() {
	DINFO("Initializing weapon system...\n");
	// Start with no weapon - weapon state already initialized to WEAPON_NONE
	DINFOF("Starting with weapon type: %d (NONE)\n", g_weaponState.currentWeapon);
}

// Implement the loadWeaponSprites function
void loadWeaponSprites(std::unordered_map<std::string, 
	std::shared_ptr<SpriteSheet>>& sheets) {
	SpriteSheetLoader loader {sheets};

	// Sword sprites
	loader
		.loadStatic("player-idle-sword",
				loadTexture("./resources/textures/player/idle_sword.webp", true),
				{1,1}, {24,32})
	.loadStatic("player-idle-back-sword",
			loadTexture("./resources/textures/player/idle_back_sword.webp", false),
			{1,1}, {24,32})
	.loadStatic("player-right-idle-sword",
			loadTexture("./resources/textures/player/right_stand_sword.webp", true),
			{1,1}, {26,32})
	.loadStatic("player-front-sword",
			loadTexture("./resources/textures/player/front_sword.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-back-sword",
			loadTexture("./resources/textures/player/back_sword.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-right-sword",
			loadTexture("./resources/textures/player/right_sword.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-idle-sword-attack",
			loadTexture("./resources/textures/player/idle_sword_attack.webp", true),
			{1,1}, {24,32})
	.loadStatic("player-idle-back-sword-attack",
			loadTexture("./resources/textures/player/idle_back_sword_attack.webp", true),
			{1,1}, {24,32})
	.loadStatic("player-right-idle-sword-attack",
			loadTexture("./resources/textures/player/right_stand_sword_attack.webp", true),
			{1,1}, {26,32});

	// Handgun sprites
	loader
		.loadStatic("player-idle-gun",
				loadTexture("./resources/textures/player/idle_gun.webp", true),
				{1,1}, {20,32})
	.loadStatic("player-idle-back-gun",
			loadTexture("./resources/textures/player/idle_back_gun.webp", true),
			{1,1}, {18,32})
	.loadStatic("player-right-idle-gun",
			loadTexture("./resources/textures/player/right_stand_gun.webp", true),
			{1,1}, {30,34})
	.loadStatic("player-front-gun",
			loadTexture("./resources/textures/player/front_gun.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-back-gun",
			loadTexture("./resources/textures/player/back_gun.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-right-gun",
			loadTexture("./resources/textures/player/right_gun.webp", true),
			{1,3}, {26,32}, true);

	// Rocket launcher sprites
	loader
		.loadStatic("player-idle-rocket",
				loadTexture("./resources/textures/player/idle_rocket.webp", true),
				{1,1}, {19,37})
	.loadStatic("player-idle-back-rocket",
			loadTexture("./resources/textures/player/idle_back_rocket.webp", true),
			{1,1}, {18,32})
	.loadStatic("player-right-idle-rocket",
			loadTexture("./resources/textures/player/right_stand_rocket.webp", true),
			{1,1}, {26,32})
	.loadStatic("player-front-rocket",
			loadTexture("./resources/textures/player/front_rocket.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-back-rocket",
			loadTexture("./resources/textures/player/back_rocket.webp", true),
			{1,3}, {26,32}, true)
	.loadStatic("player-right-rocket",
			loadTexture("./resources/textures/player/right_rocket.webp", true),
			{1,3}, {26,32}, true);
}

// Switch to a different weapon
void switchWeapon(WeaponType newWeapon, bool playSounds) {
	// Only switch if it's a different weapon
	if (newWeapon == g_weaponState.currentWeapon) {
		DINFOF("Already using weapon type: %d\n", newWeapon);
		return;
	}

	// Store the old weapon type to check for specific upgrade paths
	WeaponType oldWeapon = g_weaponState.currentWeapon;

	// Check if we have ammo for the requested weapon (only for gun and rocket)
	if (newWeapon == WEAPON_HANDGUN && g_weaponState.handgunAmmo <= 0) {
		if (playSounds) playGameSound(PLAYER_NEED_BIGGER_WEAPON);
		DINFO("Cannot switch to handgun - no ammo\n");
		return;
	}

	if (newWeapon == WEAPON_ROCKET && g_weaponState.rocketAmmo <= 0) {
		if (playSounds) playGameSound(PLAYER_NEED_AMMO);
		DINFO("Cannot switch to rocket - no ammo\n");
		return;
	}

	// Switch the weapon
	DINFOF("Switching from weapon %d to weapon %d\n", 
		g_weaponState.currentWeapon, newWeapon);
	g_weaponState.currentWeapon = newWeapon;

	// Play appropriate sound
	if (playSounds) {
		// Check for specific weapon upgrade - gun to rocket
		if (oldWeapon == WEAPON_HANDGUN && newWeapon == WEAPON_ROCKET) {
			// Special upgrade sound
			playGameSound(PLAYER_NOW_THATS_A_WEAPON);
		} else {
			// Default weapon switch sound
			playGameSound(PLAYER_WEAPON_SWITCH);
		}
	}

	// Update player sprites based on current weapon and direction
	updateWeaponSprites(player);
}



// Fire the current weapon
bool fireWeapon() {
	// Check if player has a weapon equipped
	if (g_weaponState.currentWeapon == WEAPON_NONE) {
		// Cannot fire without a weapon
		return false;
	}

	auto currentTime = std::chrono::high_resolution_clock::now();
	auto timeSinceLastFire = std::chrono::duration_cast<std::chrono::milliseconds>(
			currentTime - g_weaponState.lastFireTime);

	// Check cooldown
	if (timeSinceLastFire < g_weaponState.cooldownPeriod) {
		return false;
	}

	auto [transform, sprite] = ecs::ecs.component().fetch<TRANSFORM, SPRITE>(player);
	if (!transform) {
		DWARN("Cannot fire weapon - player has no transform component\n");
		return false;
	}

	// Determine projectile direction based on player facing
	float dirX = 0.0f, dirY = 0.0f;

	// Use lastPlayerDirection to determine projectile direction
	switch (lastPlayerDirection) {
		case DIR_UP:
			dirX = 0.0f;
			dirY = 1.0f;
			break;
		case DIR_DOWN:
			dirX = 0.0f;
			dirY = -1.0f;
			break;
		case DIR_LEFT:
			dirX = -1.0f;
			dirY = 0.0f;
			break;
		case DIR_RIGHT:
			dirX = 1.0f;
			dirY = 0.0f;
			break;
	}

	// Check if we have ammo for handgun
	if (g_weaponState.currentWeapon == WEAPON_HANDGUN) {
		if (g_weaponState.handgunAmmo <= 0) {
			// No ammo at all
			playGameSound(PLAYER_NEED_BIGGER_WEAPON);
			return false;
		} else if (g_weaponState.handgunAmmo <= 3) {
			// Low on ammo - play warning sound but still fire
			playGameSound(PLAYER_NEED_AMMO);
			usleep(30000); // 300ms delay to let voice start before gun sound
		}
	}

	// Check if we have ammo for rocket
	if (g_weaponState.currentWeapon == WEAPON_ROCKET && 
		g_weaponState.rocketAmmo <= 0) {
		playGameSound(PLAYER_NEED_AMMO);
		return false;
	}

	// Fire the weapon
	if (g_weaponState.currentWeapon == WEAPON_SWORD) {
		// No projectile for sword, just play sound
		playGameSound(PLAYER_SWORD_SWING);
		g_weaponState.cooldownPeriod = std::chrono::milliseconds(300);

		// Set appropriate attack sprite based on direction
		if (sprite) {
			std::string currentSprite = sprite->ssheet;
			if (currentSprite == "player-idle-sword") {
				sprite->ssheet = "player-idle-sword-attack";
			} else if (currentSprite == "player-idle-back-sword") {
				sprite->ssheet = "player-idle-back-sword-attack";
			} else if (currentSprite == "player-right-idle-sword") {
				sprite->ssheet = "player-right-idle-sword-attack";
				// Preserve invert_y state
			}

			DINFOF("Sword attack! Changed sprite from %s to %s\n",
					currentSprite.c_str(), sprite->ssheet.c_str());
		}

		// Check for enemies in sword range
		auto enemies = ecs::ecs.query<TRANSFORM, SPRITE>();
		for (auto* entity : enemies) {
			auto [sprite] = ecs::ecs.component().fetch<SPRITE>(entity);
			if (!sprite) continue;

			// Check if this is an enemy entity by looking at sprite sheet name
			if (sprite->ssheet.find("enemy") != std::string::npos) {
				if (checkSwordEnemyCollision(player, entity)) {
					// Sword does 3 damage per hit
					handleEnemyHit(entity, 3);
				}
			}
		}
	}
	else if (g_weaponState.currentWeapon == WEAPON_HANDGUN) {
		g_weaponState.handgunAmmo--;

		// Always play gun sound
		playGameSound(PLAYER_GUN_SHOT);

		if (g_weaponState.handgunAmmo > 3 && rand() % 10 == 0) {
			// Small delay before playing voice line so gun sound is heard first
			usleep(100000); // 100ms delay
			playGameSound(PLAYER_HASTA_LA_VISTA);
		}

		// Create a bullet projectile
		createProjectile(transform->pos[0], transform->pos[1], dirX, dirY, 
			PROJECTILE_BULLET);
		g_weaponState.cooldownPeriod = std::chrono::milliseconds(500);
	}
	else if (g_weaponState.currentWeapon == WEAPON_ROCKET) {
		g_weaponState.rocketAmmo--;

		// Always play rocket sound
		playGameSound(PLAYER_ROCKET_LAUNCH);

		// Create a rocket projectile
		createProjectile(transform->pos[0], transform->pos[1], dirX, dirY, 
			PROJECTILE_ROCKET);
		g_weaponState.cooldownPeriod = std::chrono::milliseconds(1500);
	}
	else {
		return false;
	}

	// Update firing timestamp
	g_weaponState.lastFireTime = std::chrono::high_resolution_clock::now();

	DINFOF("Fired weapon: %d, Handgun ammo: %d, Rocket ammo: %d\n",
			g_weaponState.currentWeapon, g_weaponState.handgunAmmo, 
			g_weaponState.rocketAmmo);

	return true;
}

// Update player sprites based on current weapon and direction
void updateWeaponSprites(const ecs::Entity* player) {
	if (!player) return;

	auto [sprite, physics] = ecs::ecs.component().fetch<SPRITE, PHYSICS>(player);
	if (!sprite || !physics) return;

	// Get current direction from last player direction
	PlayerDirection dir = lastPlayerDirection;

	// Debug info to see what sprite we're trying to use
	DINFOF("Current weapon: %d, Direction: %d\n", 
			g_weaponState.currentWeapon, dir);

	// Select the correct sprite based on weapon and direction
	bool isMoving = (fabs(physics->vel[0]) > 0.1f || fabs(physics->vel[1]) > 0.1f);
	std::string spriteName;

	// Handle each weapon type separately to ensure exact naming matches
	switch (g_weaponState.currentWeapon) {
		case WEAPON_NONE:
			// For no weapon, use standard movement sprites
			if (!physics->vel[0] && !physics->vel[1]) {
				// Idle sprites
				switch (dir) {
					case DIR_UP:
						spriteName = "player-back-idle";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-idle";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right-idle";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-idle";
						sprite->invert_y = false;
						break;
				}
			} else {
				// Movement sprites
				switch (dir) {
					case DIR_UP:
						spriteName = "player-back";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-front";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right";
						sprite->invert_y = false;
						break;
				}
			}
			break;


		case WEAPON_SWORD:
			DINFO("Setting SWORD sprite\n");
			if (!isMoving) {
				// Idle sprites with sword
				switch (dir) {
					case DIR_UP:
						spriteName = "player-idle-back-sword";
						break;
					case DIR_DOWN:
						spriteName = "player-idle-sword";
						break;
					case DIR_LEFT:
						spriteName = "player-right-idle-sword";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-idle-sword";
						sprite->invert_y = false;
						break;
				}
			} else {
				// Movement sprites with sword
				switch (dir) {
					case DIR_UP:
						spriteName = "player-back-sword";
						break;
					case DIR_DOWN:
						spriteName = "player-front-sword";
						break;
					case DIR_LEFT:
						spriteName = "player-right-sword";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-sword";
						sprite->invert_y = false;
						break;
				}
			}
			break;

		case WEAPON_HANDGUN:
			// For handgun, use exact sprite names from loadWeaponSprites
			if (!physics->vel[0] && !physics->vel[1]) {
				// Idle sprites with gun
				switch (dir) {
					case DIR_UP:
						spriteName = "player-idle-back-gun";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-idle-gun";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right-idle-gun";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-idle-gun";
						sprite->invert_y = false;
						break;
				}
			} else {
				// Movement sprites with gun
				switch (dir) {
					case DIR_UP:
						spriteName = "player-back-gun";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-front-gun";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right-gun";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-gun";
						sprite->invert_y = false;
						break;
				}
			}
			break;

		case WEAPON_ROCKET:
			// For rocket launcher, use exact sprite names
			if (!physics->vel[0] && !physics->vel[1]) {
				// Idle sprites with rocket
				switch (dir) {
					case DIR_UP:
						spriteName = "player-idle-back-rocket";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-idle-rocket";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right-idle-rocket";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-idle-rocket";
						sprite->invert_y = false;
						break;
				}
			} else {
				// Movement sprites with rocket
				switch (dir) {
					case DIR_UP:
						spriteName = "player-back-rocket";
						sprite->invert_y = false;
						break;
					case DIR_DOWN:
						spriteName = "player-front-rocket";
						sprite->invert_y = false;
						break;
					case DIR_LEFT:
						spriteName = "player-right-rocket";
						sprite->invert_y = true;
						break;
					case DIR_RIGHT:
						spriteName = "player-right-rocket";
						sprite->invert_y = false;
						break;
				}
			}
			break;

	}

	// Check if sprite exists before setting it
	auto spriteIter = ssheets.find(spriteName);
	if (spriteIter != ssheets.end()) {
		// Sprite exists, set it
		sprite->ssheet = spriteName;
		DINFOF("Successfully set sprite to: %s\n", spriteName.c_str());
	} else {
		// Sprite doesn't exist, use default
		DWARNF("Sprite not found: %s, keeping current sprite: %s\n", 
				spriteName.c_str(), sprite->ssheet.c_str());
	}
}

// Handle weapon input
void handleWeaponInput(int key, const ecs::Entity* player) {
	// Weapon switching keys
	if (key == XK_0) {
		DINFO("Switching to WEAPON_NONE\n");
		switchWeapon(WEAPON_NONE);
	} else if (key == XK_1) {
		DINFO("Switching to WEAPON_SWORD\n");
		switchWeapon(WEAPON_SWORD);
	} else if (key == XK_2) {
		DINFO("Switching to WEAPON_HANDGUN\n");
		switchWeapon(WEAPON_HANDGUN);
	} else if (key == XK_3) {
		DINFO("Switching to WEAPON_ROCKET\n");
		switchWeapon(WEAPON_ROCKET);
	} else if (key == XK_space) {
		// Fire weapon on space key
		DINFO("Attempting to fire weapon\n");
		fireWeapon();
	}

	// Update player sprites based on current weapon and direction
	updateWeaponSprites(player);
}


// Render ammo UI
void renderAmmoUI(int xres, int yres) {
	// Position for ammo display (bottom right corner, adjusted based on yres)
	int x = xres - 300;
	int y = yres - 550; 

	Rect r;
	r.left = x;
	r.bot = y;
	r.center = 0;

	// Display current weapon
	const char* weaponName;
	switch (g_weaponState.currentWeapon) {
		case WEAPON_NONE:
			weaponName = "NO WEAPON";
			break;
		case WEAPON_SWORD:
			weaponName = "SWORD";
			break;
		case WEAPON_HANDGUN:
			weaponName = "HANDGUN";
			break;
		case WEAPON_ROCKET:
			weaponName = "ROCKET";
			break;
		default:
			weaponName = "UNKNOWN";
			break;
	}

	ggprint13(&r, 20, 0xFFFFFFFF, "WEAPON: %s", weaponName);

	// Display ammo counts if using a gun
	if (g_weaponState.currentWeapon == WEAPON_HANDGUN) {
		r.bot -= 20;
		ggprint13(&r, 20, 0xFFFFFFFF, "AMMO: %d", g_weaponState.handgunAmmo);
	} else if (g_weaponState.currentWeapon == WEAPON_ROCKET) {
		r.bot -= 20;
		ggprint13(&r, 20, 0xFFFFFFFF, "ROCKETS: %d", g_weaponState.rocketAmmo);
	}

	// Display weapon switch help
	r.bot -= 10;
	ggprint13(&r, 10, 0xCCCCCCFF, "0: NONE, 1: SWORD, 2: GUN, 3: ROCKET");
}


// Initialize the projectile system
void initializeProjectileSystem() {
	DINFO("Initializing projectile system...\n");

	// Initialize all projectiles as inactive
	for (int i = 0; i < MAX_PROJECTILES; i++) {
		projectiles[i].active = false;
	}

	// Load bullet textures if not already loaded
	SpriteSheetLoader loader {ssheets};

	// Load horizontal bullet
	loader.loadStatic("bullet-h", 
			loadTexture("./resources/textures/player/bullet.webp", true),
			{1, 1}, {8, 1});

	// Load vertical bullet (same texture but different dimensions)
	loader.loadStatic("bullet-v", 
			loadTexture("./resources/textures/player/bullet.webp", true),
			{1, 1}, {1, 8});

	// Load rocket projectile
	loader.loadStatic("rocket-bullet", 
			loadTexture("./resources/textures/player/rocket_bullet.webp", true),
			{1, 1}, {16, 3});

	DINFO("Projectile textures loaded successfully\n");
}


// Create a new projectile
void createProjectile(float startX, float startY, float dirX, float dirY, 
	ProjectileType type) {
	// Find first inactive projectile
	for (int i = 0; i < MAX_PROJECTILES; i++) {
		if (!projectiles[i].active) {
			DINFOF("Creating projectile at position: (%.2f, %.2f)\n", startX, 
				startY);

			// Calculate normalized direction vector
			float length = sqrt(dirX * dirX + dirY * dirY);
			if (length > 0) {
				dirX /= length;
				dirY /= length;
			} else {
				// Default direction if no direction provided
				dirX = 1.0f;
				dirY = 0.0f;
			}

			// Set projectile properties
			projectiles[i].active = true;
			projectiles[i].posX = startX;
			projectiles[i].posY = startY;
			projectiles[i].type = type;

			// Set speed and lifetime based on projectile type
			if (type == PROJECTILE_BULLET) {
				projectiles[i].speed = 800.0f; // Bullets are fast
				projectiles[i].maxLifetime = 1.0f; // Seconds before despawning
			} else if (type == PROJECTILE_ROCKET) {
				projectiles[i].speed = 400.0f; // Rockets slower than bullets
				projectiles[i].maxLifetime = 3.0f; // Rockets last longer
			}

			projectiles[i].velX = dirX * projectiles[i].speed;
			projectiles[i].velY = dirY * projectiles[i].speed;
			projectiles[i].lifetime = 0.0f;

			return;
		}
	}

	// If we get here, we couldn't create a projectile (all slots full)
	DWARN("Could not create projectile - all slots are full\n");
}

// Update all active projectiles
void updateProjectiles(float dt) {
	for (int i = 0; i < MAX_PROJECTILES; i++) {
		if (projectiles[i].active) {
			// Update position based on velocity
			projectiles[i].posX += projectiles[i].velX * dt;
			projectiles[i].posY += projectiles[i].velY * dt;

			// Update lifetime and check if expired
			projectiles[i].lifetime += dt;
			if (projectiles[i].lifetime >= projectiles[i].maxLifetime) {
				projectiles[i].active = false;
				continue;
			}

			// Check for collision with enemies
			auto enemies = ecs::ecs.query<TRANSFORM, SPRITE>();

			if (g_debugCollisions) {
				DINFOF("Checking projectile %d against %zu enemies\n", i, 
					enemies.size());
			}

			// Update in updateProjectiles function
			for (auto* entity : enemies) {
				auto [sprite] = ecs::ecs.component().fetch<SPRITE>(entity);
				if (!sprite) continue;

				// Check if this is an enemy entity
				if (sprite->ssheet.find("enemy") != std::string::npos) {
					// Skip dead enemies (already using death sprite)
					if (sprite->ssheet == "enemy-dead") {
						if (g_debugCollisions) {
							DINFO("Skipping collision - enemy is already dead \n");
						}
						continue;
					}

					// Check if the enemy has health and is already at 0
					if (ecs::ecs.component().has<HEALTH>(entity)) {
						auto [health] = ecs::ecs.component().fetch<HEALTH>(entity);
						if (health && health->health <= 0) {
							if (g_debugCollisions) {
								DINFO("Skipping collision - enemy health is already 0\n");
							}
							continue;
						}
					}

					// Check for collision and handle hit
					if (checkProjectileEnemyCollision(projectiles[i], entity)) {
						int damage = (projectiles[i].type == PROJECTILE_BULLET) ? 5 : 15;

						DINFOF("Projectile hit! Type: %d, Damage: %d, Entity ID: %d\n", 
								projectiles[i].type, damage, entity->id);

						handleEnemyHit(entity, damage);
						projectiles[i].active = false;
						break;
					}
				}
			}
		}
	}
}

// Render all active projectiles
void renderProjectiles() {
	// Find bullet sprites first
	auto bulletHSprite = ssheets.find("bullet-h");
	auto bulletVSprite = ssheets.find("bullet-v");
	auto rocketSprite = ssheets.find("rocket-bullet");

	if (bulletHSprite == ssheets.end() || bulletVSprite == ssheets.end()) {
		DWARN("Bullet sprite not found in ssheets map\n");
		return;
	}

	if (rocketSprite == ssheets.end()) {
		DWARN("Rocket sprite not found in ssheets map\n");
	}

	for (int i = 0; i < MAX_PROJECTILES; i++) {
		if (projectiles[i].active) {
			v2f pos = {projectiles[i].posX, projectiles[i].posY};

			if (projectiles[i].type == PROJECTILE_BULLET) {
				// Determine which bullet sprite to use based on velocity
				std::shared_ptr<SpriteSheet> spriteToUse;
				if (fabs(projectiles[i].velX) > fabs(projectiles[i].velY)) {
					spriteToUse = bulletHSprite->second;// Use horizontal sprite
				} else {
					spriteToUse = bulletVSprite->second;// Use vertical sprite
				}

				// Use shared sprite sheet render function
				v2f scale = {2.0f, 2.0f};
				spriteToUse->render(0, pos, scale, false);

			} else if (projectiles[i].type == PROJECTILE_ROCKET && 
				rocketSprite != ssheets.end()) {
				// For rockets, we need to load different rocket 
				// sprites for different directions
				v2f scale = {2.0f, 2.0f};
				bool isVertical = fabs(projectiles[i].velY) > 
				fabs(projectiles[i].velX);

				if (isVertical) {
					glPushMatrix();

					glTranslatef(pos[0], pos[1], 0.0f);

					// Apply rotation based on direction
					float angle = (projectiles[i].velY > 0) ? 90.0f : -90.0f;
					glRotatef(angle, 0.0f, 0.0f, 1.0f);

					GLuint texId = *rocketSprite->second->tex->tex;
					glBindTexture(GL_TEXTURE_2D, texId);
					glEnable(GL_ALPHA_TEST);
					glAlphaFunc(GL_GREATER, 0.9f);
					glColor4ub(255, 255, 255, 255);

					// Get the rocket sprite dimensions (original is 16x3)
					float rocketWidth = 3.0f;  // Original width
					float rocketHeight = 16.0f;  // Original height 

					// When rotated, we swap width and height but maintain 
					// correct aspect ratio
					float halfWidth = rocketHeight * scale[0] / 2.0f;
					float halfHeight = rocketWidth * scale[1] / 2.0f;

					glBegin(GL_QUADS);
					// Bottom-left corner
					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(-halfWidth, -halfHeight);

					// Bottom-right corner
					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(halfWidth, -halfHeight);

					// Top-right corner
					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(halfWidth, halfHeight);

					// Top-left corner
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(-halfWidth, halfHeight);
					glEnd();

					glDisable(GL_ALPHA_TEST);

					// Restore the matrix state
					glPopMatrix();
				} else {
					// Horizontal movement - rendering with potential inversion
					bool flipHorizontal = projectiles[i].velX < 0;
					rocketSprite->second->render(0, pos, scale, flipHorizontal);
				}
			}
		}
	}
}


// Load enemy sprites including death animation
void loadEnemySprites() {
	DINFO("Loading enemy death sprite...\n");

	// Check if the death sprite is already loaded
	auto it = ssheets.find("enemy-dead");
	if (it != ssheets.end()) {
		DINFO("Enemy death sprite already loaded.\n");
		return;
	}

	SpriteSheetLoader loader {ssheets};

	// Load enemy death animation sprite
	auto tex = loadTexture("./resources/textures/enemies/gobo-dead.webp", true);
	if (!tex) {
		DERROR("Failed to load enemy death texture!\n");
		return;
	}

	loader.loadStatic("enemy-dead", tex, {1, 1}, {42, 36}, false);

	// Verify the sprite was loaded correctly
	auto verifyIt = ssheets.find("enemy-dead");
	if (verifyIt != ssheets.end()) {
		DINFO("Enemy death sprite loaded successfully!\n");
	} else {
		DERROR("Failed to load enemy death sprite!\n");
	}
}



// Check if a projectile collides with an enemy
bool checkProjectileEnemyCollision(const Projectile& projectile, 
	const ecs::Entity* enemy) {
	if (!enemy || !projectile.active) return false;

	auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(enemy);
	if (!transform) return false;

	// Get enemy position and size
	float enemyX = transform->pos[0];
	float enemyY = transform->pos[1];
	float enemyRadius = 20.0f; // Adjust based on your enemy size

	// Calculate distance between projectile and enemy
	float dx = projectile.posX - enemyX;
	float dy = projectile.posY - enemyY;
	float distance = sqrt(dx * dx + dy * dy);

	// Debug output
	if (g_debugCollisions) {
		DINFOF("COLLISION CHECK: Projectile(%.1f,%.1f) Enemy(%.1f,%.1f) Distance:%.1f Radius:%.1f\n", 
				projectile.posX, projectile.posY, enemyX, enemyY, distance, enemyRadius);
	}

	// Check collision (simple circle-circle collision)
	float projectileRadius = 5.0f; // Adjust based on your projectile size
	bool collided = distance < (enemyRadius + projectileRadius);

	if (collided && g_debugCollisions) {
		DINFO("COLLISION DETECTED!\n");
	}

	return collided;
}



// Check if player's sword swing collides with an enemy
bool checkSwordEnemyCollision(const ecs::Entity* player, const ecs::Entity* enemy) {
	if (!player || !enemy) return false;

	// Only check collision if the player is actively swinging sword
	if (g_weaponState.currentWeapon != WEAPON_SWORD) return false;

	auto [playerTransform] = ecs::ecs.component().fetch<TRANSFORM>(player);
	auto [enemyTransform] = ecs::ecs.component().fetch<TRANSFORM>(enemy);

	if (!playerTransform || !enemyTransform) return false;

	// Sword range
	float swordRange = 50.0f;

	// Calculate distance between player and enemy
	float dx = playerTransform->pos[0] - enemyTransform->pos[0];
	float dy = playerTransform->pos[1] - enemyTransform->pos[1];
	float distance = sqrt(dx * dx + dy * dy);

	// Check if enemy is within sword range
	return distance < swordRange;
}

// Update handleEnemyHit to work with entity's actual components
void handleEnemyHit(const ecs::Entity* enemy, int damage) {
	if (!enemy) {
		DINFO("ERROR: Enemy pointer is null in handleEnemyHit\n");
		return;
	}

	// Check if the enemy has health component
	if (ecs::ecs.component().has<HEALTH>(enemy)) {
		auto [health] = ecs::ecs.component().fetch<HEALTH>(enemy);

		if (health) {
			// Apply damage to the enemy's actual health component
			float oldHealth = health->health;
			health->health -= damage;
			g_totalHits++;

			DINFOF("ENEMY HIT %d! Entity ID: %d, Damage: %d, Health: %.1f/%.1f\n", 
					g_totalHits, enemy->id, damage, health->health, health->max);

			// Play hit sound
			playGameSound(ENEMY_HIT);

			// Check if enemy is now dead
			if (health->health <= 0 && oldHealth > 0) {
				DINFOF("ENEMY KILLED! Entity ID: %d, Total hits: %d\n", 
					enemy->id, g_totalHits);

				// Get enemy sprite
				auto [sprite] = ecs::ecs.component().fetch<SPRITE>(enemy);
				if (sprite) {
					// Store original sprite name for debugging
					std::string oldSprite = sprite->ssheet;

					// Change sprite to death sprite
					sprite->ssheet = "enemy-dead";
					sprite->frame = 0;

					DINFOF("Changed enemy sprite from '%s' to 'enemy-dead'\n", 
							oldSprite.c_str());
				}

				// Play death sound
				playGameSound(ENEMY_DEATH);
			}
		}
	} 
	else {
		// Fallback to using global enemy state if no health component
		DINFO("Enemy doesn't have Health component, fallback damage system\n");

		// Decrement global enemy state
		g_enemyState.health -= damage;
		g_totalHits++;

		DINFOF("ENEMY HIT %d! Entity ID: %d, Damage: %d, Global Health: %d/%d\n", 
				g_totalHits, enemy->id, damage, g_enemyState.health, 
				g_enemyState.maxHealth);

		// Play hit sound
		playGameSound(ENEMY_HIT);

		// Check if enemy is dead
		if (g_enemyState.health <= 0 && !g_enemyState.isDead) {
			DINFOF("ENEMY KILLED! Entity ID: %d, Total hits: %d\n", enemy->id, 
				g_totalHits);
			g_enemyState.isDead = true;

			// Get enemy sprite
			auto [sprite] = ecs::ecs.component().fetch<SPRITE>(enemy);
			if (sprite) {
				// Change sprite to death animation
				sprite->ssheet = "enemy-dead";
				sprite->frame = 0;
			}

			// Play death sound
			playGameSound(ENEMY_DEATH);
		}
	}
}

// Update enemy state based on hit or death status
void updateEnemyState(float deltaTime) {
	// Update hit animation
	if (g_enemyState.isHit && !g_enemyState.isDead) {
		g_enemyState.hitTimer += deltaTime;

		// End hit state after a short duration
		if (g_enemyState.hitTimer > 0.2f) {
			g_enemyState.isHit = false;
		}
	}

	// Update death animation
	if (g_enemyState.isDead) {
		g_enemyState.deathTimer += deltaTime;

		// Added debug output to see death timer progress
		if (g_debugCollisions && (int)(g_enemyState.deathTimer * 10) % 10 == 0) {
			DINFOF("Enemy death timer: %.1f seconds\n", g_enemyState.deathTimer);
		}

		// Increase this value to keep the enemy dead longer - now 5 seconds
		if (g_enemyState.deathTimer > 5.0f) {
			DINFO("Enemy respawning after death animation\n");
			// Reset enemy for respawn
			g_enemyState.isDead = false;
			g_enemyState.health = g_enemyState.maxHealth;
			g_enemyState.deathTimer = 0.0f;
			g_totalHits = 0; // Reset hit counter

			// Find an enemy entity and reset its sprite
			auto enemies = ecs::ecs.query<TRANSFORM, SPRITE>();
			for (auto* entity : enemies) {
				auto [sprite] = ecs::ecs.component().fetch<SPRITE>(entity);
				if (sprite && sprite->ssheet == "enemy-dead") {
					// Reset sprite to original enemy sprite
					sprite->ssheet = "enemy-idle";
					DINFO("Reset enemy sprite from dead to idle\n");
					break;
				}
			}
		}
	}

	updateSwordAttackAnimation();
}

void testEnemySpriteChange(const ecs::Entity* enemy) {
	if (!enemy) {
		DINFO("Cannot test sprite change - enemy is null\n");
		return;
	}

	auto [sprite] = ecs::ecs.component().fetch<SPRITE>(enemy);
	if (!sprite) {
		DINFO("Cannot test sprite change - enemy has no sprite component\n");
		return;
	}

	DINFOF("Testing sprite change: Current sprite is '%s'\n", sprite->ssheet.c_str());

	// Try changing the sprite
	std::string oldSprite = sprite->ssheet;
	sprite->ssheet = "enemy-dead";

	DINFOF("Sprite changed from '%s' to '%s'\n", oldSprite.c_str(), 
	sprite->ssheet.c_str());
}

void initializeEnemySystem() {
	// Reset enemy state
	g_enemyState.health = 15;
	g_enemyState.maxHealth = 15;
	g_enemyState.isDead = false;
	g_enemyState.isHit = false;
	g_enemyState.hitTimer = 0.0f;
	g_enemyState.deathTimer = 0.0f;
	g_totalHits = 0;

	// Load enemy sprites
	loadEnemySprites();

	DINFO("Enemy system initialized\n");
}

void render_game_over_screen(int xres, int yres, int& restart_option) {
	// Create a dark overlay for the background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // Dark overlay with 80% opacity

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, yres);
	glVertex2i(xres, yres);
	glVertex2i(xres, 0);
	glEnd();

	// Draw a box for the game over message
	int boxWidth = 400;
	int boxHeight = 300; // Make taller for more content
	int boxX = (xres - boxWidth) / 2;
	int boxY = (yres - boxHeight) / 2;

	// Box background (dark red for game over)
	glColor4f(0.5f, 0.0f, 0.0f, 0.9f);
	glBegin(GL_QUADS);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	// Box border (white)
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(boxX, boxY);
	glVertex2i(boxX, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY + boxHeight);
	glVertex2i(boxX + boxWidth, boxY);
	glEnd();

	// Draw highlight box for the selected option
	int optionHeight = 40;
	int optionWidth = 200;
	int optionX = boxX + (boxWidth - optionWidth) / 2;

	if (restart_option == 0) {
		// RESTART option is selected - draw highlight box
		int restartY = yres / 2 - 15 - optionHeight/2;

		glColor4f(0.0f, 1.0f, 0.5f, 0.4f); // Green highlight
		glBegin(GL_QUADS);
		glVertex2i(optionX, restartY);
		glVertex2i(optionX, restartY + optionHeight);
		glVertex2i(optionX + optionWidth, restartY + optionHeight);
		glVertex2i(optionX + optionWidth, restartY);
		glEnd();

		// Add white border
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2i(optionX, restartY);
		glVertex2i(optionX, restartY + optionHeight);
		glVertex2i(optionX + optionWidth, restartY + optionHeight);
		glVertex2i(optionX + optionWidth, restartY);
		glEnd();
	} else {
		// EXIT option is selected - draw highlight box
		int exitY = yres / 2 - 55 - optionHeight/2;

		glColor4f(1.0f, 0.3f, 0.3f, 0.4f); // Red highlight
		glBegin(GL_QUADS);
		glVertex2i(optionX, exitY);
		glVertex2i(optionX, exitY + optionHeight);
		glVertex2i(optionX + optionWidth, exitY + optionHeight);
		glVertex2i(optionX + optionWidth, exitY);
		glEnd();

		// Add white border
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2i(optionX, exitY);
		glVertex2i(optionX, exitY + optionHeight);
		glVertex2i(optionX + optionWidth, exitY + optionHeight);
		glVertex2i(optionX + optionWidth, exitY);
		glEnd();
	}

	glDisable(GL_BLEND);

	// Render text
	Rect r;
	r.left = xres / 2;
	r.bot = yres / 2 + 80;
	r.center = 1;

	// Game over title
	ggprint40(&r, 0, 0xFF0000, "GAME OVER");

	// Optional message indicating what caused game over
	r.bot = yres / 2 + 30;
	ggprint13(&r, 30, 0xFFFFFF, "You ran out of resources!");

	// Options
	r.bot = yres / 2 - 30;

	// Restart option (0)
	if (restart_option == 0) {
		ggprint17(&r, 40, 0x00FF99FF, ">> RESTART <<");
	} else {
		ggprint17(&r, 40, 0x00FFFFFF, "RESTART"); // Normal color
	}

	r.bot = yres / 2 - 70;
	// Exit option (1)
	if (restart_option == 1) {
		ggprint17(&r, 40, 0x00FF99FF, ">> EXIT <<"); // Highlighted with arrows
	} else {
		ggprint17(&r, 40, 0x00FFFFFF, "EXIT"); // Normal color
	}

	r.bot = yres / 2 - 120;
	ggprint13(&r, 0, 0xCCCCCC, "Use UP/DOWN to select, ENTER to confirm");

	// Debug output showing selection state
	r.bot = 50; // Near bottom of screen
	r.left = 50;
	r.center = 0; // Left aligned
	ggprint13(&r, 0, 0xFFFFFF, "Selected option: %d (%s)", 
			restart_option, 
			restart_option == 0 ? "RESTART" : "EXIT");

	DINFOF("Game over screen rendered with restart_option: %d\n", restart_option);
}

int handle_game_over_keys(int key, GameState &state, int &restart_option) {
	switch(key) {
		case XK_Up:
		case XK_Down:
			// Toggle between restart and exit options
			playGameSound(MENU_CLICK);
			restart_option = 1 - restart_option; // Toggle between 0 and 1
			break;

		case XK_Return:
			playGameSound(MENU_CLICK);
			if (restart_option == 0) {
				// Restart option selected - go back to menu
				state = MENU;
				updateAudioState(state);
				return GAMEOVER_ACTION_RESTART;
			} else {
				// Exit option selected
				return GAMEOVER_ACTION_EXIT;
			}
			break;

		case XK_Escape:
			// Exit the game
			playGameSound(MENU_CLICK);
			return GAMEOVER_ACTION_EXIT;
	}
	return GAMEOVER_ACTION_NONE;
}

void updateSwordAttackAnimation() {
    if (!g_swordAttackState.isAttacking) {
        return;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto attackElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - g_swordAttackState.attackStartTime);

    // Check if attack animation should end
    if (attackElapsed >= g_swordAttackState.attackDuration) {
        g_swordAttackState.isAttacking = false;

        // Reset sprite to non-attack version
        auto [sprite] = ecs::ecs.component().fetch<SPRITE>(player);
        if (sprite) {
            std::string currentSprite = sprite->ssheet;
            if (currentSprite == "player-idle-sword-attack") {
                sprite->ssheet = "player-idle-sword";
            } else if (currentSprite == "player-idle-back-sword-attack") {
                sprite->ssheet = "player-idle-back-sword";
            } else if (currentSprite == "player-right-idle-sword-attack") {
                sprite->ssheet = "player-right-idle-sword";
                // Preserve invert_y state
            }

            DINFOF("Sword attack complete! Reset sprite to %s\n", sprite->ssheet.c_str());
        }
    }
}
