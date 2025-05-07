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

MusicType AudioManager::getCurrentMusic() const {
	return currentMusic;
}

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
	success &= loadSound(FOOTSTEP_GRASS, SFX_GAMEPLAY_PATH + "footstep_grass.wav");
	success &= loadSound(FOOTSTEP_SNOW, SFX_GAMEPLAY_PATH + "footstep_snow.wav");
	success &= loadSound(FOOTSTEP_LAVA, SFX_GAMEPLAY_PATH + "footstep_lava.wav");
	
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

	// Add Space Music
	success &= loadMusic(SPACE_MUSIC, MUSIC_GAMEPLAY_PATH + "background_space.wav");

	return success;
}

// Play a sound effect
void AudioManager::playSound(SoundType sound) {
#ifdef USE_OPENAL_SOUND
    if (!audioInitialized || !soundEnabled) return;

    auto it = soundSources.find(sound);
    if (it != soundSources.end()) {
        ALint state;
        alGetSourcei(it->second, AL_SOURCE_STATE, &state);
        
        // For footstep sounds specifically, handle differently
        if (sound == FOOTSTEP_GRASS || sound == FOOTSTEP_SNOW || sound == FOOTSTEP_LAVA) {
            // If already playing, don't interrupt it
            if (state == AL_PLAYING) {
                return;
            }
            
            // Set footstep-specific properties
            alSourcef(it->second, AL_GAIN, soundVolume / 100.0f * 1.2f); // Slightly louder
            alSourcef(it->second, AL_PITCH, 0.9f + ((float)rand() / RAND_MAX) * 0.2f); // Random pitch variation
        }
        
        alSourcePlay(it->second);
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
		case SPLASH:
		case MENU:
			playGameMusic(MENU_MUSIC);
			break;
		case PLAYING:
			playGameMusic(GAME_MUSIC);
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

int handle_pause_keys(int key, GameState &state, GameState &previous_state, int &selected_option, MusicType previous_music) {
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
    DINFO("Make sure 'player-idle-back' and 'player-right-stand' are loaded in texture loader.\n");
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

    DINFOF("Setting player idle direction based on last direction: %d\n", lastPlayerDirection);

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
        if (temp >= -10.0f && temp <= 10.0f && humidity >= 0.2f && humidity <= 0.6f) {
            soundType = FOOTSTEP_SNOW;
        }
        else if (temp >= 40.0f || (temp >= 30.0f && humidity <= 0.2f)) {
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
            g_footstepState.lastFootstepTime = currentTime - g_footstepState.footstepInterval;
        }
    }

    // Play footstep sound at regular intervals while walking
    if (g_footstepState.isWalking && timeSinceLastFootstep >= g_footstepState.footstepInterval) {
        DINFOF("Playing footstep! Time since last: %ld ms\n", timeSinceLastFootstep.count());
        playFootstepSound();
        g_footstepState.lastFootstepTime = currentTime;
    }
}
