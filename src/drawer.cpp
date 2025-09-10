#include "drawer.h"

Drawer::Drawer() {}

void Drawer::init(olc::PixelGameEngine* _pge_)
{
    pge = _pge_;
}

void Drawer::CreatePolygon(std::vector<olc::vf2d>& v, int n, olc::vf2d o)
{
    v.clear();
    for (int i = 0; i < n; i++)
        v.push_back(olc::vf2d(8 * cos(2 * PI * i / n), 8 * sin(2 * PI * i / n)));
    for (int i = 0; i < v.size(); i++)
        v[i] =
    {
        (v[i].x * cosf(-PI / 2)) - (v[i].y * sinf(-PI / 2)) + o.x,
        (v[i].x * sinf(-PI / 2)) + (v[i].y * cosf(-PI / 2)) + o.y,
    };
}

void Drawer::DrawRect(olc::vf2d p, olc::vf2d s, olc::Pixel c)
{
    if (pge != nullptr) pge->FillRectDecal(p, s, c);
}

void Drawer::DrawPolygon(std::vector<olc::vf2d> v, olc::Pixel c)
{
    if (pge == nullptr || v.size() < 3) return;

    for (int i = 0; i < v.size(); i++)
        pge->DrawLineDecal({ v[i].x, v[i].y }, { v[(i + 1) % v.size()].x, v[(i + 1) % v.size()].y }, c);
}

void Drawer::DrawPolygonFilled(std::vector<olc::vf2d> v, olc::Pixel c)
{
    if (pge == nullptr || v.size() < 3) return;

    pge->DrawPolygonDecal(nullptr, v, v, c);
}

void Drawer::DrawChar(olc::vf2d p, std::string c)
{
    if (pge != nullptr) pge->DrawStringDecal(p, c);
}