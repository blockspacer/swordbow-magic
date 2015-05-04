#include "world.hpp"
#include "entitymanager.hpp"
#include "componentmanager.hpp"
#include "movecomponent.hpp"
#include "rendercomponent.hpp"
#include "sizecomponent.hpp"

World::World(EntityManager* entityManager)
	: entityManager(entityManager) {
	for(short y = 0; y < NUM_TILES; y++) {
		for(short x = 0; x < NUM_TILES; x++) {
			auto id  = entityManager->createTile();
			tiles[x][y] = id;

			//Align tiles into a grid, spatially
			auto& mc = entityManager->componentManager->moveComponents.at(id);
			mc.xpos = x * TILE_SIZE;
			mc.ypos = y * TILE_SIZE;
			mc.oldXpos = x * TILE_SIZE;
			mc.oldYpos = y * TILE_SIZE;

			auto sc = entityManager->componentManager->sizeComponents.at(id);
			sc.width = TILE_SIZE;
			sc.height = TILE_SIZE;
		}
	}
}
