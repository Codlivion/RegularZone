#pragma once

#include "olcPixelGameEngine.h"
#include "model.h"
#include "scene.h"

class PhysicsEngine
{
public:
	static bool ShapeOverlap_DIAGS(WireFrame& r1, WireFrame& r2);
	static bool ShapeOverlap_DIAGS_STATIC(WireFrame& r1, WireFrame& r2);
	static bool CircleVsTile(WireFrame& r1, Screen& screen, std::vector<Object*>& itemPool);
};