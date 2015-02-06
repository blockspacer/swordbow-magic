#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "isystem.h"
#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <vector>

using namespace std;

class RenderComponent;
class MoveComponent;
class GridIndexer;

struct RenderData {
	RenderComponent* renderComponent;
	MoveComponent* moveComponent;
};

struct TextureData {
	SDL_Texture* texture;
	unsigned char width;
	unsigned char height;
};

class RenderSystem : public ISystem {
 private:
 	static constexpr unsigned char MAX_IDS = 255;
 	unsigned long long int* ids[MAX_IDS] {0};
	unordered_map<unsigned long long int*, RenderData> renderDatas;
	static constexpr ushort SCREEN_WIDTH = 640;
	static constexpr ushort SCREEN_HEIGHT = 480;
	bool renderingRequired = true;

	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* screenSurface = nullptr;
	SDL_Texture* targetTexture = nullptr;
	unordered_map<string, TextureData> textureDatas;
	GridIndexer* gridIndexer;
 public:
 	RenderSystem(GridIndexer* gridIndexer);
 	~RenderSystem();
	void add(unsigned long long int* id);
	void remove(unsigned long long int* id);
	void update();
	unsigned int count() const;
	void render(const RenderData& data) const;
	const string getIdentifier() const;
};

#endif //RENDERSYSTEM_H