#pragma once

#include "olcPixelGameEngine.h"
#include "model.h"
#include <iostream>
#include <fstream>

#define GRID_SIZE 16 * 14

struct Screen
{
	int   tiles[GRID_SIZE];
	int   items[GRID_SIZE];
	int enemies[GRID_SIZE];

	void Initiate(std::vector<Object*>& enemies);
};

struct World
{
	std::vector<int> map;
	std::vector<Screen*> screens;

	World();
	void Load();
};