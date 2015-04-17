#include "hashgridsystem.h"
#include "componentmanager.h"
#include <iostream>
#include "movecomponent.h"
#include "sizecomponent.h"
#include "rendercomponent.h"
#include "rendersystem.h"
#include "namecomponent.h"

using std::cout;
using std::endl;

HashGridSystem::HashGridSystem(ComponentManager* componentManager, const unsigned int worldWidth, const unsigned int worldHeight, const unsigned int side) :
		componentManager(componentManager),
		side(side),
		width(worldWidth / side + (worldWidth % side != 0)),
		height(worldHeight / side + (worldHeight % side != 0)),
		cellsCapacity(width * height),
		cells(new unordered_set<ID>[width * height])
		 {
}

HashGridSystem::~HashGridSystem() {
	delete[] cells;
}

void HashGridSystem::add(const ID id) {
	ids.insert(id);
	activeIds.push(id);
}

void HashGridSystem::remove(const ID id) {
	ids.erase(id);
	removeFromCells(id);
}

void HashGridSystem::addToCells(const ID id) {
	const auto& mc = componentManager->moveComponents.at(id);
	const auto& sc = componentManager->sizeComponents.at(id);
	const auto& rc = componentManager->renderComponents.at(id);

	unsigned int cellx = (mc->xpos + rc->xoffset)/side;
	unsigned int cellxw = (mc->xpos + rc->xoffset + rc->textureData.width-1)/side;
	unsigned int celly = (mc->ypos + rc->yoffset)/side;
	unsigned int cellyh = (mc->ypos + rc->yoffset + rc->textureData.height-1)/side;

	//Place ID in all cells which partially or completely contains the ID
	for(unsigned int y = celly; y <= cellyh; y++) {
		for(unsigned int x = cellx; x <= cellxw; x++) {
			cells[y*width + x].insert(id);
		}
	}
}

void HashGridSystem::removeFromCells(const ID id) {
	const auto& mc = componentManager->moveComponents.at(id);
	const auto& sc = componentManager->sizeComponents.at(id);
	const auto& rc = componentManager->renderComponents.at(id);

	unsigned int cellx = (mc->xpos + rc->xoffset)/side;
	unsigned int cellxw = (mc->xpos + rc->xoffset + rc->textureData.width-1)/side;
	unsigned int celly = (mc->ypos + rc->yoffset)/side;
	unsigned int cellyh = (mc->ypos + rc->yoffset + rc->textureData.height-1)/side;

	//Remove ID in all cells which previously contained this ID
	for(unsigned int y = celly; y <= cellyh; y++) {
		for(unsigned int x = cellx; x <= cellxw; x++) {
			if(cells[y*width + x].erase(id) == 0) {
				/*cout << "ERROR: hashgrid tried to erase id " << *id << " from cell " << y*width + x << " but the ID wasn't there." << endl;
				cout << "Strange things may happen from now on, because the ID could remain in cells ";
				cout << "where the ID isnt physically located anymore (hashgrid got desynced)" << endl;*/
			};
		}
	}
}

void HashGridSystem::removeFromCellsOldBoundingBox(const ID id) {
	const auto& mc = componentManager->moveComponents.at(id);
	const auto& sc = componentManager->sizeComponents.at(id);
	const auto& rc = componentManager->renderComponents.at(id);

	unsigned int cellx = (mc->oldXpos + rc->xoffset)/side;
	unsigned int cellxw = (mc->oldXpos + rc->xoffset + rc->textureData.width-1)/side;
	unsigned int celly = (mc->oldYpos + rc->yoffset)/side;
	unsigned int cellyh = (mc->oldYpos + rc->yoffset + rc->textureData.height-1)/side;

	//Remove ID in all cells which previously contained this ID
	for(unsigned int y = celly; y <= cellyh; y++) {
		for(unsigned int x = cellx; x <= cellxw; x++) {
			if(cells[y*width + x].erase(id) == 0) {
				/*cout << "ERROR: hashgrid tried to erase id " << *id << " from cell " << y*width + x << " but the ID wasn't there." << endl;
				cout << "Strange things may happen from now on, because the ID could remain in cells ";
				cout << "where the ID isnt physically located anymore (hashgrid got desynced)" << endl;
				cout << "as of 2015-04-16 this error-message occurs during initialization of gameworld," << endl;
				cout << "in that case you need not to worry about the error message. It happens since" << endl;
				cout << "adding to the HashGridSystem is done BEFORE world.cpp sets appropiate coordinates of each tile" << endl;
				cout << "thus when the tile becomes active it tries to remove itself from {0,0}. This is of course harmless" << endl;
				cout << "and can be ignored" << endl;*/
			};
		}
	}
}

//Returns all entities that are fully or partly contained by id's texture bounding box
void HashGridSystem::overlaps(unordered_set<ID>& overlappingEntities, const ID id) {
	const auto& mc = componentManager->moveComponents.at(id);
	const auto& rc = componentManager->renderComponents.at(id);

	const SpatialIndexer::Rect queryRect {
		mc->xpos + rc->xoffset,
		mc->ypos + rc->yoffset,
		rc->textureData.width-1,
		rc->textureData.height-1
	};

	//query using textures bounding box
	query(overlappingEntities, queryRect);

	overlappingEntities.erase(id);
}

//Return all entities that are fully or partly contained by a queryArea
void HashGridSystem::query(unordered_set<ID>& queryIds, Rect queryArea) {

	//Convert from world-space coordinates to cellCoordinates and store
	//cellCoordinates in cellRect
	const SpatialIndexer::Rect cellRect {
		queryArea.x/side,
		queryArea.y/side,
		queryArea.w/side,
		queryArea.h/side,
	};

	//Loop through all cells in which this ID is partly or fully contained
	for(unsigned int y = cellRect.y; y <= (queryArea.y + queryArea.h)/side; y++) {
		for(unsigned int x = cellRect.x; x <= (queryArea.x + queryArea.w)/side; x++) {

			//For all ids in the same cells as this one..
			for(auto otherId : cells[y*width + x]) {
				const auto& omc = componentManager->moveComponents.at(otherId);
				const auto& osc = componentManager->sizeComponents.at(otherId);
				const auto& orc = componentManager->renderComponents.at(otherId);

				if(intersect(
					queryArea,
					SpatialIndexer::Rect {
						omc->xpos + orc->xoffset,
						omc->ypos + orc->yoffset,
						orc->textureData.width-1,
						orc->textureData.height-1 }
					)
				) {
					queryIds.insert(otherId);
				}
			}
		}
	}
}

void HashGridSystem::getNearbyIds(unordered_set<ID>& nearbyIds, const ID id) {

}

void HashGridSystem::makeIdActive(const ID id) {
	//if the id is a member of hashgrid...
	if(ids.find(id) != ids.end()) {
		//then it is legit to make it an active id
		activeIds.push(id);
	}
}

void HashGridSystem::update() {
	while(!activeIds.empty()) {
		auto id = activeIds.front(); activeIds.pop();

		//search cells bounded by old bounding box for id
		//and remove id from those cells
		removeFromCellsOldBoundingBox(id);

		//add id to the cells bounded by current bounding box for id
		addToCells(id);
	}
}

unsigned int HashGridSystem::count() const {
	return ids.size();
}
const string HashGridSystem::getIdentifier() const {
	return "HashGridSystem";
}
