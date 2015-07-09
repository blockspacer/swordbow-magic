#include "movediffsystem.hpp"
#include "componentmanager.hpp"

void MoveDiffSystem::add(ID id) {
    ids.push(id);
}
void MoveDiffSystem::remove(ID id) {

}
void MoveDiffSystem::update() {
    componentManager->moveComponentsDiff.clear();
    while(!ids.empty()) {
        const auto id = ids.front(); ids.pop();
        componentManager->moveComponentsDiff.insert({id, componentManager->moveComponents.at(id)});
    }
}
unsigned int MoveDiffSystem::count() const {
    return ids.size();
}
const System::Identifier MoveDiffSystem::getIdentifier() const {
    return System::MOVEDIFF;
}
void MoveDiffSystem::activateId(ID id) {
    add(id);
}
