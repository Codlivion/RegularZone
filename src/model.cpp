#include "model.h"


ResizeModule::ResizeModule(WireFrame* t)
{
	target = t;
}

void ResizeModule::Initiate(float spd, float d, int r, bool act)
{
	if (target == nullptr) return;

	if (resizeEpoch <= 0) {
		resizeSpeed = spd;
		resizeDuration = d;
		resizeTimer = resizeDuration;
		resizeEpoch = r;
		keepActive = act;
		originalSize = target->size;
	}
}

void ResizeModule::Update(float elapsed)
{
	if (target == nullptr) return;

	if (resizeEpoch > 0) {
		if (resizeTimer > 0.f) {
			target->size += resizeSpeed * elapsed;
			target->UpdateModel(target->originalModel.size());
			resizeTimer -= elapsed;
			if (resizeTimer <= 0.f) {
				resizeEpoch--;
				if (resizeEpoch <= 0) {
					if (!keepActive) target->size = originalSize;
					target->parent->active = keepActive;
				}
				else {
					target->size = originalSize;
					resizeTimer = resizeDuration;
				}
			}
		}
	}
}

void FlickerModule::Initiate(float d)
{
	flicker = true;
	flickerTimer = 0.f;
	flickerDuration = d;
}

void FlickerModule::Update(float elapsed)
{
	if (target == nullptr) return;

	if (flickerDuration > 0.f) {
		flickerDuration -= elapsed;
		if (flickerDuration <= 0.f) {
			*target = defType;
		}
		else {
			flickerTimer -= elapsed;
			if (flickerTimer <= 0.f) {
				flickerTimer += 0.1f;
				flicker = !flicker;
				*target = flicker ? NOTDRAWN : defType;
			}
		}
	}
}

FlashModule::FlashModule(olc::Pixel* t, olc::Pixel c)
{
	target = t;
	defColor = *t;
	flashColor = c;
}

void FlashModule::Initiate(float d)
{
	flash = true;
	flashTimer = 0.f;
	flashDuration = d;
}

void FlashModule::Update(float elapsed)
{
	if (target == nullptr) return;

	if (flashDuration > 0.f) {
		flashDuration -= elapsed;
		if (flashDuration <= 0.f) {
			*target = defColor;
		}
		else {
			flashTimer -= elapsed;
			if (flashTimer <= 0.f) {
				flashTimer += 0.1f;
				flash = !flash;
				*target = flash ? flashColor : defColor;
			}
		}
	}
}

FrameTrace::FrameTrace(WireFrame* p)
{
	parent = p;
	transformedModel.resize(parent->originalModel.size());
}

void FrameTrace::Update(float elapsed)
{
	if (parent == nullptr) return;

	if (activeTimer > 0.f) {
		traceTimer -= elapsed;
		if (traceTimer <= 0.f) {
			traceTimer = 0.05f;
			origin = parent->origin;
		}
		for (int i = 0; i < parent->originalModel.size(); i++)
			transformedModel[i] =
		{
			(parent->originalModel[i].x * cosf(parent->parent->angle)) - (parent->originalModel[i].y * sinf(parent->parent->angle)) + origin.x,
			(parent->originalModel[i].x * sinf(parent->parent->angle)) + (parent->originalModel[i].y * cosf(parent->parent->angle)) + origin.y,
		};
		Drawer::get().DrawPolygon(transformedModel, parent->color);
		activeTimer -= elapsed;
		if (activeTimer <= 0.f) activeTimer = 0.f;
	}
}

WireFrame::WireFrame(Object* p, int v, float s)
{
	parent = p;
	origin = parent->origin;
	int n = std::max(v, 3);
	size = s;
	UpdateModel(n);
}

void WireFrame::AddVertex(int i) //Only needed for player?
{
	int n = std::min((int)originalModel.size() + i, 8);
	UpdateModel(n);
	if (parent->life != nullptr) parent->life->ModifyMax(n * 3);
}

void WireFrame::RemoveVertex(int i) //Only needed for player?
{
	int n = std::max((int)originalModel.size() - i, 3);
	UpdateModel(n);
	if (parent->life != nullptr) parent->life->ModifyMax(n * 3);
}


void WireFrame::UpdateModel(int n)
{
	originalModel.clear();
	transformedModel.clear();
	for (int i = 0; i < n; i++)
		originalModel.push_back(olc::vf2d(size * cos(2 * PI * i / n), size * sin(2 * PI * i / n)));
	std::copy(originalModel.begin(), originalModel.end(), back_inserter(transformedModel));
	if (trace != nullptr) trace->transformedModel.resize(n);
	if (parent->shootModule != nullptr) {
		for (auto& a : parent->shootModule->weapons) {
			if (a->type < 5) {
				for (auto& w : a->pool) {
					w->model->UpdateModel(n);
				}
			}
		}
	}
}

void WireFrame::TransformModel(olc::vf2d o)
{
	origin = o;
	for (int i = 0; i < originalModel.size(); i++)
		transformedModel[i] =
	{
		(originalModel[i].x * cosf(parent->angle)) - (originalModel[i].y * sinf(parent->angle)) + origin.x,
		(originalModel[i].x * sinf(parent->angle)) + (originalModel[i].y * cosf(parent->angle)) + origin.y,
	};
}

void WireFrame::Draw()
{
	if (drawType == DrawType::FILLED) Drawer::get().DrawPolygonFilled(transformedModel, color);
	else if (drawType == DrawType::OUTLINED) Drawer::get().DrawPolygon(transformedModel, color);
}

EnergyModule::EnergyModule(int n)
{
	value = n;
	maxValue = n;
}

void EnergyModule::ModifyMax(int m)
{
	value = m;
	maxValue = m;
}

void EnergyModule::Restore(int n, float t)
{
	if (timer > 0.f) return;
	value = std::max(value + n, maxValue);
	timer = t;
}

bool EnergyModule::Consume(int n, float t)
{
	if (timer > 0.f) return true;
	value = std::max(value - n, 0);
	if (value <= 0) return false;
	timer = t;
	return true;
}

void EnergyModule::Update(float elapsed)
{
	if (timer > 0.f) timer -= elapsed;
}

Object::Object(olc::vi2d o, int v, float s)
{
	origin = o;
	model = new WireFrame(this, v, s);
}

void Object::AddBehavior(Behavior* b)
{
	if (behavior != nullptr) delete behavior;
	behavior = b;
}

void Object::Collided(bool blocked) {}

int Object::Damage() { return 1; }

void Object::Update(float elapsed)
{
	if (behavior != nullptr) {
		behavior->Update(elapsed);
	}

	//Clamp Velocity:
	if (bodyType != PROJECTILE) {
		if (velocity.x * velocity.x + velocity.y * velocity.y > maxSpeed * maxSpeed) {
			float a = atan2(velocity.y, velocity.x);
			velocity = { cosf(a) * maxSpeed, sinf(a) * maxSpeed };
		}
	}
	origin += velocity * elapsed;

	if (life != nullptr) life->Update(elapsed);
	if (energy != nullptr) energy->Update(elapsed);
	for (auto& m : modules) m->Update(elapsed);

	if (model->trace != nullptr) model->trace->Update(elapsed);
	if (model != nullptr) model->TransformModel(origin);

	if (head != nullptr) {
		olc::vf2d o = {
			origin.x + ((model->originalModel[0].x * cosf(angle)) - (model->originalModel[0].y * sinf(angle))) / 3,
			origin.y + ((model->originalModel[0].x * sinf(angle)) + (model->originalModel[0].y * cosf(angle))) / 3
		};
		head->TransformModel(o);
	}

	//Handle Object vs Screen Bounds
	if (bodyType == BODY) {
		if (origin.x < 0) {
			origin.x += SCREEN_W;
			if (isPlayer) {
				GLBL::get().screenTransition = -1;
			}
		}
		else if (origin.x > SCREEN_W) {
			origin.x -= SCREEN_W;
			if (isPlayer) {
				GLBL::get().screenTransition = 1;
			}
		}
		if (origin.y < 0) {
			origin.y += SCREEN_H;
			if (isPlayer) {
				GLBL::get().screenTransition = -16;
			}
		}
		else if (origin.y > SCREEN_H) {
			origin.y -= SCREEN_H;
			if (isPlayer) {
				GLBL::get().screenTransition = +16;
			}
		}
	}
	else if (bodyType == PROJECTILE) {
		if (origin.x < 0 || origin.x > 256 || origin.y < 0 || origin.y > 224) active = false;
	}
}

void Object::Draw()
{
	if (model != nullptr) model->Draw();
	if (head != nullptr) head->Draw();
	if (item != nullptr) item->Draw();
}

ItemModule::ItemModule(Object* p, int c)
{
	parent = p;
	type = c;
	active = false;
}

void ItemModule::Draw()
{
	Drawer::get().DrawChar(parent->origin - olc::vf2d(3, 3), letter);
}

Weapon::Weapon(olc::vi2d o, int v, float s) : Object(o, v, s) {}

int Weapon::Damage() { return damage; }

void Weapon::Collided(bool blocked)
{
	if (!continuous || blocked) active = false;
}

Arsenal::Arsenal(WireFrame& model, WeaponType t, float rof, bool friendly)
{
	type = t;
	switch (type)
	{
	case WP_POLY:
		for (int i = 0; i < 5; i++) pool.push_back(UnitBuilder::BuildProjectile(friendly, model.originalModel.size()));
		for (auto& w : pool) w->wpnPoly = true;
		break;
	case WP_SHIELD:
		pool.push_back(UnitBuilder::BuildShield(friendly, 3, model.size + 4));
		break;
	case WP_BLOCK:
		pool.push_back(UnitBuilder::BuildBlock(friendly, 3, model.size));
		break;
	case WP_MULTI:
		for (int i = 0; i < 8; i++) pool.push_back(UnitBuilder::BuildProjectile(friendly, model.originalModel.size()));
		consumption = 3;
		break;
	case WP_BALL:
		pool.push_back(UnitBuilder::BuildBall());
		consumption = 2;
		break;
	case WP_WAVE:
		pool.push_back(UnitBuilder::BuildWave());
		consumption = 2;
		break;
	case WP_BOMB:
		pool.push_back(UnitBuilder::BuildBomb());
		consumption = 3;
		break;
	case WP_HOMING:
		for (int i = 0; i < 3; i++) pool.push_back(UnitBuilder::BuildHoming());
		consumption = 1;
		break;
	case WP_LASER:
		pool.push_back(UnitBuilder::BuildLaser());
		consumption = 1;
		break;
	case WP_DISCHARGE:
		pool.push_back(UnitBuilder::BuildDischarge(friendly, model.originalModel.size(), model.size));
		consumption = 45;
		break;
	case WP_PROJECTILE:
		pool.push_back(UnitBuilder::BuildProjectile(friendly, model.originalModel.size()));
		break;
	default: break;
	}
	rateOfFire = rof;
}

void Arsenal::Update(float elapsed)
{
	if (shootTimer > 0.f) shootTimer -= elapsed;
}

ShootModule::ShootModule(Object* p)
{
	parent = p;
	unlocked[0] = true;
	for (int i = 1; i < 8; i++) unlocked[i] = false;
}

void ShootModule::AddArsenal(WeaponType type, float rof, bool friendly)
{
	weapons.push_back(new Arsenal(*parent->model, type, rof, friendly));
}

void ShootModule::NextWeapon()
{
	int idx = weaponIndex;
	for (int i = 0; i < 7; i++) {
		idx = (idx + 1) % 8;
		if (unlocked[idx]) {
			weaponIndex = idx;
			break;
		}
	}
}

void ShootModule::PreviousWeapon()
{
	int idx = weaponIndex;
	for (int i = 0; i < 7; i++) {
		idx = (idx + 7) % 8;
		if (unlocked[idx]) {
			weaponIndex = idx;
			break;
		}
	}
}

void ShootModule::Shoot(int w)
{
	if (w > 100 && w < 105) {
		FrameTrace* trace = parent->model->trace;
		if (trace != nullptr && trace->activeTimer <= 0.f) {
			trace->activeTimer = 1.f;
			trace->origin = parent->origin;
			float a;
			if (w < 103) a = w == 101 ? parent->angle : parent->angle + PI;
			else a = w == 103 ? parent->angle - PI / 2 : parent->angle + PI / 2;
			parent->velocity = { cosf(a) * parent->speed * 2, sinf(a) * parent->speed * 2 };
		}
	}

	if (w < 0 || w >= weapons.size()) return;

	Arsenal* arsenal = weapons[w];
	WeaponType t = arsenal->type;

	switch (t)
	{
	case WP_SHIELD:
	{
		arsenal->pool[0]->added = true;
		arsenal->pool[0]->life->value += 1;
		break;
	}
	case WP_BLOCK:
	{
		if (arsenal->shootTimer <= 0.f) {
			arsenal->shootTimer = arsenal->rateOfFire;
			arsenal->pool[0]->origin = parent->origin;
			arsenal->pool[0]->angle = parent->angle;
			arsenal->pool[0]->resizer->Initiate(64, 0.2f);
			arsenal->pool[0]->added = true;
			arsenal->pool[0]->life->value = 1;
		}
		break;
	}
	case WP_POLY: case WP_PROJECTILE:
	{
		if (arsenal->shootTimer <= 0.f) {
			for (int i = 0; i < arsenal->pool.size(); i++) {
				if (!arsenal->pool[i]->active) {
					arsenal->shootTimer = arsenal->rateOfFire;
					arsenal->pool[i]->origin = parent->model->transformedModel[0];
					arsenal->pool[i]->angle = parent->angle;
					float spd = t == WP_POLY ? 80.f : 40.f;
					arsenal->pool[i]->velocity = (parent->model->transformedModel[0] - parent->origin) * spd;
					arsenal->pool[i]->added = true;
					break;
				}
			}
		}
		break;
	}
	case WP_MULTI:
	{
		if (parent->energy->value >= arsenal->consumption && arsenal->shootTimer <= 0.f) {
			std::vector<Object*> available;
			for (int i = 0; i < arsenal->pool.size(); i++) {
				if (!arsenal->pool[i]->active) {
					available.push_back(arsenal->pool[i]);
					if (available.size() >= parent->model->originalModel.size()) break;
				}
			}
			if (available.size() >= parent->model->originalModel.size()) {
				arsenal->shootTimer = arsenal->rateOfFire;
				for (int i = 0; i < available.size(); i++) {
					parent->energy->Consume(arsenal->consumption, 0.f);
					available[i]->origin = parent->model->transformedModel[i];
					available[i]->angle = parent->angle + 2 * PI * i / available.size();
					available[i]->velocity = (parent->model->transformedModel[i] - parent->origin) * 80.f;
					available[i]->added = true;
				}
			}
		}
		break;
	}
	case WP_BALL: {
		if (parent->energy->value >= arsenal->consumption) {
			if (arsenal->shootTimer <= 0.f) {
				if (!arsenal->pool[0]->active) {
					parent->energy->Consume(arsenal->consumption, 0.f);
					arsenal->shootTimer = arsenal->rateOfFire;
					arsenal->pool[0]->origin = parent->model->transformedModel[0];
					arsenal->pool[0]->angle = parent->angle;
					arsenal->pool[0]->velocity = (parent->model->transformedModel[0] - parent->origin) * 80.f;
					arsenal->pool[0]->added = true;
					break;
				}
			}
		}
		break;
	}
	case WP_WAVE: {
		if (parent->energy->value >= arsenal->consumption) {
			if (arsenal->shootTimer <= 0.f) {
				if (!arsenal->pool[0]->active) {
					parent->energy->Consume(arsenal->consumption, 0.f);
					arsenal->shootTimer = arsenal->rateOfFire;
					arsenal->pool[0]->origin = parent->model->transformedModel[0];
					arsenal->pool[0]->angle = parent->angle;
					arsenal->pool[0]->velocity = (parent->model->transformedModel[0] - parent->origin) * 128.f;
					arsenal->pool[0]->model->trace->activeTimer = 1.f;
					arsenal->pool[0]->added = true;
					break;
				}
			}
		}
		break;
	}
	case WP_BOMB: {
		if (parent->energy->value >= arsenal->consumption) {
			if (arsenal->shootTimer <= 0.f) {
				if (!arsenal->pool[0]->active) {
					parent->energy->Consume(arsenal->consumption, 0.f);
					arsenal->shootTimer = arsenal->rateOfFire;
					arsenal->pool[0]->origin = parent->model->transformedModel[0];
					arsenal->pool[0]->angle = parent->angle;
					arsenal->pool[0]->velocity = (parent->model->transformedModel[0] - parent->origin) * 40.f;
					arsenal->pool[0]->model->drawType = FILLED;
					arsenal->pool[0]->behavior->Init(0);
					arsenal->pool[0]->added = true;
					break;
				}
			}

		}
		break;
	}
	case WP_HOMING: {
		if (parent->energy->value >= arsenal->consumption) {
			if (arsenal->shootTimer <= 0.f) {
				for (int i = 0; i < arsenal->pool.size(); i++) {
					if (!arsenal->pool[i]->active) {
						parent->energy->Consume(arsenal->consumption, 0.f);
						arsenal->shootTimer = arsenal->rateOfFire;
						arsenal->pool[i]->origin = parent->model->transformedModel[0];
						arsenal->pool[i]->angle = parent->angle;
						arsenal->pool[i]->velocity = (parent->model->transformedModel[0] - parent->origin) * 40.f;
						arsenal->pool[i]->behavior->Init(0);
						arsenal->pool[i]->added = true;
						break;
					}
				}
			}
		}
		break;
	}
	case WP_LASER: {
		arsenal->pool[0]->behavior->Init(0);
		if (parent->energy->value >= arsenal->consumption) {
			if (arsenal->shootTimer <= 0.f) {
				parent->energy->Consume(arsenal->consumption, 0.f);
				arsenal->shootTimer = arsenal->rateOfFire;
				if (!arsenal->pool[0]->active) arsenal->pool[0]->added = true;
				break;
			}
		}
		break;
	}
	case WP_DISCHARGE: {
		if (parent->energy->value >= arsenal->consumption && arsenal->shootTimer <= 0.f) {
			parent->energy->Consume(arsenal->consumption, 0.f);
			arsenal->shootTimer = arsenal->rateOfFire;
			arsenal->pool[0]->origin = parent->origin;
			arsenal->pool[0]->angle = parent->angle;
			parent->life->ModifyMax(100);
			arsenal->pool[0]->resizer->Initiate(800.f, 0.5f);
			arsenal->pool[0]->added = true;
		}
		break;
	}
	default: break;
	}
}

void ShootModule::Update(float elapsed)
{
	for (auto& a : weapons) a->Update(elapsed);
}

Behavior::Behavior(Object* p)
{
	parent = p;
}

void Behavior::Init(int c) {}

void Behavior::Update(float elapsed) {}

BombBehavior::BombBehavior(Object* p) : Behavior(p) {}

void BombBehavior::Init(int c)
{
	timer = 0.33f;
	parent->model->UpdateModel(parent->model->originalModel.size());
}

void BombBehavior::Update(float elapsed)
{
	if (timer > 0.f) {
		timer -= elapsed;
		if (timer <= 0.f) {
			parent->velocity = olc::vf2d(0, 0);
			parent->model->drawType = OUTLINED;
			parent->resizer->Initiate(108.f, 0.3f, 3);
		}
	}
}

HomingBehavior::HomingBehavior(Object* p) : Behavior(p) {}

void HomingBehavior::Init(int c)
{
	target = nullptr;
	float minDist = 256 * 256;
	for (auto& u : GLBL::get().units) {
		if (u->bodyType == BODY && !u->friendly) {
			float dist = (u->origin - parent->origin).mag2();
			if (dist < minDist) {
				target = u;
				minDist = dist;
			}
		}
	}
}

void HomingBehavior::Update(float elapsed)
{
	if (target != nullptr && target->active) {
		olc::vf2d targetVec = target->origin - parent->origin;
		float targetAngle = atan2(targetVec.y, targetVec.x);
		parent->angle = targetAngle - parent->angle * 2 * elapsed;
		parent->velocity = olc::vf2d(cosf(parent->angle) * 160.f, sinf(parent->angle) * 160.f);
	}
}

LaserBehavior::LaserBehavior(Object* p) : Behavior(p)
{
	parent->model->originalModel[0] += olc::vf2d(128.f, 0.f);
}

void LaserBehavior::Init(int c)
{
	heldDown = true;
}

void LaserBehavior::Update(float elapsed)
{
	parent->origin = GLBL::get().player->model->transformedModel[0];
	parent->angle = GLBL::get().player->angle;
	if (!heldDown) parent->active = false;
	heldDown = false;
}

GooBehavior::GooBehavior(Object* p) : Behavior(p) {}

void GooBehavior::Update(float elapsed)
{
	moveTimer -= elapsed;
	if (moveTimer <= 0) {
		moveTimer = 1.f;
		int rand = std::rand() % 4;
		float a = rand * PI / 2;
		parent->velocity = olc::vf2d(cosf(a) * 16.f, sinf(a) * 16.f);
	}
}

StalkerBehavior::StalkerBehavior(Object* p) : Behavior(p) {}

void StalkerBehavior::Update(float elapsed)
{
	moveTimer -= elapsed;
	if (moveTimer <= 0) {
		moveTimer = 0.2f;
		if (GLBL::get().player->active) {
			olc::vf2d targetVec = GLBL::get().player->origin - parent->origin;
			float targetAngle = atan2(targetVec.y, targetVec.x);
			parent->angle = targetAngle - parent->angle * 2 * elapsed;
			parent->velocity = olc::vf2d(cosf(parent->angle) * 32.f, sinf(parent->angle) * 32.f);
		}
	}
}

SniperBehavior::SniperBehavior(Object* p) : Behavior(p) {}

void SniperBehavior::Update(float elapsed)
{
	moveTimer -= elapsed;
	if (seek) {
		parent->angle -= rotDir * PI * elapsed;
		if (moveTimer <= 0) {
			parent->angle = targetAngle;
			parent->shootModule->Shoot(0);
			moveTimer = 2.f;
			seek = false;
		}
	}
	else {
		if (moveTimer <= 0) {
			if (GLBL::get().player->active) {
				olc::vf2d targetVec = GLBL::get().player->origin - parent->origin;
				targetAngle = atan2(targetVec.y, targetVec.x);
				rotDir = parent->angle > targetAngle ? 1 : -1;
				moveTimer = abs(parent->angle - targetAngle) / PI;
				seek = true;
			}
		}
	}
}

SliderBehavior::SliderBehavior(Object* p) : Behavior(p) {}

void SliderBehavior::Init(int c)
{
	horizontal = c == 0 ? true : false;
	if (horizontal) {
		start = parent->origin.x - 32.f;
		end = parent->origin.x + 32.f;
		targetVel.x = 64.f;
	}
	else {
		start = parent->origin.y - 32.f;
		end = parent->origin.y + 32.f;
		targetVel.y = 64.f;
	}
}

void SliderBehavior::Update(float elapsed)
{
	if (parent->velocity != targetVel) {
		parent->velocity += targetVel * elapsed;
		if ((parent->velocity).dot(targetVel) < 0.1f) parent->velocity = targetVel;
	}
	if (horizontal) {
		if (parent->velocity.x > 0) {
			if (parent->origin.x > end) targetVel.x = -64.f;
		}
		else {
			if (parent->origin.x < start) targetVel.x = 64.f;
		}
	}
	else {
		if (parent->velocity.y > 0) {
			if (parent->origin.y > end) targetVel.y = -64.f;
		}
		else {
			if (parent->origin.y < start) targetVel.y = 64.f;
		}
	}
}

TurretBehavior::TurretBehavior(Object* p) : Behavior(p) {}

void TurretBehavior::Update(float elapsed)
{
	parent->angle += rotDir * PI * elapsed;
	moveTimer -= elapsed;
	if (moveTimer <= 0) {
		parent->shootModule->Shoot(0);
		moveTimer = 0.55f;
	}
}

RoamerBehavior::RoamerBehavior(Object* p) : Behavior(p) {}

void RoamerBehavior::Update(float elapsed)
{
	moveTimer -= elapsed;
	if (moveTimer <= 0) {
		moveTimer = 1.f;
		parent->velocity = { 0.f, 0.f };
		if (parent->origin.x < 48.f) parent->velocity.x = 48.f;
		else if (parent->origin.x > 208.f) parent->velocity.x = -48.f;
		if (parent->origin.y < 48.f) parent->velocity.y = 48.f;
		else if (parent->origin.y > 208.f) parent->velocity.y = -48.f;
		if (parent->velocity.x == 0 && parent->velocity.y == 0) {
			int rand = std::rand() % 8;
			float a = rand * PI / 4;
			parent->velocity = olc::vf2d(cosf(a) * 48.f, sinf(a) * 48.f);
		}
	}
}

Object* UnitBuilder::BuildPlayer(Object* existing)
{
	Object* unit;
	
	if (existing == nullptr) {
		unit = new Object(olc::vf2d(128.f, 112.f), 3, 8);
		unit->head = new WireFrame(unit, unit->model->originalModel.size(), unit->model->size / 2);

		unit->bodyType = BodyType::BODY;
		unit->model->drawType = DrawType::FILLED;
		unit->model->color = olc::WHITE;
		unit->head->color = olc::BLUE;
		unit->isPlayer = true;

		unit->life = new EnergyModule(unit->model->originalModel.size() * 3);
		unit->energy = new EnergyModule(unit->model->originalModel.size() * 30);
		unit->flashModule = new FlashModule(&unit->model->color, olc::RED);
		unit->modules.push_back(unit->flashModule);
		unit->model->trace = new FrameTrace(unit->model);
		unit->shootModule = new ShootModule(unit);
		unit->modules.push_back(unit->shootModule);

		unit->shootModule->AddArsenal(WP_SHIELD, 1.f, true);
		unit->shootModule->AddArsenal(WP_BLOCK, 1.f, true);
		unit->shootModule->AddArsenal(WP_POLY, 0.45f, true);
		unit->shootModule->AddArsenal(WP_MULTI, 0.33f, true);
		unit->shootModule->AddArsenal(WP_BALL, 0.45f, true);
		unit->shootModule->AddArsenal(WP_WAVE, 0.45f, true);
		unit->shootModule->AddArsenal(WP_BOMB, 1.f, true);
		unit->shootModule->AddArsenal(WP_HOMING, 0.25f, true);
		unit->shootModule->AddArsenal(WP_LASER, 0.5f, true);
		unit->shootModule->AddArsenal(WP_DISCHARGE, 3.f, true);
	}
	else {
		unit = existing;
		unit->origin = olc::vf2d(128.f, 112.f);
		unit->velocity = olc::vf2d(0.f, 0.f);
		unit->angle = -PI / 2;
		unit->life->ModifyMax(unit->model->originalModel.size() * 3);
		unit->energy->ModifyMax(unit->model->originalModel.size() * 30);
	}
	unit->active = true;
	return unit;
}

enum EnemyType { GOO, STALKER, SNIPER, SLIDER, TURRET, ROAMER };

Object* UnitBuilder::BuildEnemy(olc::vd2d p, int type, Object* existing)
{
	int v = type + 2;
	float size = type == 1 ? 4 : type == 7 ? 16 : 8;
	Object* unit;
	if (existing == nullptr) {
		unit = new Object(p, v, size);
		unit->head = new WireFrame(unit, v, size / 2);

		unit->bodyType = BodyType::BODY;
		unit->friendly = false;
		unit->model->drawType = DrawType::FILLED;
		unit->model->color = olc::RED;
		unit->head->color = olc::MAGENTA;

		unit->life = new EnergyModule(v - 2);
		unit->flashModule = new FlashModule(&unit->model->color, olc::WHITE);
		unit->modules.push_back(unit->flashModule);
		unit->shootModule = new ShootModule(unit);
		unit->modules.push_back(unit->shootModule);
		unit->shootModule->AddArsenal(WP_PROJECTILE, 0.25f, false);
	}
	else {
		unit = existing;
		unit->origin = p;
		unit->model->size = size;
		unit->model->UpdateModel(v);
		unit->head->size = size / 2;
		unit->head->UpdateModel(v);
		unit->life->ModifyMax(v - 2);

		switch (type)
		{
		case 1: unit->AddBehavior(new GooBehavior(unit));

			break;
		case 2: unit->AddBehavior(new StalkerBehavior(unit));
			break;
		case 3: unit->AddBehavior(new SniperBehavior(unit));
			break;
		case 4: unit->AddBehavior(new SliderBehavior(unit));
			unit->behavior->Init(0);
			break;
		case 5: unit->AddBehavior(new SliderBehavior(unit));
			unit->behavior->Init(1);
			break;
		case 6: unit->AddBehavior(new TurretBehavior(unit));
			break;
		case 7: unit->AddBehavior(new RoamerBehavior(unit));
			break;
		default:
			break;
		}
	}
	return unit;
}

Object* UnitBuilder::BuildItem(olc::vd2d p, int c, Object* existing)
{
	//Object* unit = existing != nullptr ? existing : new Object(p, 4, 8);
	Object* unit;
	if (existing == nullptr) {
		unit = new Object(p, 4, 8);
		unit->bodyType = BodyType::ITEM;
		unit->model->drawType = DrawType::FILLED;
		unit->model->color = olc::WHITE;
		unit->active = false;
	}
	else {
		unit = existing;
		unit->origin = p;
		if (c == 2 || c == 4) unit->model->size = 4;
		else unit->model->size = 8;
		unit->model->UpdateModel(GLBL::get().itemVertices[c]);
		unit->model->color = GLBL::get().itemColors[c];
		if (unit->item != nullptr) delete unit->item;
		unit->item = new ItemModule(unit, c);
		unit->item->letter = GLBL::get().itemLetters[c];
	}
	return unit;
}

Weapon* UnitBuilder::BuildProjectile(bool friendly, int level)
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, level, 4);
	unit->bodyType = BodyType::PROJECTILE;
	unit->friendly = friendly;
	unit->model->drawType = DrawType::FILLED;
	unit->model->color = friendly ? olc::WHITE : olc::RED;
	unit->damage = level - 2;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildBall()
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, 16, 8);
	unit->bodyType = BodyType::PROJECTILE;
	unit->model->drawType = DrawType::FILLED;
	unit->model->color = olc::DARK_CYAN;
	unit->damage = 6;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildWave()
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, 3, 12);
	unit->bodyType = BodyType::PROJECTILE;
	unit->model->drawType = DrawType::OUTLINED;
	unit->model->color = olc::DARK_MAGENTA;
	unit->model->trace = new FrameTrace(unit->model);
	unit->damage = 2;
	unit->solid = false;
	unit->continuous = true;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildBomb()
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, 8, 4);
	unit->bodyType = BodyType::PROJECTILE;
	unit->model->drawType = DrawType::FILLED;
	unit->model->color = olc::DARK_YELLOW;
	unit->resizer = new ResizeModule(unit->model);
	unit->modules.push_back(unit->resizer);
	unit->behavior = new BombBehavior(unit);
	unit->damage = 10;
	unit->solid = false;
	unit->continuous = true;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildHoming()
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, 4, 4);
	unit->bodyType = BodyType::PROJECTILE;
	unit->model->drawType = DrawType::FILLED;
	unit->model->color = olc::DARK_RED;
	unit->behavior = new HomingBehavior(unit);
	unit->damage = 2;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildLaser()
{
	Weapon* unit = new Weapon(olc::vf2d{ 0.f, 0.f }, 4, 4);
	unit->bodyType = BodyType::PROJECTILE;
	unit->model->drawType = DrawType::FILLED;
	unit->model->color = olc::CYAN;
	unit->behavior = new LaserBehavior(unit);
	unit->damage = 1;
	unit->solid = false;
	unit->continuous = true;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildShield(bool friendly, int v, int size)
{
	Weapon* unit = new Weapon(olc::vd2d{ 0.f, 0.f }, v, size + 4);
	unit->bodyType = BodyType::SHIELD;
	unit->friendly = friendly;
	unit->model->drawType = DrawType::OUTLINED;
	unit->model->color = friendly ? olc::WHITE : olc::RED;
	unit->life = new EnergyModule(0);
	unit->solid = false;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildBlock(bool friendly, int v, int size)
{
	Weapon* unit = new Weapon(olc::vd2d{ 0.f, 0.f }, v, size);
	unit->bodyType = BodyType::SHIELD;
	unit->friendly = friendly;
	unit->model->drawType = DrawType::OUTLINED;
	unit->model->color = friendly ? olc::WHITE : olc::RED;
	unit->life = new EnergyModule(0);
	unit->resizer = new ResizeModule(unit->model);
	unit->modules.push_back(unit->resizer);
	unit->solid = false;
	unit->continuous = true;
	unit->active = false;
	return unit;
}

Weapon* UnitBuilder::BuildDischarge(bool friendly, int v, int size)
{
	Weapon* unit = new Weapon(olc::vd2d{ 0.f, 0.f }, v, size);
	unit->bodyType = BodyType::SHIELD;
	unit->friendly = friendly;
	unit->model->drawType = DrawType::FILLED;
	unit->life = new EnergyModule(100);
	unit->model->color = friendly ? olc::WHITE : olc::RED;
	unit->resizer = new ResizeModule(unit->model);
	unit->modules.push_back(unit->resizer);
	unit->damage = v * 10;
	unit->solid = false;
	unit->continuous = true;
	unit->active = false;
	return unit;
}