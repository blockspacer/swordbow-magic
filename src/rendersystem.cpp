#include "rendersystem.h"
#include "componentmanager.h"
#include "rendercomponent.h"
#include "movecomponent.h"
#include "sizecomponent.h"
#include <iostream>
#include "spatialindexer.h"
#include "heap.h"
#include "dynamicarray.h"
#include "heapsort.h"
#include <queue>
#include "namecomponent.h"
#include "systemmanager.h"
#include "hashgridsystem.h"

using namespace std;

bool RenderData::operator< (const RenderData& rhs) const {
    return
        renderComponent->zindex_base == rhs.renderComponent->zindex_base ?
        renderComponent->zindex < rhs.renderComponent->zindex :
        renderComponent->zindex_base < rhs.renderComponent->zindex_base;
}

bool RenderData::operator> (const RenderData& rhs) const { return rhs < *this; }
bool RenderData::operator<=(const RenderData& rhs) const { return !(*this > rhs); }
bool RenderData::operator>=(const RenderData& rhs) const { return !(*this < rhs); }


RenderSystem::RenderSystem() {
	//Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "ERROR: SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
    } else {
        window = SDL_CreateWindow(
            "SDL Tutorial",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);

        if(!window) {
            cout << "ERROR: Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        } else {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
            if(!renderer) {
                cout << "ERROR: Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            } else {

				//imgFlags is used to initialize PNG-loading
				//so that IMG_Load can be used instead of SDL_LoadBMP
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)) {
					cout << "ERROR: Couldnt initialze PNG usage!" << endl;
				} else {


		            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

			        targetTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 8192, 8192);
					if(!targetTexture) {
						cout << "ERROR: Couldn't create targetTexture!" << endl;
						cout << "Screen will probably go black from now on" << endl;
						cout << "SDL_GetError(): " << SDL_GetError() << endl;
					}
					SDL_SetRenderTarget(renderer, targetTexture);
					SDL_RenderClear(renderer);
				}
            }
        }
    }

    //We want to access surfaces by their image path later on, so store paths right away
    for(auto path :{
            "./resources/images/HelloWorld.bmp",
            "./resources/images/player.bmp",
            "./resources/images/grass.bmp",
			"./resources/images/SmallTree.png",
            }
        )
    {
        SDL_Surface* rawImage = IMG_Load(path);
        if(!rawImage) {
            cout << "ERROR: Couldn't load image on: " << path << endl;
        } else {
            //Make purple parts of images transparent by using color keying
            SDL_SetColorKey(rawImage, SDL_TRUE, SDL_MapRGB( rawImage->format, 0xFF, 0x00, 0xFF ));
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rawImage);
            if(!texture) {
                cout << "ERROR: Couldn't convert surface on addr " << rawImage << " to texture! SDL_Error: " << SDL_GetError() << endl;
            } else {
                SDL_FreeSurface(rawImage);
                int w, h;
                SDL_QueryTexture(texture, NULL, NULL, &w, &h);
                textureDatas.insert(
                    make_pair(
                        path,
                        TextureData{
                            texture,
                            (unsigned int)w,
                            (unsigned int)h
                        }
                    )
                );
            }
        }
    }
}

RenderSystem::~RenderSystem() {
    //Free all loaded images
    for(auto textureData : textureDatas) {
        SDL_DestroyTexture(get<1>(textureData).texture);
    }

    SDL_DestroyTexture(targetTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void RenderSystem::add(unsigned long long int* id) {
	ids.insert(id);
	activeIds.push(id);
}

void RenderSystem::remove(unsigned long long int* id) {
	ids.erase(id);
}

void RenderSystem::renderArea(heap<RenderData>& pq, SpatialIndexer::Rect area) {
	//Make sure all overlapping ids have their part in the intersection area
	//are redrawn
	auto spatialIndexer = dynamic_cast<SpatialIndexer*>(systemManager->getSystem("TextureHashGridSystem"));
	unordered_set<ID> overlappings;
	SpatialIndexer::Rect intersectionArea;
	SpatialIndexer::Rect idbb;

	spatialIndexer->query(overlappings, area);
	for(auto id : overlappings) {
		auto& mc = componentManager->moveComponents.at(id);
		auto& rc = componentManager->renderComponents.at(id);

		//Get the intersection between an entity within drawarea and the drawarea
		spatialIndexer->getBoundingBox(idbb, id);
		spatialIndexer->getIntersectionArea(intersectionArea, area, idbb);

		//Only draw that portion of the texture that intersects
		calculateZIndex(id);
		pq.insert({id, &rc, rc.textureData, {
				intersectionArea.x - mc.xpos - rc.xoffset,
				intersectionArea.y - mc.ypos - rc.yoffset,
				intersectionArea.w,
				intersectionArea.h }, {
				intersectionArea.x,
				intersectionArea.y,
				intersectionArea.w,
				intersectionArea.h}

				}
			);
	}
}

void RenderSystem::update() {
	heap<RenderData> pq; //TODO: Use reference instead (no need for copying!)
	auto spatialIndexer = dynamic_cast<SpatialIndexer*>(systemManager->getSystem("TextureHashGridSystem"));

	//Redraw everything within drawareas from previous frame
	while(!previousDrawAreas.empty()) {
		auto prevDrawArea = previousDrawAreas.front(); previousDrawAreas.pop();
		renderArea(pq, prevDrawArea);
	}

	//For all activeIds in rendersystem (henceforth activeIds will be referred
	//to as just ids)
	while(!activeIds.empty()) {

		//Draw everything within the activeIds texturearea
		SpatialIndexer::Rect drawArea;
		spatialIndexer->getBoundingBox(drawArea, activeIds.front());
		activeIds.pop();
		previousDrawAreas.push(drawArea);
		renderArea(pq, drawArea);
	}

	SDL_SetTextureBlendMode(targetTexture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, targetTexture);

	while(!pq.isEmpty()) {
		render(pq.poll());
	}

	const auto& cameraXpos = componentManager->moveComponents.at(cameraTarget).xpos - SCREEN_WIDTH/2;
	const auto& cameraYpos = componentManager->moveComponents.at(cameraTarget).ypos - SCREEN_HEIGHT/2;
	const SDL_Rect camera = {
		cameraXpos < 0 ? 0 : (int)cameraXpos,
		cameraYpos < 0 ? 0 : (int)cameraYpos,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
	};

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, targetTexture, &camera, NULL);
	SDL_RenderPresent(renderer);
}

unsigned int RenderSystem::count() const {
	return ids.size();
}

void RenderSystem::render(const RenderData& rd) const {
    SDL_RenderCopy(renderer, rd.textureData.texture, &rd.cliprect, &rd.target);
}

const string RenderSystem::getIdentifier() const {
	return "RenderSystem";
}

void RenderSystem::calculateZIndex(unsigned long long int* id) {
	auto& rc = componentManager->renderComponents.at(id);
	auto& mc = componentManager->moveComponents.at(id);
	rc.zindex = mc.ypos + rc.textureData.height + rc.yoffset;
}

void RenderSystem::makeIdActive(unsigned long long int* id) {
	//Only make the id active if it is a member of rendersystem
	if(ids.find(id) != ids.end()) {
		activeIds.push(id);
	}
}

void RenderSystem::setCameraTarget(unsigned long long int* id) {
	cameraTarget = id;
}

void RenderSystem::setImage(unsigned long long int* id, string path) {
	auto& rc = componentManager->renderComponents.at(id);
	rc.imagePath = path;
	rc.textureData = textureDatas.at(path);
}
