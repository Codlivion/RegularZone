#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "global.h"
#include "model.h"
#include "physics.h"
#include "scene.h"
#include "drawer.h"

GLBL::GLBL() {}

enum GameState { MAIN_MENU, IN_GAME, STATUS, ENDING };

class Game : public olc::PixelGameEngine
{
public:
	Game()
	{
		sAppName = "Regular Zone";
	}

public:
	GameState gameState = MAIN_MENU;
	World* world = nullptr;
	Screen* screen;
	int originalTiles[GRID_SIZE];
	int currentScreen = 119;
	float restartTimer = 2.f;
	std::vector<Object*> enemyPool;
	std::vector<Object*> itemPool;
	std::string weaponString[8];
	bool visited[16 * 16];
	Object* endingBall = nullptr;

	bool OnUserCreate() override
	{
		Drawer::get().init(this);

		GLBL::get().itemVertices = { 0, 8, 10, 20, 10, 20, 5, 3, 8, 8, 8, 8, 8, 8, 8, 20 };
		GLBL::get().itemLetters = { "", "V", "", "", "", "", "S", "R", "M", "C", "W", "B", "H", "L", "D", "U"};
		GLBL::get().itemColors =
		{
			olc::YELLOW, olc::GREEN, olc::GREEN, olc::BLUE, olc::BLUE, olc::GREY, olc::GREY, olc::YELLOW,
			olc::DARK_CYAN, olc::DARK_MAGENTA, olc::DARK_YELLOW, olc::DARK_RED, olc::CYAN, olc::WHITE, olc::WHITE
		};

		weaponString[0] = "NONE";
		weaponString[1] = "MULTI";
		weaponString[2] = "BALL";
		weaponString[3] = "WAVE";
		weaponString[4] = "BOMB";
		weaponString[5] = "HOMNG";
		weaponString[6] = "LASER";
		weaponString[7] = "DSCHRG";

		world = new World();
		GLBL::get().player = UnitBuilder::BuildPlayer();
		for (int i = 0; i < 10; i++)
			enemyPool.push_back(UnitBuilder::BuildEnemy({ 0.f, 0.f }, 0));
		for (int i = 0; i < 3; i++)
			itemPool.push_back(UnitBuilder::BuildItem({ 32.f + i * 256.f / 4, 180.f }, 0));
		screen = world->screens[world->map[currentScreen]];
		screen->Initiate(enemyPool);
		for (int i = 0; i < GRID_SIZE; i++) originalTiles[i] = screen->tiles[i];

		for (auto& b : visited) b = false;
		visited[currentScreen] = true;
		endingBall = new Object(olc::vf2d(128, 112), 32, 32);
		endingBall->model->TransformModel(endingBall->origin);

		//All Weapons Cheat:
		//for (auto& u : GLBL::get().player->shootModule->unlocked) u = true;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		switch (gameState)
		{
		case MAIN_MENU:
		{
			DrawStringDecal(olc::vi2d(32,  64), "REGULAR ZONE", olc::WHITE, olc::vf2d(2, 2));
			DrawStringDecal(olc::vi2d(32, 144), "Press Enter to Start Game");
			DrawStringDecal(olc::vi2d(32, 224), "Developed by Codlivion");
			DrawStringDecal(olc::vi2d(32, 240), "for the olcCodeJam2025");
			//Keys?

			if (GetKey(olc::ENTER).bPressed) gameState = IN_GAME;

			break;
		}
		case IN_GAME:
		{
			if (GLBL::get().screenTransition != 0) {
				int temp = currentScreen + GLBL::get().screenTransition;
				if (temp >= 0 && temp <= 255 && world->map[currentScreen] > -1) {
					for (int i = 0; i < GRID_SIZE; i++) screen->tiles[i] = originalTiles[i];
					currentScreen += GLBL::get().screenTransition;
					screen = world->screens[world->map[currentScreen]];
					screen->Initiate(enemyPool);
					for (int i = 0; i < GRID_SIZE; i++) originalTiles[i] = screen->tiles[i];
					visited[currentScreen] = true;
				}
				GLBL::get().screenTransition = 0;
			}

			//Input and Controls: (if player active)
			Object* player = GLBL::get().player;
			std::vector<Object*>& units = GLBL::get().units;
			std::vector<Object*>& added = GLBL::get().added;
			if (player->active) {
				//Rotate:
				if (GetKey(olc::Q).bHeld) player->angle -= 3.f * fElapsedTime;
				else if (GetKey(olc::E).bHeld) player->angle += 3.f * fElapsedTime;
				//Move Forward:
				if (GetKey(olc::W).bHeld) {
					player->velocity.x += cosf(player->angle) * player->speed * fElapsedTime;
					player->velocity.y += sinf(player->angle) * player->speed * fElapsedTime;
				}
				else {
					player->velocity -= player->velocity * fElapsedTime;
					if (player->velocity.x * player->velocity.x + player->velocity.y * player->velocity.y < 0.1f)
						player->velocity = { 0.f, 0.f };
				}
				//Block:
				if (GetKey(olc::SPACE).bPressed) player->shootModule->Shoot(1);
				//Shoot:
				if (GetKey(olc::CTRL).bHeld || GetKey(olc::NP1).bHeld) {
					if (player->shootModule->weaponIndex != 0) player->shootModule->Shoot(player->shootModule->weaponIndex + 2);
					else player->shootModule->Shoot(2);
				}
				//Change Weapon:
				if (GetKey(olc::NP2).bPressed) player->shootModule->PreviousWeapon();
				if (GetKey(olc::NP3).bPressed) player->shootModule->NextWeapon();
				//Thrust:
				if (GetKey(olc::UP).bPressed) player->shootModule->Shoot(101);
				else if (GetKey(olc::DOWN).bPressed) player->shootModule->Shoot(102);
				else if (GetKey(olc::LEFT).bPressed) player->shootModule->Shoot(103);
				else if (GetKey(olc::RIGHT).bPressed) player->shootModule->Shoot(104);
				//Menu:
				if (GetKey(olc::ENTER).bPressed) gameState = STATUS;
			}

			//Update and Draw Units:
			for (auto& u : units) {
				if (u->active) {
					u->Update(fElapsedTime);
					if (u->solid) PhysicsEngine::CircleVsTile(*u->model, *screen, itemPool);
					if (u->shootModule != nullptr) {
						for (auto& a : u->shootModule->weapons) {
							for (auto& w : a->pool) {
								if (a->type == WP_SHIELD) {
									w->origin = u->origin;
									w->angle = u->angle;
								}
								if (w->added) {
									added.push_back(w);
									w->added = false;
									w->active = true;
								}
							}
						}
					}
					u->Draw();
				}
			}

			//Check Collisions:
			for (int i = 0; i < std::max((int)units.size() - 1, 0); i++) {
				if (units[i]->active) {
					for (int j = i + 1; j < units.size(); j++) {
						if (units[j]->active) {
							int col = units[i]->bodyType * 10 + units[j]->bodyType; //SHIELD<->SHIELD Collision??
							switch (col) {
							case 00: {
								if (units[i]->friendly != units[j]->friendly) {
									if (PhysicsEngine::ShapeOverlap_DIAGS(*units[i]->model, *units[j]->model)) {
										//Body<->Body Collision:
										olc::vf2d collisionVec = units[i]->origin - units[j]->origin;
										if (units[i]->friendly) {
											if (!units[i]->life->Consume(1, 0.5f)) units[i]->active = false;
											else {
												units[i]->velocity = collisionVec * 4.f;
												units[i]->flashModule->Initiate(0.5f);
											}
										}
										else {
											if (!units[j]->life->Consume(1, 0.5f)) units[j]->active = false;
											else {
												units[j]->velocity = collisionVec * 4.f;
												units[j]->flashModule->Initiate(0.5f);
											}
										}
									}
								}
								break;
							}
							case 01: case 10: {
								if (units[i]->friendly != units[j]->friendly) {
									if (PhysicsEngine::ShapeOverlap_DIAGS(*units[i]->model, *units[j]->model)) {
										//Projectile<->Body Collision:
										if (units[i]->bodyType == BODY) {
											float d = units[i]->friendly ? 0.5f : 0.2f;
											if (!units[i]->life->Consume(units[j]->Damage(), d)) units[i]->active = false;
											else units[i]->flashModule->Initiate(d);
											units[j]->Collided();
										}
										else {
											float d = units[j]->friendly ? 0.5f : 0.2f;
											if (!units[j]->life->Consume(units[i]->Damage(), d)) units[j]->active = false;
											else units[j]->flashModule->Initiate(d);
											units[i]->Collided();
										}
									}
								}
								break;
							}
							case 02: case 20: {
								if (units[i]->friendly != units[j]->friendly) {
									if (PhysicsEngine::ShapeOverlap_DIAGS(*units[i]->model, *units[j]->model)) {
										//Shield<->Body Collision: (Initiate Flash)
										if (units[i]->bodyType == BODY) {
											if (!units[i]->life->Consume(1, 0.2f)) units[i]->active = false;
											if (!units[j]->life->Consume(1, 0.2f)) units[j]->active = false;
										}
										else {
											if (!units[j]->life->Consume(1, 0.2f)) units[j]->active = false;
											if (!units[i]->life->Consume(1, 0.2f)) units[i]->active = false;
										}
									}
								}
								break;
							}
							case 03: case 30: {
								if (units[i]->friendly == units[j]->friendly) {
									if (PhysicsEngine::ShapeOverlap_DIAGS(*units[i]->model, *units[j]->model)) {
										//Body<->Item Collision:
										Object* unit;
										int itemCode = 0;
										int itemScreenIndex = 0;
										if (units[i]->bodyType == BODY) {
											unit = units[i];
											itemCode = units[j]->item->type;
											units[j]->active = false;
											olc::vi2d truncated = units[j]->origin / 16;
											itemScreenIndex = truncated.y * 16 + truncated.x;
										}
										else {
											unit = units[j];
											itemCode = units[i]->item->type;
											units[i]->active = false;
											olc::vi2d truncated = units[i]->origin / 16;
											itemScreenIndex = truncated.y * 16 + truncated.x;
										}
										switch (itemCode) {
										case 1: unit->model->AddVertex(1);
											player->life->ModifyMax(player->model->originalModel.size() * 5);
											player->energy->ModifyMax(player->model->originalModel.size() * 15);
											for (auto& w : player->shootModule->weapons[2]->pool) w->damage++;
											for (auto& w : player->shootModule->weapons[3]->pool) w->damage++;
											screen->items[itemScreenIndex] = 0;
											break;
										case 2: unit->life->Restore(3, 0);
											break;
										case 3: unit->life->Restore(9, 0);
											break;
										case 4: unit->energy->Restore(9, 0);
											break;
										case 5: unit->energy->Restore(45, 0);
											break;
										case 6:
											if (player->shootModule->weapons[0]->pool[0]->life->value < player->model->originalModel.size())
												unit->shootModule->Shoot(0);
											break;
										case 7:
											if (unit->shootModule->weapons[2]->rateOfFire > 0.2f)
												unit->shootModule->weapons[2]->rateOfFire -= 0.05f;
											screen->items[itemScreenIndex] = 0;
											break;
										case 8: unit->shootModule->unlocked[1] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 9: unit->shootModule->unlocked[2] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 10: unit->shootModule->unlocked[3] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 11: unit->shootModule->unlocked[4] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 12: unit->shootModule->unlocked[5] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 13: unit->shootModule->unlocked[6] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 14: unit->shootModule->unlocked[7] = true;
											screen->items[itemScreenIndex] = 0;
											break;
										case 15: gameState = ENDING;
											break;
										default: break;
										}
									}
								}
								break;
							}
							case 12: case 21: {
								if (units[i]->friendly != units[j]->friendly) {
									if (PhysicsEngine::ShapeOverlap_DIAGS(*units[i]->model, *units[j]->model)) {
										//Projectile<->Shield Collision:
										if (units[i]->bodyType == SHIELD) {
											if (!units[i]->life->Consume(1, 0.2f)) units[i]->active = false;
											units[j]->Collided(true);
										}
										else {
											if (!units[j]->life->Consume(1, 0.2f)) units[j]->active = false;
											units[i]->Collided(true);
										}
									}
								}
								break;
							}
							default: break;
							}
						}
					}
				}
			}

			FillRectDecal(olc::vf2d(0, 224), olc::vf2d(256, 32), olc::BLACK);
			DrawStringDecal({ 8, 228 }, "HP");
			DrawStringDecal({ 8, 244 }, std::to_string(player->life->value));
			DrawStringDecal({ 40, 228 }, "SP");
			DrawStringDecal({ 40, 244 }, std::to_string(player->energy->value));
			DrawStringDecal({ 72, 228 }, "SHD");
			DrawStringDecal({ 72, 244 }, std::to_string(player->shootModule->weapons[0]->pool[0]->life->value));
			DrawStringDecal({ 104, 228 }, "BLK");
			DrawStringDecal({ 104, 244 }, std::to_string((int)(player->shootModule->weapons[1]->shootTimer * 100)));
			DrawStringDecal({ 136, 228 }, "BST");
			DrawStringDecal({ 136, 244 }, std::to_string((int)(player->model->trace->activeTimer * 100)));
			DrawStringDecal({ 168, 228 }, "ROF");
			DrawStringDecal({ 168, 244 }, std::to_string((int)(player->shootModule->weapons[2]->rateOfFire * 100)));
			DrawStringDecal({ 200, 228 }, "WPN");
			DrawStringDecal({ 200, 244 }, weaponString[player->shootModule->weaponIndex]);

			if (!player->active) {
				restartTimer -= fElapsedTime;
				if (restartTimer <= 0.f) {
					restartTimer = 2.f;
					for (int i = 0; i < GRID_SIZE; i++) screen->tiles[i] = originalTiles[i];
					currentScreen = 119;
					screen = world->screens[world->map[currentScreen]];
					screen->Initiate(enemyPool);
					for (int i = 0; i < GRID_SIZE; i++) originalTiles[i] = screen->tiles[i];
					UnitBuilder::BuildPlayer(player);
					return true;
				}
			}

			units.erase(std::remove_if(units.begin(), units.end(), [](Object* u) { return !u->active; }), units.end());
			if (added.size() > 0) {
				units.insert(units.end(), added.begin(), added.end());
				added.clear();
			}

			std::vector<olc::vf2d> symbol;
			olc::vi2d o;
			for (int i = 0; i < GRID_SIZE; i++) {
				if (screen->tiles[i] == 0) continue;
				if (screen->tiles[i] == 1) FillRectDecal(olc::vi2d{ (i % 16) * 16, (i / 16) * 16 }, olc::vi2d{ 16, 16 }, olc::VERY_DARK_GREY);
				else {
					FillRectDecal(olc::vi2d{ (i % 16) * 16, (i / 16) * 16 }, olc::vi2d{ 16, 16 }, olc::GREY);
					if (screen->tiles[i] >= 3) {
						o = olc::vi2d{ (i % 16) * 16 + 8, (i / 16) * 16 + 8 };
						Drawer::CreatePolygon(symbol, screen->tiles[i], o);
						DrawPolygonDecal(nullptr, symbol, symbol, olc::BLUE);
					}
				}
			}

			break;
		}
		case STATUS:
		{
			Object* player = GLBL::get().player;
			int w = player->shootModule->weaponIndex;
			bool success = false;
			for (auto& b : player->shootModule->unlocked)
				if (b) {
					success = true;
					break;
				}
			if (success) {
				if (GetKey(olc::UP).bPressed || GetKey(olc::DOWN).bPressed) {
					do w = (w + 4) % 8;
					while (!player->shootModule->unlocked[w]);
				}
				else if (GetKey(olc::LEFT).bPressed) {
					do w = (w + 7) % 8;
					while (!player->shootModule->unlocked[w]);
				}
				else if (GetKey(olc::RIGHT).bPressed) {
					do w = (w + 1) % 8;
					while (!player->shootModule->unlocked[w]);
				}
				player->shootModule->weaponIndex = w;
			}

			for (int y = 0; y < 16; y++) {
				for (int x = 0; x < 16; x++) {

					if (world->map[y * 16 + x] > -1) {
						if (visited[y * 16 + x]) FillRectDecal(olc::vi2d(64 + x * 8, 16 + y * 8), olc::vi2d(8, 8), olc::BLUE);
						else FillRectDecal(olc::vi2d(64 + x * 8, 16 + y * 8), olc::vi2d(8, 8), olc::DARK_BLUE);
					}
					DrawRectDecal(olc::vi2d(64 + x * 8, 16 + y * 8), olc::vi2d(8, 8), olc::BLACK);
				}
			}
			olc::vi2d currentScreenPos = olc::vi2d(64 + (currentScreen % 16) * 8, 16 + (currentScreen / 16) * 8);
			FillRectDecal(currentScreenPos, olc::vi2d(8, 8), olc::CYAN);

			DrawStringDecal(olc::vi2d( 32, 160), "HP:");
			DrawStringDecal(olc::vi2d( 64, 160), std::to_string(player->life->value));
			DrawStringDecal(olc::vi2d( 96, 160), "SP:");
			DrawStringDecal(olc::vi2d(128, 160), std::to_string(player->energy->value));
			DrawStringDecal(olc::vi2d(160, 160), "SHLD:");
			DrawStringDecal(olc::vi2d(224, 160), std::to_string(player->shootModule->weapons[0]->pool[0]->life->value));
			DrawStringDecal(olc::vi2d( 32, 176), " Damage:");
			DrawStringDecal(olc::vi2d(104, 176), std::to_string(player->shootModule->weapons[w]->pool[0]->damage));
			DrawStringDecal(olc::vi2d(128, 176), "Consume:");
			DrawStringDecal(olc::vi2d(200, 176), std::to_string(player->shootModule->weapons[w]->consumption));
			FillRectDecal(olc::vf2d{ 28.f + (w % 4) * 48, 204.f + (w / 4) * 16 }, olc::vf2d(40, 12), olc::BLUE);
			for (int i = 0; i < 8; i++)
				if (player->shootModule->unlocked[i])
					DrawStringDecal(olc::vf2d{ 32.f + (i % 4) * 48, 208.f + (i / 4) * 16 }, weaponString[i]);

			if (GetKey(olc::ENTER).bPressed) gameState = IN_GAME;

			break;
		}
		case ENDING:
		{
			DrawStringDecal(olc::vi2d(48,  48), "  CONGRATULATIONS!");
			Drawer::get().DrawPolygonFilled(endingBall->model->transformedModel, olc::WHITE);
			DrawStringDecal(olc::vi2d(48, 176), "YOU ARE NOW A BALL!");

			break;
		}
		}

		return true;
	}
};

int main()
{
	//::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	Game game;
	if (game.Construct(256, 256, 4, 4))
		game.Start();
	return 0;
}