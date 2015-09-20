#ifndef MESSAGETYPES_HPP
#define MESSAGETYPES_HPP

enum MESSAGE_TYPE {
    OUTDATED,
    CONNECT,
    DISCONNECT,
    INPUTDATA,
    INITIAL_COMPONENTS,
    MOVECOMPONENTS,
    RENDERCOMPONENTS,
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
};

#endif //MESSAGETYPES_HPP
