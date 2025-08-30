#include "scene.h"

World::World()
{
	map =
	{
	     -1,  -1,  -1,  -1,  -1,   0,   1,   2,   3,   4,  -1,  -1,  -1,  -1,  -1,  -1,
	      5,   6,   7,   8,  -1,   9,  10,  11,  12,  13,  -1,  14,  15,  16,  17,  18,
	     19,  20,  21,  22,  -1,  23,  24,  25,  26,  27,  -1,  28,  29,  30,  31,  32,
	     33,  34,  35,  36,  -1,  37,  38,  39,  40,  41,  -1,  42,  43,  44,  45,  46,
	     47,  48,  49,  50,  -1,  -1,  -1,  51,  -1,  -1,  -1,  52,  53,  54,  55,  56,
	     57,  58,  59,  60,  -1,  61,  62,  63,  64,  65,  -1,  66,  67,  68,  69,  70,
	     71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,
	     -1,  -1,  -1,  -1,  -1,  87,  88,  89,  90,  91,  -1,  -1,  -1,  -1,  -1,  -1,
	     92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107,
	    108, 109, 110, 111,  -1, 112, 113, 114, 115, 116,  -1, 117, 118, 119, 120, 121,
	    122, 123, 124, 125,  -1,  -1,  -1, 126,  -1,  -1,  -1, 127, 128, 129, 130, 131,
	    132, 133, 134, 135,  -1, 136, 137, 138, 139, 140,  -1, 141, 142, 143, 144, 145,
	    146, 147, 148, 149,  -1, 150, 151, 152, 153, 154,  -1, 155, 156, 157, 158, 159,
	    160, 161, 162, 163,  -1, 164, 165, 166, 167, 168,  -1, 169, 170, 171, 172, 173,
	     -1,  -1,  -1,  -1,  -1, 174, 175, 176, 177, 178,  -1,  -1,  -1,  -1,  -1,  -1,
	     -1,  -1,  -1,  -1,  -1, 179, 180, 181, 182, 183,  -1,  -1,  -1,  -1,  -1,  -1
	};
	Load();
}

void Screen::Initiate(std::vector<Object*>& enemyPool)
{
	GLBL::get().units.clear();
	GLBL::get().units.push_back(GLBL::get().player);
	int poolIndex = 0;
	for (int i = 0; i < GRID_SIZE; i++) {
		if (enemies[i] > 0) {
			olc::vf2d pos = olc::vf2d((i % 16) * 16 + 8, (i / 16) * 16 + 8);
			GLBL::get().added.push_back(UnitBuilder::BuildEnemy(pos, enemies[i], enemyPool[poolIndex]));
			enemyPool[poolIndex]->active = true;
			poolIndex++;
		}
		if (poolIndex >= enemyPool.size()) break;
	}
}

void World::Load()
{
	std::ifstream file("world.txt");
	std::string data;
	for (int w = 0; w < 184; w++) {
		screens.push_back(new Screen());
		for (int i = 0; i < GRID_SIZE; i++) {
			file >> data;
			screens[w]->tiles[i] = std::stoi(data);
		}
		for (int i = 0; i < GRID_SIZE; i++) {
			file >> data;
			screens[w]->items[i] = std::stoi(data);
		}
		for (int i = 0; i < GRID_SIZE; i++) {
			file >> data;
			screens[w]->enemies[i] = std::stoi(data);
		}
	}
	file.close();
}