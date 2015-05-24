#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "isystem.hpp"
#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <unordered_set>
#include <queue>
#include "spatialindexer.hpp"
#include "heap.hpp"
#include "text.hpp"

using namespace std;

class RenderComponent;
class MoveComponent;
class SizeComponent;
class FlagComponent;
class SpatialIndexer;

typedef unsigned int ID;

class CameraSystem;

struct TextureData {
	SDL_Texture* texture;
	unsigned int width;
	unsigned int height;
};

struct RenderData {
	ID id;
	RenderComponent* renderComponent;
	TextureData textureData;
	SDL_Rect cliprect;
	SDL_Rect target;

	inline bool operator< (const RenderData& rhs) const;
    inline bool operator> (const RenderData& rhs) const;
    inline bool operator<=(const RenderData& rhs) const;
    inline bool operator>=(const RenderData& rhs) const;
};

class RenderSystem : public ISystem {
 private:
	unordered_set<ID> ids;
	queue<ID> activeIds;
	queue<SpatialIndexer::Rect> previousDrawAreas;
	queue<Text> texts;
	CameraSystem* cameraSystem;


	static constexpr ushort SCREEN_WIDTH = 640;
	static constexpr ushort SCREEN_HEIGHT = 480;

	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Texture* worldTexture = nullptr;
	SDL_Texture* fontTexture = nullptr;
	TTF_Font* font;
	unordered_map<string, TextureData> textureDatas;

	void renderArea(heap<RenderData>& pq, SpatialIndexer::Rect area);
	void renderTexts();

 public:
 	RenderSystem();
 	~RenderSystem();
	void add(ID id);
	void remove(ID id);
	void update();
	unsigned int count() const;
	void render(const RenderData& rd) const;
	const string getIdentifier() const;
	void calculateZIndex(ID id);
	void activateId(ID id);
	void setCameraSystem(CameraSystem* cameraSystem);
	void setImage(ID id, string path);
	void printText(const Text& text);

	//Forces a redraw within an area
	void inline constexpr renderArea(const SpatialIndexer::Rect& area) {
	    previousDrawAreas.push(area);
	}
	static constexpr ushort getScreenWidth() { return SCREEN_WIDTH; }
    static constexpr ushort getScreenHeight() { return SCREEN_HEIGHT; }

};

#endif //RENDERSYSTEM_H
