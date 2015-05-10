#include "healthsystem.hpp"
#include <iostream>
#include "healthcomponent.hpp"
#include "componentmanager.hpp"

using std::cout;
using std::endl;

void HealthSystem::add(ID id) {
    ids.insert(id);
}
void HealthSystem::remove(ID id) {
    if(ids.find(id) == ids.end()) {
        cout << "WARNING: Tried to remove unpresent ID from HealthSystem. Anything can happen!" << endl;
        return;
    }
    ids.erase(id);
}
void HealthSystem::update() {
    for(auto id : ids) {
        auto& hc = componentManager->healthComponents.at(id);
        hc.health--;
        //Check if id is dead
        if(hc.health <= 0) {
            //Activate this id within some "removalSystem" of some sort
        }
    }
}
unsigned int HealthSystem::count() const {
    return ids.size();
}
const string HealthSystem::getIdentifier() const {
    return "HealthSystem";
}
void HealthSystem::activateId(ID id) {

}
