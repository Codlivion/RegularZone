#pragma once

#include "olcPixelGameEngine.h"

#define PI 3.14159

class Drawer
{
public:
    static Drawer& get()
    {
        static Drawer me;
        return me;
    }
    Drawer(Drawer const&) = delete;
    void operator=(Drawer const&) = delete;

    olc::PixelGameEngine* pge = nullptr;

private:
    Drawer();

public:
    void init(olc::PixelGameEngine* pge);
    void static CreatePolygon(std::vector<olc::vf2d>& v, int n, olc::vf2d o);
    void DrawPolygon(std::vector<olc::vf2d> v, olc::Pixel c);
    void DrawPolygonFilled(std::vector<olc::vf2d> v, olc::Pixel c);
    void DrawChar(olc::vf2d p, std::string c);
};