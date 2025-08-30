#include "physics.h"

bool PhysicsEngine::ShapeOverlap_DIAGS(WireFrame& r1, WireFrame& r2)
{
	WireFrame* poly1 = &r1;
	WireFrame* poly2 = &r2;

	for (int shape = 0; shape < 2; shape++)
	{
		if (shape == 1)
		{
			poly1 = &r2;
			poly2 = &r1;
		}

		// Check diagonals of polygon...
		for (int p = 0; p < poly1->transformedModel.size(); p++)
		{
			olc::vf2d line_r1s = poly1->origin;
			olc::vf2d line_r1e = poly1->transformedModel[p];

			// ...against edges of the other
			for (int q = 0; q < poly2->transformedModel.size(); q++)
			{
				olc::vf2d line_r2s = poly2->transformedModel[q];
				olc::vf2d line_r2e = poly2->transformedModel[(q + 1) % poly2->transformedModel.size()];

				// Standard "off the shelf" line segment intersection
				float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
				float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
				float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

				if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool PhysicsEngine::ShapeOverlap_DIAGS_STATIC(WireFrame& r1, WireFrame& r2)
{
	//handle velocity properly when collision occurs! Bounce from tiles!
	WireFrame* poly1 = &r1;
	WireFrame* poly2 = &r2;

	for (int shape = 0; shape < 2; shape++)
	{
		if (shape == 1)
		{
			poly1 = &r2;
			poly2 = &r1;
		}

		// Check diagonals of this polygon...
		for (int p = 0; p < poly1->transformedModel.size(); p++)
		{
			olc::vf2d line_r1s = poly1->origin;
			olc::vf2d line_r1e = poly1->transformedModel[p];

			olc::vf2d displacement = { 0,0 };

			// ...against edges of this polygon
			for (int q = 0; q < poly2->transformedModel.size(); q++)
			{
				olc::vf2d line_r2s = poly2->transformedModel[q];
				olc::vf2d line_r2e = poly2->transformedModel[(q + 1) % poly2->transformedModel.size()];

				// Standard "off the shelf" line segment intersection
				float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
				float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
				float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

				if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
				{
					displacement.x += (1.0f - t1) * (line_r1e.x - line_r1s.x);
					displacement.y += (1.0f - t1) * (line_r1e.y - line_r1s.y);
				}
			}

			r1.parent->origin.x += displacement.x * (shape == 0 ? -1 : +1);
			r1.parent->origin.y += displacement.y * (shape == 0 ? -1 : +1);

			olc::vf2d collisionVec = r1.origin - r2.origin;
			if (r1.parent->friendly) {
				r1.parent->velocity = -collisionVec * 16.f;
			}
			else {
				r2.parent->velocity = collisionVec * 16.f;
			}
		}
	}

	// Cant overlap if static collision is resolved
	return false;
}

bool PhysicsEngine::CircleVsTile(WireFrame& r1, Screen& screen, std::vector<Object*>& itemPool)
{
	olc::vf2d start, end;
	for (int i = 0; i < GRID_SIZE; i++) {
		if (screen.tiles[i] > 0) {
			start = olc::vf2d((i % 16) * 16, (i / 16) * 16);
			end = start + olc::vf2d(16, 16);
			olc::vf2d nearest = olc::vf2d(std::max(start.x, std::min(end.x, r1.origin.x)), std::max(start.y, std::min(end.y, r1.origin.y)));
			nearest -= r1.origin;
			if (nearest.mag2() < r1.size * r1.size) {
				float overlap = r1.size - nearest.mag();
				r1.parent->origin = r1.parent->origin - nearest.norm() * overlap;
				r1.parent->velocity -= nearest.norm() * r1.parent->maxSpeed / 5;
				r1.parent->Collided();
				bool dropItem = false;
				if (r1.parent->bodyType == PROJECTILE) {
					if (screen.tiles[i] == 2) {
						screen.tiles[i] = 0;
						if (screen.items[i] > 0) dropItem = true;
					}
					else {
						if (screen.tiles[i] > 2 && r1.parent->wpnPoly) {
							if (r1.originalModel.size() >= screen.tiles[i]) {
								screen.tiles[i] = 0;
								if (screen.items[i] > 0) dropItem = true;
							}
						}
					}
					if (dropItem) {
						olc::vf2d pos = olc::vf2d((i % 16) * 16 + 8, (i / 16) * 16 + 8);
						int poolIndex = -1;
						for (int j = 0; j < itemPool.size(); j++)
							if (!itemPool[j]->active) {
								poolIndex = j;
								break;
							}
						if (poolIndex > -1) {
							GLBL::get().added.push_back(UnitBuilder::BuildItem(pos, screen.items[i], itemPool[poolIndex]));
							itemPool[poolIndex]->active = true;
						}
					}
				}
			}
		}
	}
	return false;
}