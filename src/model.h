#pragma once

#include "olcPixelGameEngine.h"
#include "global.h"
#include "drawer.h"

enum BodyType { BODY, PROJECTILE, SHIELD, ITEM };

enum DrawType { FILLED, OUTLINED, NOTDRAWN };

enum WeaponType { WP_POLY, WP_SHIELD, WP_BLOCK, WP_DISCHARGE, WP_PROJECTILE, WP_MULTI, WP_BALL, WP_WAVE, WP_BOMB, WP_HOMING, WP_LASER };

struct Behavior;

struct WireFrame;

//Write Destructors for the Objects!
//Add hit effect (Flash)

struct Module
{
	virtual void Update(float elapsed) = 0;
};

struct ResizeModule : Module
{
	WireFrame* target = nullptr; //Target to manipulate - Shared - Make sure target is set when adding!
	float originalSize = 0.f; //original size of the model
	float resizeSpeed = 0.f; //Speed at which the size grows
	float resizeTimer = 0.f; //timer for each epoch
	float resizeDuration = 0.f; //Duration of the resize event
	int resizeEpoch = 0; //How many times the resize is repeated
	bool keepActive = false;

	ResizeModule(WireFrame* t);

	void Initiate(float spd, float d, int r = 1, bool act = false);
	void Update(float elapsed) override;
};

struct FlickerModule : Module
{
	DrawType* target = nullptr; //Target to manipulate - Shared Pointer
	DrawType defType = FILLED; //default type for the target
	bool flicker = false; //On-Off switch for drawing or not
	float flickerTimer = 0.f; //Timer for each tick
	float flickerDuration = 0.f; //Duration of the flicker event

	//FlickerModule();
	void Initiate(float d);
	void Update(float elapsed) override;
};

struct FlashModule : Module
{
	olc::Pixel* target = nullptr; //Target to manipulate - Shared Pointer
	olc::Pixel flashColor = olc::RED; //color of the flash
	olc::Pixel defColor = olc::WHITE; //default color for the target
	bool flash = false; //On-Off switch for swapping colors
	float flashTimer = 0.f; //Timer for each tick
	float flashDuration = 0.f; //Duration of the flash event

	FlashModule(olc::Pixel* t, olc::Pixel c);
	void Initiate(float d);
	void Update(float elapsed) override;
};

struct FrameTrace
{
	WireFrame* parent; //Shared - parent model!
	olc::vf2d origin = { 128.f, 120.f }; //position of the trace
	std::vector<olc::vf2d> transformedModel; //Trace uses the same model but transformed at its own origin
	float activeTimer = 0.f; //How long it lasts
	float traceTimer = 0.f; //origin is updated at each tick

	FrameTrace(WireFrame* p);
	void Update(float elapsed);
};

//struct Object;

struct WireFrame
{
	Object* parent = nullptr; //Shared - parent object!
	olc::vf2d origin = { 0.f, 0.f }; //Model has its own origin seperate from its parent?
	std::vector<olc::vf2d> originalModel; //Model of the WireFrame - Can change with events
	std::vector<olc::vf2d> transformedModel; //Model Transformed to the Screen - Updated every frame
	FrameTrace* trace = nullptr; //Frame may or may not have a trace - Destruct!
	float size = 8.f; //Size of the Model
	DrawType drawType = DrawType::FILLED; //Draw Type of the Shape
	olc::Pixel color = olc::WHITE; //color at which to draw the shape

	WireFrame(Object* p, int v, float s);
	void AddVertex(int i);
	void RemoveVertex(int i);

	void UpdateModel(int n);
	void TransformModel(olc::vf2d o);

	void Draw();
};

struct EnergyModule
{
	int value = 1;
	int maxValue = 1;
	float timer = 0.f;

	EnergyModule(int n);

	void ModifyMax(int m);
	void Restore(int n, float t);
	bool Consume(int n, float t);

	void Update(float elapsed);
};

struct ItemModule
{
	Object* parent = nullptr;
	int type = 0;
	std::string letter = "";
	bool active = false;

	ItemModule(Object* p, int c);
	void Draw();
};

struct Weapon;

struct Arsenal
{
	WeaponType type;
	std::vector<Weapon*> pool; //Pool of projectiles - Destruct!
	int consumption = 1;
	float rateOfFire = 0.33f; //Rate at which the shoot event can be recalled
	float shootTimer = 0.f; //Timer since the last shoot event

	Arsenal(WireFrame& model, WeaponType t, float rof, bool friendly);

	void Update(float elapsed);
};

struct ShootModule : Module
{
	Object* parent;
	int weaponIndex = 0;
	bool unlocked[8];
	std::vector<Arsenal*> weapons; //weapons - Destruct!

	ShootModule(Object* p);
	void AddArsenal(WeaponType type, float rof, bool friendly);
	void NextWeapon();
	void PreviousWeapon();
	void Shoot(int w);
	void Update(float elapsed) override;
};

struct Object
{
	olc::vf2d origin = { 0.f, 0.f }; //Every object has a position?
	olc::vf2d velocity = { 0.f, 0.f }; //Every object has a velocity?
	float angle = -PI / 2; //Angle of the Shape
	WireFrame* model = nullptr; //Objects may or may not have a Shape - Destruct!
	WireFrame* head = nullptr; //Objects may or may not have a direction indicator - Destruct!

	EnergyModule* life = nullptr;
	EnergyModule* energy = nullptr;
	ItemModule* item = nullptr;
	ShootModule* shootModule = nullptr;
	BodyType bodyType = BodyType::BODY;
	bool friendly = true;
	bool solid = true;
	bool isPlayer = false;
	bool wpnPoly = false;

	Behavior* behavior = nullptr; //Behavior for the Object - Destruct!
	ResizeModule* resizer = nullptr;
	FlashModule* flashModule = nullptr;
	std::vector<Module*> modules; //OOP list of modules - Shared!

	float maxSpeed = 128.f; //For terminal speed
	float speed = 64.f; //For moving objects

	bool active = true; //Active control
	bool added = false; //For adding pooled objects into the game

	Object(olc::vi2d o, int v, float s);
	void AddBehavior(Behavior* b);
	virtual int Damage();
	virtual void Collided(bool blocked = false);
	virtual void Update(float elapsed); //Main functionality - OOP base for spreading Update!
	virtual void Draw(); //OOP call drawing routine for this object
};

struct Weapon : Object
{
	int damage = 1; //can be both damage and hp? (or seperate durability and damage)
	bool continuous = false; //continuous flag for the projectiles
	bool clearProjectiles = false; //for discharge to detect projectile vs projectile

	Weapon(olc::vi2d o, int v, float s);
	int Damage() override;
	void Collided(bool blocked = false) override;
};

struct Behavior
{
	Object* parent = nullptr;

	Behavior(Object* p);
	virtual void Init(int c);
	virtual void Update(float elapsed);
};

struct BombBehavior : Behavior
{
	float timer = 0.33f;

	BombBehavior(Object* p);
	void Init(int c) override;
	void Update(float elapsed) override;
};

struct HomingBehavior : Behavior
{
	Object* target = nullptr;

	HomingBehavior(Object* p);
	void Init(int c) override;
	void Update(float elapsed) override;
};

struct LaserBehavior : Behavior
{
	bool heldDown = false;

	LaserBehavior(Object* p);
	void Init(int c) override;
	void Update(float elapsed) override;
};

struct GooBehavior : Behavior
{
	float moveTimer = 0.f;

	GooBehavior(Object* p);

	void Update(float elapsed) override;
};

struct StalkerBehavior : Behavior
{
	float moveTimer = 0.f;

	StalkerBehavior(Object* p);

	void Update(float elapsed) override;
};

struct SniperBehavior : Behavior
{
	float moveTimer = 0.f;
	float targetAngle = 0.f;
	int rotDir = 1;
	bool seek = false;

	SniperBehavior(Object* p);

	void Update(float elapsed) override;
};

struct SliderBehavior : Behavior
{
	olc::vf2d targetVel = {0, 0};
	bool horizontal = true;
	float start = 0.f;
	float end = 0.f;

	SliderBehavior(Object* p);
	void Init(int c) override;
	void Update(float elapsed) override;
};

struct TurretBehavior : Behavior
{
	float moveTimer = 0.f;
	int rotDir = 1;

	TurretBehavior(Object* p);
	void Update(float elapsed) override;
};

struct RoamerBehavior : Behavior
{
	float moveTimer = 0.f;

	RoamerBehavior(Object* p);

	void Update(float elapsed) override;
};

class UnitBuilder
{
public:
	static Object* BuildPlayer(Object* existing = nullptr);
	static Object* BuildEnemy(olc::vd2d p, int type, Object* existing = nullptr);
	static Object* BuildItem(olc::vd2d p, int c, Object* existing = nullptr);
	static Weapon* BuildProjectile(bool friendly, int level);
	static Weapon* BuildBall();
	static Weapon* BuildWave();
	static Weapon* BuildBomb();
	static Weapon* BuildHoming();
	static Weapon* BuildLaser();
	static Weapon* BuildShield(bool friendly, int v, int size);
	static Weapon* BuildBlock(bool friendly, int v, int size);
	static Weapon* BuildDischarge(bool friendly, int v, int size);
};

/*
Items:
	HP (Small/Large)
	SP (Small/Large)
	Shield (Capped at Vertices)
Upgrades:
	ROF (1: 0.3 - 2: 0.25 - 3: 0.2 - 4: 0.15 - 5: 0.1)
Weapons?:
	Multi Shot: Shoot Around Regular Shots From All Vertices
	Big Shot: Shoot Large Circles (Decagon?) That Does Decent Damage
	Wave Shot: Shoot A Fast Traced Triangle That Goes Through Everything (Damaging Once!)
	Explosive Shot: Shoot An Octagon That Explodes Into A Discharge At A Certain Distance
	Homing Shot: Shoot Up To 3 Rhombuses That Gravitate Towards The Nearest Enemy
	LaserShot: Shot An Instant Line Towards The Edge Of Screen That Damages Anything Over Time
	Discharge: Release A Discharge That Damage/Kill All In Screen (SP or Count?)
*/