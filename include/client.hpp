#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "deltatime.hpp"
#include "componentmanager.hpp"
#include "systemmanager.hpp"
#include "renderer.hpp"
#include "rendersystem.hpp"
#include "hashgridsystem.hpp"
#include "camerasystem.hpp"
#include "textureboundingbox.hpp"
#include "soundengine.hpp"
#include "socket.hpp"
#include "messagetypes.hpp"
#include "timer.hpp"

#include <SDL2/SDL.h>
#include <mutex>
#include <thread>

typedef unsigned int ID;

class Client {
private:
    Socket socket;
    IpAddress server;
    uint16_t sequence;

    std::thread receiveThread;
    bool receiveThreadRunning;

    SDL_Event event;

    DeltaTime deltaTime;
    ComponentManager componentManager;
    SystemManager systemManager;
    Renderer renderer;
    TextureBoundingBox textureBoundingBox;
    Timer keepAlive;
    static constexpr float secondsUntilTimeout = 10;

    //Client-side systems
    HashGridSystem textureHashGridSystem;
    CameraSystem cameraSystem;
    RenderSystem renderSystem;
    SoundEngine soundEngine;

    std::mutex componentsMutex;

    //ID that this client controlls
    ID playerId;

    void receive();

    //Helper method for sending packets
    template<class DataType>
    void send(DataType data, MESSAGE_TYPE message) {

        using PacketType = Packet<DataType>;
        PacketType cameraPacket = {
            stringhash("swordbow-magic"),
            sequence++,
            message,
            data,
            sizeof(data)
        };

        socket.send<PacketType>(server, cameraPacket);
    }

public:
    bool running = true;

    Client(int argc, char** argv);
    ~Client();

    void connect(const IpAddress& server);
    void disconnect();
    void run();
    void step();
};

#endif //CLIENT_HPP
