#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "isystem.h"
#include <unordered_set>
#include <queue>
#include "spatialindexer.h"

using std::unordered_set;
using std::queue;

class MoveComponent;
class SizeComponent;
class SpatialIndexer;

typedef unsigned long long int* ID;

class CollisionSystem : public ISystem {
private:
	unordered_set<ID> ids;
	queue<ID> activeIds;
	SpatialIndexer* spatialIndexer;

public:
	CollisionSystem(SpatialIndexer* spatialIndexer);
	void add(ID id);
	void remove(ID id);
	void update();
	unsigned int count() const ;
	const string getIdentifier() const;
	void makeIdActive(ID id);

};

#endif //COLLISIONSYSTEM_H
