#pragma once

#include "olcPixelGameEngine.h"
#include "global.h"
#include "drawer.h"

enum BodyType { BODY, PROJECTILE, SHIELD, ITEM };

enum DrawType { FILLED, OUTLINED, NOTDRAWN };

enum WeaponType { WP_POLY, WP_SHIELD, WP_BLOCK, WP_DISCHARGE, WP_PROJECTILE, WP_MULTI, WP_BALL, WP_WAVE, WP_BOMB, WP_HOMING, WP_LASER };

struct Behavior;

struct WireFrame;

struct Module
{
	virtual void Update(float elapsed) = 0;
};

struct Particle
{
	olc::vf2d origin;
	float radius = 0.1f;
	float timer = 0.1f;

	void Init(olc::vf2d o);
	void Update(float elapsed);
	void Draw();
};

struct ResizeModule : Module
{
	WireFrame* target = nullptr;
	float originalSize = 0.f;
	float resizeSpeed = 0.f;
	float resizeTimer = 0.f;
	float resizeDuration = 0.f;
	int resizeEpoch = 0;
	bool keepActive = false;

	ResizeModule(WireFrame* t);

	void Initiate(float spd, float d, int r = 1, bool act = false);
	void Update(float elapsed) override;
};

struct FlickerModule : Module
{
	DrawType* target = nullptr;
	DrawType defType = FILLED;
	bool flicker = false;
	float flickerTimer = 0.f;
	float flickerDuration = 0.f;

	//FlickerModule();
	void Initiate(float d);
	void Update(float elapsed) override;
};

struct FlashModule : Module
{
	olc::Pixel* target = nullptr;
	olc::Pixel flashColor = olc::RED;
	olc::Pixel defColor = olc::WHITE;
	bool flash = false;
	float flashTimer = 0.f;
	float flashDuration = 0.f;

	FlashModule(olc::Pixel* t, olc::Pixel c);
	void Initiate(float d);
	void Update(float elapsed) override;
};

struct FrameTrace
{
	WireFrame* parent;
	olc::vf2d origin = { 128.f, 120.f };
	std::vector<olc::vf2d> transformedModel;
	float activeTimer = 0.f;
	float traceTimer = 0.f;

	FrameTrace(WireFrame* p);
	void Update(float elapsed);
};

struct WireFrame
{
	Object* parent = nullptr;
	olc::vf2d origin = { 0.f, 0.f };
	std::vector<olc::vf2d> originalModel;
	std::vector<olc::vf2d> transformedModel;
	FrameTrace* trace = nullptr;
	float size = 8.f;
	DrawType drawType = DrawType::FILLED;
	olc::Pixel color = olc::WHITE;

	WireFrame(Object* p, int v, float s);
	~WireFrame();
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
	std::vector<Weapon*> pool;
	int consumption = 1;
	float rateOfFire = 0.33f;
	float shootTimer = 0.f;
	bool upgradable = false;

	Arsenal(WireFrame& model, WeaponType t, float rof, bool friendly);
	~Arsenal();

	void Update(float elapsed);
};

struct ShootModule : Module
{
	Object* parent;
	int weaponIndex = 0;
	bool unlocked[8];
	std::vector<Arsenal*> weapons;

	ShootModule(Object* p);
	~ShootModule();
	void AddArsenal(WeaponType type, float rof, bool friendly);
	void NextWeapon();
	void PreviousWeapon();
	void Shoot(int w);
	void Update(float elapsed) override;
};

struct Object
{
	olc::vf2d origin = { 0.f, 0.f };
	olc::vf2d velocity = { 0.f, 0.f };
	float angle = -PI / 2;
	WireFrame* model = nullptr;
	WireFrame* head = nullptr;

	EnergyModule* life = nullptr;
	EnergyModule* energy = nullptr;
	ItemModule* item = nullptr;
	ShootModule* shootModule = nullptr;
	BodyType bodyType = BodyType::BODY;
	bool friendly = true;
	bool solid = true;
	bool isPlayer = false;
	bool wpnPoly = false;

	Behavior* behavior = nullptr;
	ResizeModule* resizer = nullptr;
	FlashModule* flashModule = nullptr;
	std::vector<Module*> modules;

	float maxSpeed = 128.f;
	float speed = 64.f;

	bool active = true;
	bool added = false;

	Object(olc::vi2d o, int v, float s);
	~Object();
	void AddBehavior(Behavior* b);
	virtual int Damage();
	virtual void Collided(bool blocked = false);
	virtual void Update(float elapsed);
	virtual void Draw();
};

struct Weapon : Object
{
	int damage = 1;
	bool continuous = false;
	bool clearProjectiles = false;

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