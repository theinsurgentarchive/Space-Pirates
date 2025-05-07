#include <cstdint>
#include <bitset>
#include <array>
#include <memory>
#include <vector>
#include <deque>
#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <climits>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>

#include <GL/glx.h>
#include <dirent.h>
#include <unistd.h>

#include "image.h"
#include "jlo.h"
#include "balrowhany.h"
#include "fonts.h"
#include "image.h"

std::array<Biome, 5> biomes = {
    Biome(FOREST, v2f{10.0f, 35.0f}, v2f{0.2f, 1.0f}, 
        "Moderate temperature, lush tree-filled areas."),
    Biome(DESERT, v2f{30.0f, 85.0f}, v2f{0.0f, 0.8f}, 
        "Hot and dry, minimal precipitation and vegetation."),
    Biome(TAIGA, v2f{-40.0f, 10.0f}, v2f{0.1f, 0.8f}, 
        "Cold, coniferous forest with long winters."),
    Biome(MEADOW, v2f{0.0f, 20.0f}, v2f{0.4f, 1.0f}, 
        "Cold, wet grasslands often found in mountainous regions."),
    Biome(HELL, v2f{40.0f, _MAX_TEMPERATURE}, v2f{0.0f, 0.6f}, 
        "Extreme temperatures and dry conditions, inhospitable to life.")
};


void show_jlo(Rect* r)
{
	ggprint8b(r, 16, 0x00ff0000, "Developer - Justin Lo");
}

// ----- Globals ----- 
std::random_device rd;
std::mt19937 generator(rd());
v2i dirs[4] {{0,1},{0,-1},{-1,0},{1,0}};
Direction opposite[4] {BOTTOM,TOP,RIGHT,LEFT};
extern float gold;
extern std::unordered_map<
std::string,
	std::shared_ptr<SpriteSheet>> ssheets;
extern std::unordered_map<
std::string,
	std::shared_ptr<Texture>> textures;
extern std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);
extern const Camera* c;
extern std::vector<Collision> cols;
extern std::atomic<bool> done;
extern ThreadPool* pool;
extern const ecs::Entity* player;
extern const ecs::Entity* spaceship;
struct PlayerState {
	bool isMoving;
	std::string lastMovingSprite;
	std::string lastIdleSprite;
	bool lastInvertY;
} g_playerState = {false, "player-idle", "player-idle", false};
void loadTextures(
		std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets)
{
    SpriteSheetLoader loader {ssheets};
    loader
    .loadStatic("cherry-001", 
        loadTexture(
            "./resources/textures/decorations/cherry-001.png", true))
    .loadStatic("cherry-002", 
        loadTexture(
            "./resources/textures/decorations/cherry-002.png", true))
    .loadStatic("chestnut-001", 
        loadTexture(
            "./resources/textures/decorations/chestnut-001.png", true))
    .loadStatic("chestnut-002", 
        loadTexture(
            "./resources/textures/decorations/chestnut-002.png", true))
    .loadStatic("chestnut-003", 
        loadTexture(
            "./resources/textures/decorations/chestnut-003.png", true))
    .loadStatic("pine-001", 
        loadTexture(
            "./resources/textures/decorations/pine-001.png", true))
    .loadStatic("pine-002", 
        loadTexture(
            "./resources/textures/decorations/pine-002.png", true))
    .loadStatic("pine-003", 
        loadTexture(
            "./resources/textures/decorations/pine-003.png", true))
    .loadStatic("pine-004", 
        loadTexture(
            "./resources/textures/decorations/pine-004.png", true))
    .loadStatic("pine-005", 
        loadTexture(
            "./resources/textures/decorations/pine-005.png", true))
    .loadStatic("cactus-001", 
        loadTexture(
            "./resources/textures/decorations/cactus-001.png", true))
    .loadStatic("cactus-002", 
        loadTexture(
            "./resources/textures/decorations/cactus-002.png", true))
    .loadStatic("cactus-003", 
        loadTexture(
            "./resources/textures/decorations/cactus-003.png", true))
    .loadStatic("cactus-004", 
        loadTexture(
            "./resources/textures/decorations/cactus-004.png", true))
    .loadStatic("cactus-005", 
        loadTexture(
            "./resources/textures/decorations/cactus-005.png", true))
    .loadStatic("pine-001snow", 
        loadTexture(
            "./resources/textures/decorations/pine-001snow.png", true))
    .loadStatic("pine-002snow", 
        loadTexture(
            "./resources/textures/decorations/pine-002snow.png", true))
    .loadStatic("pine-003snow", 
        loadTexture(
            "./resources/textures/decorations/pine-003snow.png", true))
    .loadStatic("pine-004snow", 
        loadTexture(
            "./resources/textures/decorations/pine-004snow.png", true))
    .loadStatic("pine-005snow", 
        loadTexture(
            "./resources/textures/decorations/pine-005snow.png", true))
    .loadStatic("player-idle",
        loadTexture(
            "./resources/textures/player/idle.webp",true),
        {1,1},{18,32})
    .loadStatic("player-front",
        loadTexture(
            "./resources/textures/player/front.webp",true),
        {1,3},{26,32},true)
    .loadStatic("player-back",
        loadTexture(
            "./resources/textures/player/back.webp",true),
        {1,3},{26,32},true)
    .loadStatic("player-right",
        loadTexture(
            "./resources/textures/player/right.webp",true),
        {1,3},{26,32},true)
    .loadStatic("sand-001",
        loadTexture(
            "./resources/textures/tiles/sand-001.webp",false))
    .loadStatic("sand-002",
        loadTexture(
            "./resources/textures/tiles/sand-002.webp",false))
    .loadStatic("sand-003",
        loadTexture(
            "./resources/textures/tiles/sand-003.webp",false))
    .loadStatic("hell-001",
        loadTexture(
            "./resources/textures/tiles/hell-001.webp",false))
    .loadStatic("hell-002",
        loadTexture(
            "./resources/textures/tiles/hell-002.webp",false))
    .loadStatic("hell-003",
        loadTexture(
            "./resources/textures/tiles/hell-003.webp",false))
    .loadStatic("snow-001",
        loadTexture(
            "./resources/textures/tiles/snow-001.webp",false))
    .loadStatic("snow-002",
        loadTexture(
            "./resources/textures/tiles/snow-002.webp",false))
    .loadStatic("grass-001",
        loadTexture(
            "./resources/textures/tiles/grass-001.webp",false))
    .loadStatic("grass-002",
        loadTexture(
            "./resources/textures/tiles/grass-002.webp",false))
    .loadStatic("grass-004",
        loadTexture(
            "./resources/textures/tiles/grass-004.webp",false))
    .loadStatic("grass-005",
        loadTexture(
            "./resources/textures/tiles/grass-005.webp",false))
    .loadStatic("flower-001",
        loadTexture(
            "./resources/textures/tiles/flower-001.webp",false))
    .loadStatic("flower-002",
        loadTexture(
            "./resources/textures/tiles/flower-002.webp",false))
    .loadStatic("flower-003",
        loadTexture(
            "./resources/textures/tiles/flower-003.webp",false))
    .loadStatic("flower-004",
        loadTexture(
            "./resources/textures/tiles/flower-004.webp",false))
    .loadStatic("dirt-001",
        loadTexture(
            "./resources/textures/tiles/dirt-001.webp",false))
    .loadStatic("dirt-002",
        loadTexture(
            "./resources/textures/tiles/dirt-002.webp",false))
    .loadStatic("warm-water",
        loadTexture(
            "./resources/textures/tiles/warm-water.webp",false),
        {1,3},{16,16},true)
    .loadStatic("cold-water",
        loadTexture(
            "./resources/textures/tiles/cold-water.webp",false),
        {1,3},{16,16},true)
    .loadStatic("lava-001",
        loadTexture(
            "./resources/textures/tiles/lava-001.webp",false),
        {1,45},{16,16}, true)
    .loadStatic("chest",
        loadTexture(
            "./resources/textures/decorations/chest.png",false))
    .loadStatic("chest-open",
        loadTexture(
            "./resources/textures/decorations/chest_open.png",true))
	.loadStatic("player-back-idle",
		loadTexture(
			"./resources/textures/player/idle_back.webp",true)
		, {1,1},{18,32}, false)
	.loadStatic("player-right-idle",
		loadTexture(
			"./resources/textures/player/right_stand.webp",true
		), {1,1},{26,32}, false);
}

void checkRequiredSprites() {
    DINFO("Checking required sprites...\n");
    
    auto checkSprite = [](const std::string& name) {
        auto it = ssheets.find(name);
        if (it == ssheets.end()) {
            DERRORF("Required sprite '%s' is missing!\n", name.c_str());
            return false;
        }
        DINFOF("Sprite '%s' found.\n", name.c_str());
        return true;
    };
    
    bool allFound = true;
    allFound &= checkSprite("player-idle");
    allFound &= checkSprite("player-idle-back");
    allFound &= checkSprite("player-right-stand");
    allFound &= checkSprite("player-front");
    allFound &= checkSprite("player-back");
    allFound &= checkSprite("player-right");
    
    if (!allFound) {
        DWARN("Some required sprites are missing!\n");
    } else {
        DINFO("All required sprites are available.\n");
    }
}

std::shared_ptr<Texture> loadTexture(
		const std::string& file_name, 
		bool alpha)
{
	static std::vector<std::string> img_extensions 
	{".webp",".png",".jpeg",".jpg",".gif"};
	std::ifstream file (file_name);
	if (!file.is_open()) {
		DERRORF("file does not exist: %s\n",file_name.c_str());
		return nullptr;
	}

	std::string ext;
	for (auto& extension : img_extensions) {
		auto length = file_name.length() - extension.length();
		if (extension == file_name.substr(length)) {
			ext = extension;
			break;
		}
	} 
	if (ext.empty()) {
		DWARNF("file does not have a valid extension: %s",
				file_name.c_str());
		return nullptr;
	}
	Image img {file_name.c_str()};
	auto tex = std::make_shared<Texture>(
			v2u{img.width,img.height},
			alpha);
	glGenTextures(1,tex->tex.get());
	glBindTexture(GL_TEXTURE_2D,*tex->tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			alpha ? GL_RGBA : GL_RGB, 
			img.width, 
			img.height, 
			0, 
			alpha ? GL_RGBA : GL_RGB, 
			GL_UNSIGNED_BYTE, 
			alpha ? buildAlphaData(&img).get() : img.data.get());


	DINFOF("loaded texture: %s\n",file_name.c_str());
	return tex;
}

////////////////////////////////////////////////////////////////////////////////

Biome::Biome(
		BiomeType type, 
		const v2f& temperature, 
		const v2f& humidity,
		const std::string& description) 
	: 
		type{type}, 
		temperature{temperature}, 
		humidity{humidity},
		description{description}
{
}

std::unordered_map<std::string, wfc::TileMeta> Biome::tiles()
{
    std::unordered_map<std::string, wfc::TileMeta> tile_map;    
    switch (type) {
        case FOREST:
            tile_map.insert({"0", wfc::TileBuilder{0.05, "dirt-001"}
                .omni("1").omni("2").omni("3")
                .coefficient("1", 0.3).coefficient("2", 0.3)
                .coefficient("3", 0.1)
                .build()});
            tile_map.insert({"1", wfc::TileBuilder{0.4, "grass-001"}
                .omni("0").omni("1").omni("2")
                .coefficient("0", 0.3).coefficient("1", 0.3)
                .coefficient("2", 0.3)
                .build()});
            tile_map.insert({"2", wfc::TileBuilder{0.4, "grass-002"}
                .omni("0").omni("1").omni("2")
                .coefficient("0", 0.3).coefficient("1", 0.3)
                .coefficient("2", 0.3)
                .build()});
            tile_map.insert({"3", wfc::TileBuilder{0.1, "cold-water"}
                .omni("0").omni("3")
                .coefficient("0", 0.25).coefficient("3", 0.5)
                .build()});
            tile_map.insert({"4", wfc::TileBuilder{0.05, "sand-003"}
                .omni("0").omni("4")
                .coefficient("0", 0.3).coefficient("4", 0.3)
                .build()});
            break;

        case DESERT:
            tile_map.insert({"0", wfc::TileBuilder{0.3, "sand-001"}
                .omni("1").omni("2").omni("3")
                .coefficient("1", 0.3).coefficient("2", 0.3)
                .coefficient("3", 0.1)
                .build()});
            tile_map.insert({"1", wfc::TileBuilder{0.3, "sand-002"}
                .omni("0").omni("1").omni("2")
                .coefficient("0", 0.3).coefficient("1", 0.3)
                .coefficient("2", 0.3)
                .build()});
            tile_map.insert({"2", wfc::TileBuilder{0.3, "sand-003"}
                .omni("0").omni("1").omni("2")
                .coefficient("0", 0.3).coefficient("1", 0.3)
                .coefficient("2", 0.3)
                .build()});
            tile_map.insert({"3", wfc::TileBuilder{0.1, "warm-water"}
                .omni("0").omni("3")
                .coefficient("0", 0.25).coefficient("3", 0.25)
                .build()});
            break;
		case TAIGA:
			tile_map.insert({"0", wfc::TileBuilder{0.3, "grass-004"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.4).coefficient("1", 0.4)
					.coefficient("2", 0.2).coefficient("3", 0.2)
					.coefficient("4", 0.1)
					.build()});
			tile_map.insert({"1", wfc::TileBuilder{0.3, "grass-005"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.5).coefficient("1", 0.5)
					.coefficient("2", 0.1).coefficient("3", 0.1)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"2", wfc::TileBuilder{0.3, "snow-001"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.6).coefficient("3", 0.6)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"3", wfc::TileBuilder{0.3, "snow-002"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.7).coefficient("3", 0.7)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"4", wfc::TileBuilder{0.1, "cold-water"}
					.omni({"0","1","4"})
					.coefficient("0", 0.15).coefficient("1", 0.15)
					.coefficient("4", 0.7).coefficient("2", 0.05)
					.coefficient("3", 0.05)
					.build()});
			tile_map.insert({"5", wfc::TileBuilder{0.2, "sand-001"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.1).coefficient("3", 0.1)
					.coefficient("4", 0.8)
					.build()});
			break;

		case MEADOW:
			tile_map.insert({"0", wfc::TileBuilder{0.1, "flower-001"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.4).coefficient("1", 0.4)
					.coefficient("2", 0.2).coefficient("3", 0.2)
					.coefficient("4", 0.1)
					.build()});
			tile_map.insert({"1", wfc::TileBuilder{0.1, "flower-002"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.5).coefficient("1", 0.5)
					.coefficient("2", 0.1).coefficient("3", 0.1)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"2", wfc::TileBuilder{0.1, "flower-003"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.6).coefficient("3", 0.6)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"3", wfc::TileBuilder{0.1, "flower-004"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.7).coefficient("3", 0.7)
					.coefficient("4", 0.05)
					.build()});
			tile_map.insert({"4", wfc::TileBuilder{0.4, "grass-004"}
					.omni({"0","1","4"})
					.coefficient("0", 0.15).coefficient("1", 0.15)
					.coefficient("4", 0.7).coefficient("2", 0.05)
					.coefficient("3", 0.05)
					.build()});
			tile_map.insert({"5", wfc::TileBuilder{0.3, "grass-005"}
					.omni({"0","1","2","3"})
					.coefficient("0", 0.1).coefficient("1", 0.1)
					.coefficient("2", 0.1).coefficient("3", 0.1)
					.coefficient("4", 0.8)
					.build()});
			break;

		case HELL:
			tile_map.insert({"0", wfc::TileBuilder{0.5, "hell-001"}
					.omni({"1","2","3"})
					.coefficient("1", 0.3).coefficient("2", 0.3)
					.build()});
			tile_map.insert({"1", wfc::TileBuilder{0.3, "hell-002"}
					.omni({"0","1","2"})
					.coefficient("0", 0.3).coefficient("1", 0.3)
					.coefficient("2", 0.3)
					.build()});
			tile_map.insert({"2", wfc::TileBuilder{0.3, "hell-003"}
					.omni({"0","1","2"})
					.coefficient("0", 0.3).coefficient("1", 0.3)
					.coefficient("2", 0.3)
					.build()});
			tile_map.insert({"3", wfc::TileBuilder{0.4, "lava-001"}
					.omni({"0","3"})
					.coefficient("0", 0.25).coefficient("3", 0.5)
					.build()});
			break;
	}

	return tile_map;
}

std::vector<std::string> Biome::decor(BiomeType type)
{
	switch(type) {
		case FOREST:
			return {"pine-001","pine-002","pine-003","pine-004","pine-005"};
		case DESERT:
			return {"cactus-001","cactus-002","cactus-003",
				"cactus-004","cactus-005"};
		case TAIGA:
			return {"pine-001snow","pine-002snow","pine-003snow",
				"pine-004snow","pine005snow"};
		case MEADOW:
			return {"chestnut-001","chestnut-002","chestnut-003","pine-001",
				"pine-002","pine-003","pine-004","pine-005"};
		case HELL:
			return {};
	}
	return {""};
}

////////////////////////////////////////////////////////////////////////////////

/*
   Camera of the game, center of the camera is bound to 'pos_'.
   */

Camera::Camera(v2f& pos, v2u& dim, v2u& margin) : pos_{pos}, dim_{dim}, 
    margin_{margin} {}

void Camera::bind(v2f& pos)
{
	pos_ = std::ref(pos);
}

void Camera::bind(const ecs::Entity* entity)
{
	if (ecs::ecs.component().has<TRANSFORM>(entity)) {
		auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(entity);
		bind(transform->pos);
	}
}

void Camera::update() const
{
	glTranslatef((dim_.get()[0] >> 1) - pos_.get()[0], 
			(dim_.get()[1] >> 1) - pos_.get()[1],0);
}

bool Camera::visible(v2f curr) const
{
    float wh = dim_.get()[0] >> 1;
    float hh = dim_.get()[1] >> 1;
    u16 wm = margin_.get()[0] >> 1;
    u16 hm = margin_.get()[1] >> 1;
    v2f v1 {pos_.get()[0] - wh - wm, pos_.get()[1] - hh - hm};
    v2f v2 {pos_.get()[0] + wh + wm, pos_.get()[1] + hh + hm};
    return curr[0] >= v1[0] && curr[0] <= v2[0] &&
    curr[1] >= v1[1] && curr[1] <= v2[1];
}

v2u Camera::dim() const
{
	return dim_;
}

void Camera::visibleHelper(
		std::vector<const ecs::Entity*>& entities, 
		std::vector<const ecs::Entity*>& visible_entities, 
		const u32 start, 
		const u32 end,
		std::mutex& visible_entities_mutex) const
{
	std::vector<const ecs::Entity*> thread_visible;
	thread_visible.reserve(end - start);
	for (u32 i {start}; i < end; ++i) {
		auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(entities[i]);
		if (visible(transform->pos)) {
			thread_visible.emplace_back(entities[i]);
		}
	}

	std::lock_guard<std::mutex> lock(visible_entities_mutex);
	for (auto& entity : thread_visible) {
		visible_entities.push_back(entity);
	}
}

/*

   Returns all entities that are currently visible to the camera

   @return collection of entities

*/

std::vector<const ecs::Entity*> Camera::findVisible(
		std::vector<const ecs::Entity*>& entities,
		ThreadPool& pool) const
{
	const u32 nthreads = pool.size(); 
	const u32 entities_size = static_cast<u32>(entities.size());
	const u32 ethread = {entities_size / nthreads}; // entities per thread
	std::mutex task_mutex;
	std::condition_variable tasks_finished;
	std::atomic<u32> worker_count {nthreads};
	std::vector<const ecs::Entity*> visible_entities;
	for (u32 i {0}; i < nthreads; ++i) { //static partitioning
		u32 start {i * ethread};
		u32 end {i == (nthreads - 1) ? entities_size : start + ethread };
		pool.enqueue([
				this, 
				&entities, 
				&visible_entities, 
				start, 
				end, 
				&task_mutex, 
				&tasks_finished, 
				&worker_count
		]() {
		visibleHelper(entities, visible_entities, start, end, task_mutex);
		if (--worker_count == 0) {
		std::lock_guard<std::mutex> lock(task_mutex);
		tasks_finished.notify_all();
		}
		});
	}

	{
		std::unique_lock<std::mutex> lock(task_mutex);
		tasks_finished.wait(lock,[&worker_count]() { return worker_count == 0; });
	}

	return visible_entities;
}

////////////////////////////////////////////////////////////////////////////////

Texture::Texture(
		const Vec2<u16>& dim, 
		bool alpha) : 
	dim{dim}, 
	alpha{alpha}
{
	tex = std::make_unique<GLuint>();
}

////////////////////////////////////////////////////////////////////////////////

SpriteSheetLoader::SpriteSheetLoader(
		std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets) 
	: 
		ssheets_{ssheets}
{
}

SpriteSheetLoader& SpriteSheetLoader::loadStatic(
		const std::string& key, 
		const std::shared_ptr<Texture>& tex,
		const v2u& frame_dim,
		const v2u& sprite_dim,
		bool animated)
{
	ssheets_.insert({key,std::make_shared<SpriteSheet>(frame_dim,
				sprite_dim[0] == 0 && sprite_dim[1] == 0 ? tex->dim : sprite_dim,tex,
				animated)});
	return *this;
}

////////////////////////////////////////////////////////////////////////////////

SpriteSheet::SpriteSheet(
		const v2u& frame_dim,
		const v2u& sprite_dim,
		const std::shared_ptr<Texture> tex,
		bool animated)
	:
		frame_dim{frame_dim},
		sprite_dim{sprite_dim},
		tex{tex},
		animated{animated}
{
}

u16 SpriteSheet::maxFrames() const
{
	return sprite_dim[0] * sprite_dim[1];
}

void SpriteSheet::render(u16 frame, v2f pos, v2f scale, bool invertY)
{
	v2u f {
		static_cast<u16>(frame % frame_dim[1]), 
			static_cast<u16>(frame / frame_dim[1])
	};
	const u16 sw {static_cast<u16>(sprite_dim[0] * scale[0])};
	const u16 sh {static_cast<u16>(sprite_dim[1] * scale[1])};
	const u16 swhalf {static_cast<u16>(sw * 0.5)};
	const u16 shhalf {static_cast<u16>(sh * 0.5)};
	const u16 rows {frame_dim[0]};
	const u16 cols {frame_dim[1]};
	const float fx {(float) f[0] / cols};
	const float fy {(float) f[1] / rows};
	const float xo {(float) 1 / cols};
	const float xy {(float) 1 / rows};
	glBindTexture(GL_TEXTURE_2D,*tex->tex);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.9f);
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
	if (invertY) {
		glTexCoord2f(fx + xo, fy + xy);     
		glVertex2i(pos[0] - swhalf, pos[1] - shhalf);
		glTexCoord2f(fx + xo, fy);          
		glVertex2i(pos[0] - swhalf, pos[1] + shhalf);
		glTexCoord2f(fx, fy);     
		glVertex2i(pos[0] + swhalf, pos[1] + shhalf);
		glTexCoord2f(fx, fy + xy);
		glVertex2i(pos[0] + swhalf, pos[1] - shhalf);
	} else {
		glTexCoord2f(fx, fy + xy);     
		glVertex2i(pos[0] - swhalf, pos[1] - shhalf);
		glTexCoord2f(fx, fy);          
		glVertex2i(pos[0] - swhalf, pos[1] + shhalf);
		glTexCoord2f(fx + xo, fy);     
		glVertex2i(pos[0] + swhalf, pos[1] + shhalf);
		glTexCoord2f(fx + xo, fy + xy);
		glVertex2i(pos[0] + swhalf,pos[1] - shhalf);
	}
	glEnd();
	glDisable(GL_ALPHA_TEST);
}

////////////////////////////////////////////////////////////////////////////////

Biome selectBiome(float temperature, [[maybe_unused]]float humidity, u32 seed)
{
	std::vector<int> indices(biomes.size());
	std::iota(indices.begin(), indices.end(), 0);
	generator.seed(seed);
	std::shuffle(indices.begin(), indices.end(), generator);

	for (int i : indices) {
		auto& biome = biomes[i];
		if (temperature >= biome.temperature[0] && 
				temperature <= biome.temperature[1])
			return biome;
	}
	return biomes[0];
}


WorldGenerationSettings::WorldGenerationSettings(
		float temperature, 
		float humidity, 
		u16 radius, 
		u32 seed,
		int chest_count) : 
	temperature{temperature},
	humidity{humidity},
	radius{radius},
	biome_seed{seed},
	chest_count{chest_count}
{}

const ecs::Entity* createWorldTile(WorldGenerationSettings& settings, 
    v2u& sprite_dim, v2i& cell_pos, wfc::TileMeta& tile_meta)
{
    const ecs::Entity* entity = ecs::ecs.entity().checkout();
    auto [transform,sprite] = ecs::ecs.component()
        .assign<TRANSFORM,SPRITE>(entity);
    transform->scale = {3,3};
    transform->pos = settings.origin + v2f {
        static_cast<float>(sprite_dim[0] * cell_pos[0] * transform->scale[0]),
        static_cast<float>(sprite_dim[1] * cell_pos[1] * transform->scale[1])
    };
    sprite->ssheet = tile_meta.ssheet;
    sprite->render_order = 65536 - 17;
    if (sprite->ssheet.find("water") != std::string::npos || 
    sprite->ssheet.find("lava") != std::string::npos) {
        auto [collider] = ecs::ecs.component().assign<COLLIDER>(entity);
        collider->dim = v2u {static_cast<u16>(16 * transform->scale[0]),
            static_cast<u16>(16 * transform->scale[1])};
        collider->passable = false;
    }
    return entity;
}

const ecs::Entity* createChest(WorldGenerationSettings& settings, 
    const v2u& tile_sprite_dim, const v2f& tile_scale, 
	v2i& cell_pos, LootTable& loot_table)
{
    const ecs::Entity* entity = ecs::ecs.entity().checkout();
	if (entity == nullptr) {
		DERROR("Entity was Not Generated.\n");
		return nullptr;
	}
    auto [transform,sprite,collider,chest] = ecs::ecs.component()
        .assign<TRANSFORM,SPRITE,COLLIDER,CHEST>(entity);
    transform->pos = settings.origin + v2f {
        static_cast<float>(tile_sprite_dim[0] * tile_scale[0] * cell_pos[0]),
        static_cast<float>(tile_sprite_dim[1] * tile_scale[1] * cell_pos[1])
    };
    sprite->ssheet = "chest";
    sprite->render_order = 65536 - 16;
    collider->dim = {static_cast<u16>(32),static_cast<u16>(32)};
    collider->callback = [sprite,chest,&loot_table]
		([[maybe_unused]] const ecs::Entity* first, 
			[[maybe_unused]] const ecs::Entity* second) {
		if (first != player && second != player) {
			return;
		}
        if (!chest->opened) {
            sprite->ssheet = "chest-open";
            chest->opened = true;
            Loot loot = loot_table.random();
            auto [health] = ecs::ecs.component().fetch<HEALTH>(player);
            auto [shealth,fuel,oxygen] = ecs::ecs.component()
				.fetch<HEALTH,ecs::Fuel,ecs::Oxygen>(spaceship);
            switch (loot.type) {
                case PLAYER_HEALTH:
                    health->health += loot.amount;
                    break;
                case SHIP_HEALTH:
                    shealth->health += loot.amount;
                    break;
                case LOOT_FUEL:
                    fuel->fuel += loot.amount;
                    break;
                case LOOT_OXYGEN:
                    oxygen->oxygen += loot.amount;
                    break;
                case GOLD:
                    gold += loot.amount;
                    break;
            }
            std::cout << "You got: " << loot.type << ' ' << 
                loot.amount << std::endl;
        }
    };

    return entity;
}

void populateWithChests(World& world, const v2u& grid_size, 
	std::mt19937& generator, LootTable& loot_table)
{
    using WorldCell = std::vector<const ecs::Entity*>;
    auto chest = ssheets.find("chest");
    if (chest == ssheets.end())
        return;
    std::shared_ptr<SpriteSheet> chest_ssheet = chest->second;
    std::vector<std::pair<WorldCell*, Vec2<i32>>> local_cells;
    for (int i = 0; i < grid_size[1]; ++i) {
        for (int j = 0; j < grid_size[0]; ++j) {
            WorldCell& cell = world.cells[i][j];
            auto [collider,sprite] = ecs::ecs.component()
				.fetch<COLLIDER,SPRITE>(cell[0]);
            if (cell.size() != 1 || 
				(collider != nullptr && !collider->passable) || 
				(sprite->ssheet.find("water") != std::string::npos || 
				sprite->ssheet.find("lava") != std::string::npos))
                continue;
            local_cells.push_back({&cell, {i,j}});
        }
    }
    std::shuffle(local_cells.begin(),local_cells.end(),generator);
    for (int i = 0; i < world.getSettings().chest_count; ++i) {
        auto pair = local_cells[i];
        WorldCell* cell = pair.first;
		const ecs::Entity* tile = cell->at(0);
		auto [sprite,transform] = ecs::ecs.component()
			.fetch<SPRITE,TRANSFORM>(tile);
		auto ssheet_it = ssheets.find(sprite->ssheet);
		if (ssheet_it == ssheets.end())
			continue;
        cell->push_back(createChest(world.getSettings(),
            ssheet_it->second->sprite_dim,
			transform->scale, pair.second, loot_table));
    }
}

WorldGenerationSettings& World::getSettings()
{
    return settings_;
}

World::World(WorldGenerationSettings& settings, LootTable& loot_table) 
    : 
	settings_{settings},
	generator_{std::random_device{}()},
	loot_table_{loot_table}
{
    Biome biome = selectBiome(
        settings.temperature,
        settings.humidity, 
        settings.biome_seed);
    std::unordered_map<std::string,wfc::TileMeta> tile_map = biome.tiles();
    wfc::Grid grid {{settings.radius,settings.radius}};
    wfc::WaveFunction wf {grid,tile_map};
    const auto grid_size = grid.size;
    cells.resize(grid_size[1]);
    for (i32 i {0}; i < grid_size[1]; i++) {
        cells[i].resize(grid_size[0]);
        for (i32 j {0}; j < grid_size[0]; j++) {
            auto cell = grid.get({i,j});
            if (cell == nullptr || cell->state.empty())
                continue;
            auto tile_pair = tile_map.find(cell->state);
            if (tile_pair == tile_map.end())
                continue;
            wfc::TileMeta tile_meta = tile_pair->second;
            auto ssheet = ssheets.find(tile_meta.ssheet);
            if (ssheet == ssheets.end())
                continue;
            v2u sprite_dim = ssheet->second->sprite_dim;
            const ecs::Entity* tile = createWorldTile(settings,sprite_dim,
                cell->pos,tile_meta);
            auto [transform,sprite] = ecs::ecs.component()
                .fetch<TRANSFORM,SPRITE>(tile);
            cells[i][j].push_back(tile);
            std::uniform_real_distribution<double> dis(0.0,1.0);
            if (dis(generator) > 0.8) {
                std::vector<std::string> decors = Biome::decor(biome.type);
                if (decors.size() == 0) {
                    continue;
                }
                std::random_device rd;
                std::mt19937 generator { rd() };
                std::uniform_int_distribution<> dist (0, decors.size() - 1);
                int decor_index = dist(generator_);
                auto decor_ssheet = ssheets.find(decors[decor_index]);
                if (decor_ssheet == ssheets.end() || 
                    sprite->ssheet.find("water") != std::string::npos || 
                    sprite->ssheet.find("lava") != std::string::npos)
                    continue;
                auto decor_sd = decor_ssheet->second->sprite_dim;
                const ecs::Entity* decor = ecs::ecs.entity().checkout();
                auto [dtransform,dsprite] = ecs::ecs.component()
                    .assign<TRANSFORM,SPRITE>(decor);
                dsprite->ssheet = decors[decor_index];
                dsprite->render_order = 65536 - 15;
                dtransform->pos = transform->pos;
                dtransform->pos[1] += (decor_sd[1] / 2);
                cells[i][j].push_back(decor);
            }
        }
    }
    populateWithChests(*this, grid_size, generator_, loot_table);
}

World::~World()
{
	for (auto& row : cells) {
		for (auto& column : row) {
			for (const auto& tile : column) {
				ecs::ecs.entity().ret(tile);
			}
		}        
	}
}

////////////////////////////////////////////////////////////////////////////////

namespace wfc
{

////////////////////////////////////////////////////////////////////////////////

	TileMeta::TileMeta(
			float weight, 
			std::string ssheet,
			std::array<std::unordered_set<std::string>,4>& rules, 
			std::unordered_map<std::string,float>& coefficients) 
		: 
			weight{weight}, 
		ssheet{ssheet},
		rules{rules}, 
		coefficients{coefficients} 
	{        
	}

////////////////////////////////////////////////////////////////////////////////

	TileBuilder::TileBuilder(
			float weight, 
			const std::string& ssheet)
		:
			_weight{weight},
		_ssheet{ssheet}
	{
	}

	TileBuilder& TileBuilder::rule(int dir, const std::string& tile)
	{
		_rules[dir].insert(tile);
		return *this;
	}

	TileBuilder& TileBuilder::omni(const std::string& tile)
	{
		for (int i = 0; i < 4; i++)
			_rules[i].insert(tile);
		return *this;
	}

	TileBuilder& TileBuilder::omni(std::initializer_list<std::string> tiles)
	{
		for (const auto& tile : tiles) {
			omni(tile);
		}
		return *this;
	}

	TileBuilder& TileBuilder::coefficient(
			const std::string& tile, 
			float weight)
	{
		_coefficients[tile] = weight;
		return *this;
	}

	TileMeta TileBuilder::build()
	{
		return {_weight,_ssheet,_rules,_coefficients};
	}

	////////////////////////////////////////////////////////////////////////////////

	Cell::Cell(
			const v2i& pos, 
			const std::unordered_set<std::string>& states) 
		: 
			pos{pos},
		states{states} 
	{ 
	}

	u16 Cell::entropy() const
	{
		return static_cast<u16>(states.size());
	}

	bool Cell::collapsed() const
	{
		return !state.empty();
	}

	////////////////////////////////////////////////////////////////////////////////

	Grid::Grid(const v2u& size) : size{size} {}

	Cell* Grid::get(v2i pos)
	{
		if (pos[0] >= 0 && 
				pos[0] < size[0] && 
				pos[1] >= 0 && 
				pos[1] < size[1]) {
			return &cells[pos[0] * size[0] + pos[1]];
		}
		return nullptr;
	}

	bool Grid::collapsed()
	{
		for (auto& cell : cells) {
			if(!cell.collapsed()) {
				return false;
			}
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////

	TilePriorityQueue::TilePriorityQueue(Grid& grid)
	{
		for (auto& cell : grid.cells) {
			_queue.push_back(&cell);
		}
		std::shuffle(_queue.begin(),_queue.end(),generator);
	}

	void TilePriorityQueue::_swap(u16 i1, u16 i2)
	{
		auto temp = _queue[i1];
		_queue[i1] = _queue[i2];
		_queue[i2] = temp;
	}

	void TilePriorityQueue::_bubbleUp(u16 idx)
	{
		while (idx > 0) {
			u16 parent {static_cast<u16>((idx - 1) / 2)};
			if (_queue[parent]->entropy() > _queue[idx]->entropy()) {
				_swap(parent,idx);
			} else {
				break;
			}
		}
	}

	void TilePriorityQueue::_bubbleDown(u16 idx)
	{
		auto size = _queue.size();
		while (idx <= size) {
			u16 left = 2 * idx + 1;
			u16 right = 2 * idx + 2;
			u16 smallest = idx;
			if (left < size && 
					_queue[left]->entropy() < _queue[smallest]->entropy()) {
				smallest = left;
			}
			if (right < size && 
					_queue[right]->entropy() < _queue[smallest]->entropy()) {
				smallest = right;
			}
			if (smallest == idx) {
				break;
			}
			_swap(smallest,idx);
			idx = smallest;
		}
	}

	void TilePriorityQueue::insert(Cell* cell)
	{
		_queue.push_back(cell);
		_bubbleUp(_queue.size() - 1);
	}

	bool TilePriorityQueue::empty()
	{
		return _queue.empty();
	}

	Cell* TilePriorityQueue::pop()
	{
		auto cell = _queue.front();
		_queue[0] = _queue.back();
		_queue.pop_back();
		_bubbleDown(0);
		return cell;
	}

	////////////////////////////////////////////////////////////////////////////////

	WaveFunction::WaveFunction(
			Grid& grid, 
			std::unordered_map<std::string,TileMeta>& tiles) 
		: 
			_grid{grid}, 
		_tiles{tiles}, 
		_queue{grid} {
			std::unordered_set<std::string> states;
			for (auto& pair : _tiles)
				states.insert(pair.first);
			grid.cells.reserve(grid.size[1] * grid.size[0]);
			for (i32 i = 0; i < grid.size[1]; i++) {
				for (i32 j = 0; j < grid.size[0]; j++) {
					grid.cells.emplace_back(v2i{i,j}, states);
				}
			}
			_queue = {grid};
			this->run();
		}

	v2i _shift_vector(u16 dir, const v2i& vec) {
		return v2i {vec[0] + dirs[dir][0],vec[1] + dirs[dir][1]};
	} 

	float WaveFunction::_calculateTileWeight(
			const v2i& pos, 
			const TileMeta& meta)
	{
		auto base = meta.weight;
		auto weight = base;
		for (int i = 0; i < 4; i++) {
			auto shift = pos + dirs[i];
			auto cell = _grid.get(shift);
			if (shift[0] >= 0 && 
					shift[0] < _grid.size[0] && 
					shift[1] >= 0 && 
					shift[1] < _grid.size[1] && 
					cell->collapsed()) {
				if (meta.coefficients.find(cell->state) != 
						meta.coefficients.end()) {
					auto coefficient = meta.coefficients.at(cell->state);
					weight += base * coefficient;
				}
			}
		}
		return weight;
	} 

	void WaveFunction::_collapse(Cell* c)
	{
		float cumulative_weight {0};
		std::vector<std::pair<std::string,float>> state_weights;
		for (auto& state : c->states) {
			auto it = _tiles.find(state);
			if (it != _tiles.end()) {
				auto weight = _calculateTileWeight(c->pos, it->second);
				cumulative_weight += weight;
				state_weights.push_back({state,cumulative_weight});
			}
		}

		std::uniform_real_distribution<float> dist {0.0f,cumulative_weight};

		float r = dist(generator);

		std::string selected;
		for (auto& pair : state_weights) {
			if (r <= pair.second) {
				selected = pair.first;
				break;
			}
		}
		_grid.get(c->pos)->state = selected;
	}

	void WaveFunction::_propagate(Cell* c)
	{
		for (int i {0}; i < 4; i++) {
			auto c1 = _grid.get(_shift_vector(i,c->pos));
			if (c1 == nullptr || c1->collapsed())
				continue;
			for (int j {0}; j < 4; j++) {
				auto c2 = _grid.get(_shift_vector(j,c1->pos));
				if (c2 == nullptr || !c2->collapsed())
					continue;
				auto t2 = _tiles.find(c2->state);
				if (t2 == _tiles.end())
					continue;
				auto t2_rules = t2->second.rules[opposite[j]];
				for (auto it = c1->states.begin(); it != c1->states.end();) {
					auto t1 = _tiles.find(*it);
					auto t1_rules = t1->second.rules[j];

					auto s1 = std::count(
							t2_rules.begin(),
							t2_rules.end(),
							*it);

					auto s2 = std::count(
							t1_rules.begin(),
							t1_rules.end(),
							c2->state);

					bool compatible = s1 && s2;

					if (c1->entropy() == 1)
						break;

					if (compatible) {
						++it;
					} else {
						it = c1->states.erase(it);
					}
				}
			}
		}
	}

	void WaveFunction::run()
	{
		while (!_queue.empty()) {
			if (_grid.collapsed())
				break;
			auto cell = _queue.pop();
			if (cell == nullptr)
				break;
			_collapse(cell);
			_propagate(cell);
		}
	}

	////////////////////////////////////////////////////////////////////////////////

}

namespace ecs
{   
	ECS ecs;

	Entity::Entity(u32 i, cmask m) : id(i), mask(m) {}

	////////////////////////////////////////////////////////////////////////////////

	ComponentPool::ComponentPool(u32 size) : _size {size}
	{
		_ptr_data = std::make_unique<char[]>(size * MAX_ENTITIES);
	}

	u32 ComponentPool::size() const
	{
		return _size;
	}

	void *ComponentPool::get(u16 idx)
	{
		return _ptr_data.get() + idx * _size;
	}

	EntityManager::EntityManager(
			u32 max_entities) :
		_max_entities{max_entities}
	{
		for (u32 i{0}; i < _max_entities; i++) {
			entities.push_back({i, cmask()});
			_free.push_back(i);
		}
	}

	Entity *EntityManager::checkout()
	{
		if (_free.empty())
			return nullptr;
		u32 idx{_free.front()};
		_free.pop_front();
		return &entities[idx];
	}

	void EntityManager::ret(const Entity* e_ptr)
	{
		e_ptr->mask.reset();
		_free.push_back(e_ptr->id);
		e_ptr = nullptr;
	}

	u32 EntityManager::maxEntities() const
	{
		return _max_entities;
	}

	ECS::ECS() : _entity_manager{MAX_ENTITIES} {}

	EntityManager &ECS::entity()
	{
		return _entity_manager;
	}

	bool EntityManager::isFree(const Entity* e_ptr)
	{
		return std::count(_free.begin(),_free.end(),e_ptr->id);
	}

	ComponentManager &ECS::component()
	{
		return _component_manager;
	}

	std::unordered_map<u32,Transform*> transforms;
	std::unordered_map<u32,Sprite*> sprites;
	std::unordered_map<u32,Physics*> physics;
	std::unordered_map<u32,Collider*> colliders;


	PhysicsSystem::PhysicsSystem(ECS& ecs, float sample_delta) 
		:
			System<Transform,Physics>(ecs,sample_delta)
	{
	}


	void PhysicsSystem::update(float dt)
	{
		for (auto& entity : _entities) {
			auto [tc, pc] = ecs.component().fetch<Transform, Physics>(entity);
			if (!pc || !tc) continue;

			if (pc->enabled) {
				// Only update the velocity and position if we have actual movement
				if (pc->vel[0] != 0.0f || pc->vel[1] != 0.0f) {
					pc->vel += pc->acc * dt;
					tc->pos += pc->vel * dt;

					// If this is the player, update the sprite based on movement direction
					if (entity == player) {
						auto [sprite] = ecs.component().fetch<SPRITE>(entity);
						if (sprite) {
							// Set the appropriate directional sprite based on movement
							if (fabs(pc->vel[0]) > fabs(pc->vel[1])) {
								// Horizontal movement dominates
								sprite->ssheet = "player-right";
								sprite->invert_y = (pc->vel[0] < 0.0f); // Invert if moving left
							} else {
								// Vertical movement dominates
								if (pc->vel[1] > 0.0f) {
									sprite->ssheet = "player-back";
									sprite->invert_y = false;
								} else {
									sprite->ssheet = "player-front";
									sprite->invert_y = false;
								}
							}
						}
					}
				}
			}
		}
	}
  
	RenderSystem::RenderSystem(ECS& ecs, float sample_delta) :
		System<Transform,Sprite>(ecs,sample_delta)
	{
	}

	void RenderSystem::sample()
	{
		_entities = _ecs.query<SPRITE,TRANSFORM>();
		std::sort(_entities.begin(), _entities.end(), 
				[this](const Entity* a, const Entity* b) {
				auto [as] = _ecs.component().fetch<SPRITE>(a);
				auto [bs] = _ecs.component().fetch<SPRITE>(b);
				if (as == nullptr && bs == nullptr)
				return false;
				if (as == nullptr)
				return false;
				if (bs == nullptr)
				return true;
				return as->render_order < bs->render_order;
				});
	}

	struct CompareEntity
	{
		bool operator()(const Entity* e1, const Entity* e2)
		{
			auto [s1] = ecs::ecs.component().fetch<SPRITE>(e1);
			auto [s2] = ecs::ecs.component().fetch<SPRITE>(e2);
			return s1->render_order > s2->render_order;
		}
	};

	// void renderHelper(std::vector<const Entity*>& entities, 
	// 		std::priority_queue<const Entity*, std::vector<const Entity*>, 
	// 		CompareEntity>& visible, 
	// 		std::mutex& mutex, u32 start, u32 end)
	// {
	// 	std::vector<const Entity*> local_visible;
	// 	local_visible.reserve(end - start);
	// 	for (i32 i {static_cast<i32>(start)}; i < static_cast<i32>(end); ++i) {
	// 		auto e = entities[i];
	// 		auto [transform] = ecs::ecs.component().fetch<Transform>(e);
	// 		if ((c->visible(transform->pos))) {
	// 			local_visible.emplace_back(e);
	// 		}
	// 	}

	// 	std::lock_guard<std::mutex> lock(mutex);
	// 	for (auto& entity : local_visible) {
	// 		visible.push(entity);
	// 	}
	// }


	void RenderSystem::update([[maybe_unused]]float dt)
	{
		for (auto& entity : _entities) {
			auto [transform, sprite] = _ecs.component()
				.fetch<TRANSFORM, SPRITE>(entity);

			if (!transform || !sprite) continue;
			if (!c->visible(transform->pos)) continue;
			if (sprite->ssheet.empty()) continue;

			auto ssheet = ssheets[sprite->ssheet];
			if (ssheet == nullptr) continue;
			if (ssheet->tex == nullptr) continue;

			// Render sprite
			ssheet->render(sprite->frame, transform->pos,
				transform->scale, sprite->invert_y);

			// Handle animation
			if (ssheet->animated) {
				sprite->frame++;
				auto f = ssheet->frame_dim;
				if (sprite->frame >= f[0] * f[1]) {
					sprite->frame = 0;
				}
			}

			// Handle name component
			if (_ecs.component().has<NAME>(entity)) {
				auto [name] = _ecs.component().fetch<NAME>(entity);
				Rect r;
				switch(name->alignment) {
					case 0:
						r.left = transform->pos[0];
						r.bot = transform->pos[1];
						break;
					case 1:
					case 2:
						break;
				}
				r.left += name->offset[0];
				r.bot += name->offset[1];
				ggprint8b(&r, 0, name->cref, name->name.c_str());
			}
		}
	}
}

ThreadPool::ThreadPool(u32 nthreads) : stop_{false}, size_{nthreads}
{
	workers_.reserve(nthreads);
	for (u32 i {0}; i < nthreads; ++i) {
		workers_.emplace_back(&ThreadPool::workerThread, this);
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::lock_guard<std::mutex> lock(queue_mutex_);
		stop_ = true;
	}

	task_available_.notify_all();

	for (auto& worker : workers_) {
		worker.join();
	}
}

void ThreadPool::enqueue(std::function<void()> task)
{
	{
		std::lock_guard<std::mutex> lock(queue_mutex_);
		task_queue_.push(task);
	}

	task_available_.notify_one();
}

void ThreadPool::workerThread()
{
    while (1) {
        std::function<void()> current_task = nullptr;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            task_available_.wait(lock, 
                [this] { return stop_ || !task_queue_.empty(); });
            
            if (stop_ && task_queue_.empty()) {
                return;
            }

			current_task = std::move(task_queue_.front());
			task_queue_.pop();
		}

		if (current_task) {
			current_task();
		}
	}
}

u32 ThreadPool::size() const
{
	return size_;
}

//https://singlelunch.com/2018/09/26/programming-trick-cantor-pairing-perfect-hashing-of-two-integers/
u64 cantor_hash(u32 a, u32 b) 
{
	return ((a + b) * (a + b + 1) >> 1) + b; 
}

bool collided(TRANSFORM* ta, TRANSFORM* tb, COLLIDER* ca, COLLIDER* cb)
{
	const v2f pa = ta->pos + ca->offset;
	const v2f pb = tb->pos + cb->offset;

	return (pa[0] - ca->dim[0] * 0.5f < pb[0] + cb->dim[0] * 0.5f) &&
		(pa[0] + ca->dim[0] * 0.5f > pb[0] - cb->dim[0] * 0.5f) &&
		(pa[1] + ca->dim[1] * 0.5f > pb[1] - cb->dim[1] * 0.5f) &&
		(pa[1] - ca->dim[1] * 0.5f < pb[1] + cb->dim[1] * 0.5f);
}

struct PairHash {
	template <class T1, class T2>
		std::size_t operator ()(const std::pair<T1, T2>& p) const {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return h1 ^ (h2 << 1);
		}
};

std::vector<Collision> findCollisions(
		std::vector<const ecs::Entity*>& visible_entities) 
{
	std::vector<Collision> collisions;
	std::unordered_set<std::pair<u32,u32>,PairHash> seen;
	for (const auto& a : visible_entities) {
		for (const auto& b : visible_entities) {
			if (a->id == b->id)
				continue;
			u32 min = std::min({a->id,b->id});
			u32 max = std::max({a->id,b->id});
			if (seen.count({min,max})) {
				continue;
			}
			seen.insert({min,max});        
			auto [ta, ca] = ecs::ecs.component().fetch<TRANSFORM,COLLIDER>(a);
			auto [tb, cb] = ecs::ecs.component().fetch<TRANSFORM,COLLIDER>(b);
			if (collided(ta,tb,ca,cb)) {
				collisions.push_back({a, b});
			}

		}
	}
	return collisions;
}

std::unordered_map<u32, v2f> previous_position;
void handleCollisions(std::vector<Collision>& collisions)
{

	for (auto& collision : collisions) {
		auto [ta, ca] = ecs::ecs.component()
			.fetch<TRANSFORM, COLLIDER>(collision.a);
		auto [tb, cb] = ecs::ecs.component()
			.fetch<TRANSFORM, COLLIDER>(collision.b);
		if (!ca->passable && previous_position.count(collision.b->id))
			tb->pos = previous_position[collision.b->id];
		if (!cb->passable && previous_position.count(collision.a->id))
			ta->pos = previous_position[collision.a->id];
		try {
			if (ca->callback) {
				ca->callback(collision.b, collision.a);
			}
			if (cb->callback) {
				cb->callback(collision.a, collision.b);
			}
		} catch (const std::bad_function_call& e) {
			std::cout << e.what() << '\n';
		}
	}
}


void collisions(const Camera& camera, ThreadPool& pool)
{
	while (!done) {
		std::vector<const ecs::Entity*> entities = 
			ecs::ecs.query<COLLIDER,TRANSFORM>();
		std::vector<const ecs::Entity*> visible_entities = 
			camera.findVisible(entities,pool);
		std::vector<Collision> collisions = 
			findCollisions(visible_entities);
		handleCollisions(collisions);
		for (const auto& entity : entities) {
			auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(entity);
			previous_position[entity->id] = transform->pos;
		}
		usleep(1000);
	}
}

Collision::Collision(const ecs::Entity* a, const ecs::Entity* b) : a{a}, b{b} {}

LootTable::LootTable() : generator_{std::random_device{}()} {}

LootTable& LootTable::addLoot(std::initializer_list<Loot> loot)
{
    loot_.reserve(loot_.size() + loot.size());
    for (const auto& l : loot) {
        loot_.emplace_back(l);
    }
    return *this;
}

Loot LootTable::random()
{
    float tweight = 0.0f;
    for (auto& loot : loot_) {
        tweight += loot.weight;
    }
    std::uniform_real_distribution<> dist {0.0f,tweight};
    float rvalue = dist(generator_);
    float cweight = 0.0f;
    for (auto& loot : loot_) {
        cweight += loot.weight;
        if (cweight >= rvalue) {
            return loot;
        }
    }
    return loot_[0];
}


std::pair<int, v2f> bfs(std::vector<std::vector<std::vector<const ecs::Entity*>>>& cells, 
	std::vector<std::vector<bool>>& visited, int x, int y)
{
	int m = cells.size();
	int n = cells[0].size();
	int area = 0;
	v2f pos_sum = {0.f, 0.f};

	std::queue<std::pair<int, int>> q;
	q.push({x, y});
	visited[x][y] = true;

	while (!q.empty()) {
		auto [r, c] = q.front();
		q.pop();
		area++;

		const ecs::Entity* entity = cells[r][c][0];
		auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(entity);
		if (transform != nullptr)
		pos_sum += transform->pos;

		for (const auto& dir : dirs) {
			int nr = r + dir[0];
			int nc = c + dir[1];

			if (nr >= 0 && nr < m && nc >= 0 && nc < n &&
			!visited[nr][nc] && !cells[nr][nc].empty()) {

				const ecs::Entity* neighbor = cells[nr][nc][0];
				auto [collider] = ecs::ecs.component()
					.fetch<COLLIDER>(neighbor);

				if (collider != nullptr && !collider->passable)
				continue;

				visited[nr][nc] = true;
				q.push({nr, nc});
			}
		}
	}

	return {area, pos_sum};
}


v2f World::getCenterOfLargestIsland() {
    v2f best_position = {0.0f, 0.0f};
	u16 m = 0;
	u16 n = 0;
	if (cells.empty()){
		return best_position;
	}
	if (cells[0] != nullptr) {
    	return best_position;
	}
	m = cells.size();
	n = cells[0].size();

    std::vector<std::vector<bool>> visited(m, std::vector<bool>(n, false));

    int max_area = 0;

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (!visited[i][j] && !cells[i][j].empty()) {
                const ecs::Entity* entity = cells[i][j][0];
                auto [collider] = ecs::ecs.component().fetch<COLLIDER>(entity);

                if (collider != nullptr && !collider->passable)
                    continue;

                auto [area, pos_sum] = bfs(cells, visited, i, j);
                if (area > max_area) {
                    max_area = area;
                    best_position = {pos_sum[0] / static_cast<float>(area),
						pos_sum[1] / static_cast<float>(area)};
                }
            }
        }
    }

    return best_position;
}

const ecs::Entity* createPlayer(World& world)
{
	const ecs::Entity* player = ecs::ecs.entity().checkout();
	auto [transform,sprite,name,collider,health,p] = ecs::ecs.component()
		.assign<TRANSFORM,SPRITE,NAME,COLLIDER, HEALTH,PHYSICS>(player);
	transform->pos = world.getCenterOfLargestIsland();
	name->name = "Juancarlos Sandoval";
	name->offset = {0,-25};
	sprite->ssheet = "player-idle";
	sprite->render_order = 65536 - 16;
	collider->offset = {0.0f,-8.0f};
	collider->dim = v2u {5,4};
	health->health = 100.0f;
	health->max = 100.0f;
	return player;
}
