#ifndef MESSAGETYPES_HPP
#define MESSAGETYPES_HPP

enum MESSAGE_TYPE {
    OUTDATED,
    CONNECT,
    DISCONNECT,
    INPUTDATA,
    BEGIN_TRANSMITTING_INITIAL_COMPONENTS,
    INITIAL_COMPONENTS,
    END_TRANSMITTING_INITIAL_COMPONENTS,
    MOVECOMPONENTSDIFF,
    RENDERCOMPONENTSDIFF,
    PLAY_SOUND,
    REGISTER_ID_TO_SYSTEM,
    REMOVE_ID,
    REMOVE_ID_FROM_SYSTEM,
    REMOVE_ID_FROM_SYSTEMS,
    ACTIVATE_ID,
    CONGESTED_CLIENT,
    NOT_CONGESTED_CLIENT,
    KEEP_ALIVE,
};

#endif //MESSAGETYPES_HPP
