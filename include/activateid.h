#ifndef ACTIVATEID_H

#include "icommand.h"

class SystemManager;
typedef unsigned long long int* id;

/**
 *
 *  Activates an id within a system
 *  This class should be used in conjunction with
 *  OnMoveEventComponent, populating its list.
 *  This allows for different entities to activate
 *  different systems. Just create a OnMoveEventComponent
 *  and fill it's list with different commands. Whenever
 *  something invokes onMoveEvent() for a particular entity
 *  all those commands will get invoked. Populate the list
 *  within OnMoveEventComponent during entity creation.
 *
 **/
class ActivateId : public ICommand {
private:
    const string systemIdentifier;
    const ID id;
    SystemManager* const systemManager;

public:
    ActivateId(ID id, const string& systemIdentifier, SystemManager* const systemManager) :
        systemIdentifier(systemIdentifier),
        id(id),
        systemManager(systemManager) { }

    void execute() {
        systemManager->getSystem(systemIdentifier)->activateId(id);
    }
};

#endif //ACTIVATEID_H
