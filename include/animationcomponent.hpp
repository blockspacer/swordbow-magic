#ifndef ANIMATIONCOMPONENT_HPP
#define ANIMATIONCOMPONENT_HPP

#include <vector>
#include <string>
#include <chrono>

using std::vector;
using std::string;

class AnimationComponent {
public:
    class Animation {
    public:
        vector<string> frames;
        unsigned short currentFrame = 0;
        std::chrono::high_resolution_clock::time_point startTime;
        unsigned int duration = 150; //duration per frame in milliseconds
        bool looping = true;
        bool firstLoop = true;
    };

    //Groups animations in different directions together
    class AnimationGroup {
    public:
        Animation north;
        Animation northEast;
        Animation east;
        Animation southEast;
        Animation south;
        Animation southWest;
        Animation west;
        Animation northWest;
    };

    AnimationGroup idle;
    AnimationGroup walking;
    Animation bloodsplatter;

    Animation* state;
};

#endif //ANIMATIONCOMPONENT_HPP
