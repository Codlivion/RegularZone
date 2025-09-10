#pragma once

#include "olcPixelGameEngine.h"
#include "olcSoundWaveEngine.h"

#define TILE_SIZE 16
#define SCREEN_W 256
#define SCREEN_H 224

struct Object;
struct Particle;

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
    std::vector<Object*> units;
    std::vector<Object*> added;
    olc::sound::WaveEngine* soundEngine;
    std::vector<olc::sound::Wave*> sfx;
    std::vector<float> sfxTimer;
    std::vector<Particle*> particles;
    std::vector<int> itemVertices;
    std::vector<std::string> itemLetters;
    std::vector<olc::Pixel> itemColors;
    int screenTransition = 0;

    void PlaySFX(int i);
    void CastParticle(olc::vf2d pos);

private:
    GLBL();
};