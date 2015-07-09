#include "rendersystem.hpp"
#include "componentmanager.hpp"
#include "rendercomponent.hpp"
#include "movecomponent.hpp"
#include "sizecomponent.hpp"
#include <iostream>
#include "spatialindexer.hpp"
#include "heap.hpp"
#include "dynamicarray.hpp"
#include "heapsort.hpp"
#include <queue>
#include "namecomponent.hpp"
#include "systemmanager.hpp"
#include "hashgridsystem.hpp"
#include "deltatime.hpp"
#include "renderer.hpp"

using namespace std;

RenderSystem::RenderSystem(Renderer* renderer) :
    renderer(renderer) { }

void RenderSystem::add(ID id) {
	ids.insert(id);
	activeIds.push(id);
}

void RenderSystem::remove(ID id) {
	ids.erase(id);
}

void RenderSystem::update() {
	auto spatialIndexer = dynamic_cast<SpatialIndexer*>(systemManager->getSystem(System::HASHGRID_TEXTURE));

    //Will contain all rectangles where a redraw is required
    queue<Rect> drawQueue;

    //All drawareas saved from previous frame should be drawn this frame
	drawQueue.swap(previousDrawAreas);


    //Cap the amount of IDs getting rendered each frame
    //this is only a quick-fix to ensure no program-freeze
    //when initially rendering the whole world
    char allowedRendersThisFrame = 32;
	//For all activeIds in rendersystem...
	while(!activeIds.empty() && allowedRendersThisFrame-- > 0 ) {
		//Draw everything within the activeIds texturearea
        auto drawArea = spatialIndexer->getBoundingBox(activeIds.front());
		drawQueue.push(drawArea); activeIds.pop();

        //The texturearea should be saved for next frame, it must be redrawn then too
		renderArea(drawArea);
	}

    while(!drawQueue.empty()) {
        const auto& drawArea = drawQueue.front();

    	for(auto id : spatialIndexer->query(drawArea)) {
    		auto& mc = componentManager->moveComponents.at(id);
    		auto& rc = componentManager->renderComponents.at(id);

    		//Get the intersection between an entity within drawarea and the drawarea
    		const auto intersection = Rect::getIntersection(
                drawArea, spatialIndexer->getBoundingBox(id)
            );

            const SDL_Rect clipSource = {
                int(intersection.x) - int(mc.pos.x) - int(rc.xoffset),
                int(intersection.y) - int(mc.pos.y) - int(rc.yoffset),
                int(intersection.w),
                int(intersection.h)
            };

            const SDL_Rect clipDestination = {
                (int)intersection.x,
                (int)intersection.y,
                (int)intersection.w,
                (int)intersection.h
            };

            const TextureData& textureData = renderer->getTextureDatas().at(rc.image);

            const RenderData renderData = {
                rc.image, clipSource, clipDestination,
                (int)rc.zindex_base, (int)(mc.pos.y + textureData.height + rc.yoffset)
            };

    		pq.push(renderData);
    	}

        //Dont move this piece of code. A reference to this element is used above!
        drawQueue.pop();
    }

    //Put the fps on tmp (text->surface->tmp->fontTexture->default render target)
    const std::string str = std::to_string((int)(1/deltaTime->delta())) + "fps";
    printText(Text(str, 0, 0, {231, 195, 175}));

    //If all active ids werent rendered this frame, warn
    if(allowedRendersThisFrame <= 0) {
        const std::string str = "WARNING: renderer is lagging behind";
        printText(Text(str, 0, 20, {255, 55, 55}));
    }
}

unsigned int RenderSystem::count() const {
	return ids.size();
}

const System::Identifier RenderSystem::getIdentifier() const {
	return System::RENDER;
}

void RenderSystem::activateId(ID id) {
	//Only make the id active if it is a member of rendersystem
	if(ids.find(id) != ids.end()) {
		activeIds.push(id);
	}
}

void RenderSystem::printText(const Text& text) {
    renderer->printText(text);
}

priority_queue<RenderData>& RenderSystem::getDrawPriorityQueue() {
    return pq;
}









//
