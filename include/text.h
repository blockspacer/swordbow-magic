#ifndef TEXT_H
#define TEXT_H

#include <SDL2/SDL_ttf.h>
#include <string>

using namespace std;

class Text {
public:
    const string text;
    const SDL_Color color;
    const int x, y;
    Text(const string& text, const int& x, const int& y, const SDL_Color& color = {255, 255, 255, 255});
};




#endif //TEXT_H
