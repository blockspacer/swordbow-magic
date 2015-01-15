#include "rendersystem.h"
#include "componentmanager.h"
#include "rendercomponent.h"
#include "movecomponent.h"
#include <iostream>

using namespace std;

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
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if(!renderer) {
                cout << "ERROR: Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            }
        }
    }

    //We want to access surfaces by their image path later on, so store paths right away
    for(auto path :{
            "./resources/images/HelloWorld.bmp",
            "./resources/images/player.bmp",
            "./resources/images/grass.bmp",
            }
        ) 
    {
        SDL_Surface* rawImage = SDL_LoadBMP(path);
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
                            w,
                            h
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void RenderSystem::add(unsigned long long int* id) {
	renderDatas.insert(
        make_pair(
            id,
            RenderData{
                componentManager->renderComponents.at(id),
                componentManager->moveComponents.at(id)
            }
        )
    );
}

void RenderSystem::remove(unsigned long long int* id) {
	if(id == nullptr) {
		cout << "WARNING: No id on adress: " << id << endl;
		return;
	}
	if(renderDatas.erase(id) == 0) {
		cout << "WARNING: RenderSystem tried to erase unpresent ID " << *id << ", segfault inc!" << endl;
	}
}

void RenderSystem::update() {
	for(auto data : renderDatas) {
		render(get<1>(data));
	}
}

unsigned int RenderSystem::count() const {
	return renderDatas.size();
}

void RenderSystem::render(const RenderData& data) const {
    if(data.renderComponent->doRender) {
        TextureData textureData;
        try {
            textureData = textureDatas.at(data.renderComponent->imagePath);
        } catch (const std::out_of_range &oor) {
            cout << "out_of_range exception caught in SystemManager::getSystem(string identifier): " << oor.what() << endl;
            cout << "1. Maybe there as a typo in the RenderComponent->imagePath, " << data.renderComponent->imagePath << "?" << endl;
            cout << "2. Maybe " << data.renderComponent->imagePath << " wasn't loaded by RenderSystem?" << endl;
            cout << "Things will go wrong from now on!" << endl;
        }

        SDL_Rect rect{
            (int)(data.moveComponent->xpos + data.renderComponent->xoffset),
            (int)(data.moveComponent->ypos + data.renderComponent->yoffset),
            textureData.width,
            textureData.height
        };

        //SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, textureDatas.at(data.renderComponent->imagePath).texture, NULL, &rect);
        SDL_RenderPresent(renderer);
    }
    if(data.renderComponent->renderOnce) {
        data.renderComponent->doRender = false;
    }
}

const string RenderSystem::getIdentifier() const {
	return "RenderSystem";
}