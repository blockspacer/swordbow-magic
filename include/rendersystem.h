#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "isystem.h"
#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <unordered_set>
#include <queue>

using namespace std;

class RenderComponent;
class MoveComponent;
class SizeComponent;
class FlagComponent;
class SpatialIndexer;

struct TextureData {
	SDL_Texture* texture;
	unsigned int width;
	unsigned int height;
};

struct SortHelper {
	unsigned long long int* id;
	RenderComponent* renderComponent;

	inline bool operator< (const SortHelper& rhs) const;
    inline bool operator> (const SortHelper& rhs) const;
    inline bool operator<=(const SortHelper& rhs) const;
    inline bool operator>=(const SortHelper& rhs) const;
};

class RenderSystem : public ISystem {
 private:
	unordered_set<unsigned long long int*> ids;
	queue<unsigned long long int*> activeIds;
	queue<SortHelper> previousRedraws;


	static constexpr ushort SCREEN_WIDTH = 640;
	static constexpr ushort SCREEN_HEIGHT = 480;
	bool renderingRequired = true;

	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* screenSurface = nullptr;
	SDL_Texture* targetTexture = nullptr;
	unordered_map<string, TextureData> textureDatas;

	unsigned long long int* cameraTarget;

 public:
 	RenderSystem();
 	~RenderSystem();
	void add(unsigned long long int* id);
	void remove(unsigned long long int* id);
	void update();
	void update2();
	unsigned int count() const;
	void render(unsigned long long int* id) const;
	const string getIdentifier() const;
	void calculateZIndex(unsigned long long int* id);
	void makeIdActive(unsigned long long int* id);
	void setCameraTarget(unsigned long long int* id);
};

#endif //RENDERSYSTEM_H
