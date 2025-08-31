#pragma once

#include "olcPixelGameEngine.h"

#define TILE_SIZE 16
#define SCREEN_W 256
#define SCREEN_H 224

struct Object;

class GLBL
{
public:
    static GLBL& get()
    {
        static GLBL me;
        return me;
    }
    GLBL(GLBL const&) = delete;
    void operator=(GLBL const&) = delete;

    Object* player = nullptr;
    std::vector<Object*> units; //Updated Units
    std::vector<Object*> added; //Temporary Pointers (Not Owned)
    std::vector<int> itemVertices;
    std::vector<std::string> itemLetters;
    std::vector<olc::Pixel> itemColors;
    int screenTransition = 0;

private:
    GLBL();
};