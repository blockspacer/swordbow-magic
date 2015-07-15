#include "attacksystem.hpp"
#include "hashgridsystem.hpp"
#include "componentmanager.hpp"
#include "systemmanager.hpp"
#include "soundcomponent.hpp"
#include "namecomponent.hpp"
#include "client.hpp"
#include "socket.hpp"
#include "packet.hpp"
#include "messagetypes.hpp"

#include <iostream>
using namespace std;

AttackSystem::AttackSystem(
    HashGridSystem* hashgrid,
    unordered_map<IpAddress, ID>* clients,
    Socket* socket) :
    hashgrid(hashgrid),
    clients(clients),
    socket(socket) {

}

void AttackSystem::add(ID id) {
    ids.insert(id);
}
void AttackSystem::remove(ID id) {
    if(ids.find(id) == ids.end()) {
        cout << "WARNING: Tried to remove unpresent ID from AttackSystem!" << endl;
    }
    ids.erase(id);
}
void AttackSystem::update() {
    while(!activeIds.empty()) {
        auto id = activeIds.front(); activeIds.pop();
        auto& ac = componentManager->attackComponents.at(id);
        auto& mc = componentManager->moveComponents.at(id);

        //The id is active, that means it attacked.
        //First, calculate what the absolute impact area is
        //from the relative impact area, ie the area that
        //the entity attacked taking the entities position into account

        //Calculate impactarea of a blow/strike
        //In words: Draw a vector of length 'weaponLength' from the center of the
        //entity in it's facing direction. Place a box with the size of ac.impactArea
        //with the tip of the vector as the box's center
        float x = mc.pos.x + ac.impactArea.x + mc.dir.x*ac.weaponLength - ac.impactArea.w/2;
        float y = mc.pos.y + ac.impactArea.y + mc.dir.y*ac.weaponLength - ac.impactArea.h/2;
        if(x < 0 || y < 0) return; //If someone tries to strike outside of world, disallow the strike

        const Rect impactArea {
            x,
            y,
            ac.impactArea.w,
            ac.impactArea.h };

        //So the entity probably attacked within some area
        //Query that area, for all entities that can be damaged
        //within that area (IE all entities with a healthcomponent),
        //apply the damage to their healthcomponent

        //First of all, check that sufficient amount of time has bypassed
        //since last attack...
        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ac.startTime).count();
        if(diff > ac.delay) {
            ac.startTime = std::chrono::high_resolution_clock::now();

            //Will be set to true if something was actually hit
            //Is checked when playing a default 'hurt' sound
            bool somethingWasHurt = false;

            for(auto attackedId : hashgrid->query(impactArea)) {

                //Only apply damage to entities that has a healthcomponent
                auto hcit = componentManager->healthComponents.find(attackedId);
                if(hcit != componentManager->healthComponents.end()) {
                    auto& hc = get<1>(*hcit);
                    hc.health -= ac.damage;
                    somethingWasHurt = true;
                }
            }

            if(somethingWasHurt) {
                //Play a default 'hurt'-sound for
                SoundComponent::SoundData hurtSound = {Sound::HURT};

                //Broadcast hurtsound to all clients
                for(auto it : *clients) {
                    using Type = Packet<SoundComponent::SoundData>;
                    auto packet = Type {
                		stringhash("swordbow-magic"),
                		MESSAGE_TYPE::PLAY_SOUND,
                		hurtSound,
                		sizeof(hurtSound)
                	};
                	socket->send<Type>(it.first, packet);
                }
            }
        }

    }
}
unsigned int AttackSystem::count() const {
    return ids.size();
}
const System::Identifier AttackSystem::getIdentifier() const {
    return System::ATTACK;
}
void AttackSystem::activateId(ID id) {
    if(ids.find(id) == ids.end()) {
        return;
    }
    activeIds.push(id);
}
