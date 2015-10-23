#include "rendersystem.hpp"
#include "deltatime.hpp"
#include "componentmanager.hpp"
#include "rendercomponent.hpp"
#include "movecomponent.hpp"
#include "spatialindexer.hpp"
#include "camerasystem.hpp"
#include "renderer.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "boundingbox.hpp"
#include <vector>
#include <thread>
#include <mutex>

RenderSystem::RenderSystem(
    Renderer* renderer,
    SpatialIndexer* spatialIndexer,
    CameraSystem* cameraSystem) :
    renderer(renderer),
    spatialIndexer(spatialIndexer),
    cameraSystem(cameraSystem) { }

void RenderSystem::add(ID id) {
	ids.insert(id);
	activeIds.push(id);
}

void RenderSystem::remove(ID id) {
	ids.erase(id);
}

void RenderSystem::update() {
    Timer updateTimer;
    updateTimer.start();

    //Will contain all rectangles where a redraw is required
    std::vector<Rect> draws;

    //Cap the amount of IDs getting rendered each frame
    //this is only a quick-fix to ensure no program-freeze
    //when initially rendering the whole world
    int allowedRendersThisFrame = 512;
	//For all activeIds in rendersystem...
	while(!activeIds.empty() && allowedRendersThisFrame-- > 0 ) {
		//Draw everything within the activeIds texturearea
        Rect drawArea {0, 0, 0, 0};
        const auto id = activeIds.front();
        const auto& renderComponent = componentManager->renderComponents.at(id);
        try {
            drawArea = getBoundingBox(
                componentManager->moveComponents.at(id).pos,
                renderer->getTextureDatas().at(renderComponent.image).dimension,
                renderComponent.offset
            );

            //Draw the new area that the entity moved to
            draws.push_back(drawArea);

            //Also draw the area that the entity was before
            draws.push_back(oldDrawAreas[id]);

            //Save drawarea so that next time this entity moves,
            //draw the area that will be drawn this time
            oldDrawAreas[id] = drawArea;

        } catch (std::out_of_range oor) {
            std::ostringstream oss;
            oss << "Couldn't render id " << id << " because boundingbox couldn't be retrieved";
            Logger::log(oss, Log::ERROR);
        }

        activeIds.pop();
	}

    std::priority_queue<RenderData> pq;
    std::mutex pqlock;
    std::mutex qlock;
    static constexpr char NUMBER_OF_WORKERS = 4;
    const int WORK_PER_WORKER = draws.size() / NUMBER_OF_WORKERS;

    const auto work = [this, &draws, &pq, &pqlock](const auto& beginIterator, const auto& endIterator) {
        for(auto it = beginIterator; it != endIterator; it++) {
            const auto& drawArea = *it;
            for(auto id : spatialIndexer->query(drawArea)) {
                const auto& mc = componentManager->moveComponents.at(id);
                const auto& rc = componentManager->renderComponents.at(id);
                const auto& texture = renderer->getTextureDatas().at(rc.image);

                //Get the intersection between an entity within drawarea and the drawarea
                const auto intersection = Rect::getIntersection(
                    drawArea, getBoundingBox(mc.pos, texture.dimension, rc.offset)
                );

                const SDL_Rect clipSource = {
                    int(intersection.x) - int(mc.pos.x) - int(rc.offset.x),
                    int(intersection.y) - int(mc.pos.y) - int(rc.offset.y),
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
                    (int)rc.zindex_base, (int)(mc.pos.y + textureData.dimension.y + rc.offset.y)
                };

                std::ostringstream oss;
                oss << "Rendering id " << id;
                Logger::log(oss, Log::VERBOSE);

                pqlock.lock();
                pq.push(renderData);
                pqlock.unlock();
            }
        }
    };

    auto begin = draws.begin(), end = draws.end();
    std::array<std::thread, NUMBER_OF_WORKERS> threads {
        std::thread{work, begin + 0*WORK_PER_WORKER, begin + 1*WORK_PER_WORKER},
        std::thread{work, begin + 1*WORK_PER_WORKER, begin + 2*WORK_PER_WORKER},
        std::thread{work, begin + 2*WORK_PER_WORKER, begin + 3*WORK_PER_WORKER},
        std::thread{work, begin + 3*WORK_PER_WORKER, end}
    };

    for(auto& thread : threads) { thread.join(); }

    const auto updateElapsed = updateTimer.elapsed();
    if(updateElapsed > 1.0f/50.0f) {
        std::ostringstream oss;
        oss << "Calculating drawboxes took " << updateElapsed << "s. This is a probable cause of stuttering.";
        Logger::log(oss, Log::WARNING);
    }

    //Put the fps on tmp (text->surface->tmp->fontTexture->default render target)
    const std::string str = std::to_string((int)(1/deltaTime->delta())) + "fps";
    printText(Text(str, 0, 0, {231, 195, 175}));

    //If all active ids werent rendered this frame, warn
    if(allowedRendersThisFrame <= 0) {
        const std::string str = "WARNING: renderer is lagging behind";
        printText(Text(str, 0, 20, {255, 55, 55}));
    }

    renderer->render(pq, cameraSystem->getCamera());
}

unsigned int RenderSystem::count() const {
	return ids.size();
}

const System::Identifier RenderSystem::getIdentifier() const {
	return System::RENDER;
}

bool RenderSystem::activateId(ID id) {
	//Only make the id active if it is a member of rendersystem
	if(ids.find(id) != ids.end()) {
		activeIds.push(id);
        return true;
	}
    return false;
}

void RenderSystem::printText(const Text& text) {
    renderer->printText(text);
}







//
