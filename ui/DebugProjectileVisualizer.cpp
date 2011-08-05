
#include "precompiled.h"

#include "DebugProjectileVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../game/ProjectileList.h"
#include "../game/Projectile.h"
#include "../game/GameRandom.h"
#include "../game/options/options_debug.h"
#include "../system/Timer.h"

#include <vector>
#include <Storm3D_UI.h>

#define DEBUGPROJECTILEVISUALIZER_EXTENDED_MAX_DIST 100.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugProjectileVisualizer::visualizeProjectiles(game::ProjectileList *projectiles, const VC3 &cameraPosition)
	{
		assert(projectiles != NULL);

		char textbuf[128];

		LinkedList *projectileList = projectiles->getAllProjectiles();
		LinkedListIterator iter(projectileList);

		int foo2 = Timer::getTime();
		GameRandom foo;
		foo.seed((foo2 / 500));

		//int index = 0;
		while (iter.iterateAvailable())
		{
			Projectile *projectile = (Projectile *)iter.iterateNext();

			COL col = COL(1,0.3f,0);

			VC3 pos = projectile->getPosition();
			VC3 sizes = VC3(0.08f, 0.08f, 0.08f);

			VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
			VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
			VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
			VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
			VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
			VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
			VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
			VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

			//float extra_offset = 0.0f + (index / 200.0f);
			float extra_offset_x = (foo.nextInt() % 101) / 100.0f - 0.5f;
			float extra_offset_y = (foo.nextInt() % 101) / 100.0f - 0.5f;
			float extra_offset_z = (foo.nextInt() % 101) / 100.0f - 0.5f;
			float extra_size = 0.05f;
			VC3 cb4_extra = VC3(pos.x - sizes.x + extra_offset_x, pos.y + extra_offset_y, pos.z + extra_offset_z - extra_size);
			VC3 cb4_extra2 = VC3(pos.x - sizes.x + extra_offset_x, pos.y + extra_offset_y, pos.z + extra_offset_z + extra_size);
			VC3 cb4_extra3 = VC3(pos.x - sizes.x + extra_offset_x, pos.y + extra_offset_y - extra_size, pos.z + extra_offset_z);
			VC3 cb4_extra4 = VC3(pos.x - sizes.x + extra_offset_x, pos.y + extra_offset_y + extra_size, pos.z + extra_offset_z);
			VC3 cb4_extra5 = VC3(pos.x - sizes.x + extra_offset_x - extra_size, pos.y + extra_offset_y, pos.z + extra_offset_z);
			VC3 cb4_extra6 = VC3(pos.x - sizes.x + extra_offset_x + extra_size, pos.y + extra_offset_y, pos.z + extra_offset_z);

			disposable_scene->AddLine(c1, c2, col);
			disposable_scene->AddLine(c2, c3, col);
			disposable_scene->AddLine(c3, c4, col);
			disposable_scene->AddLine(c4, c1, col);
			disposable_scene->AddLine(cb1, cb2, col);
			disposable_scene->AddLine(cb2, cb3, col);
			disposable_scene->AddLine(cb3, cb4, col);
			disposable_scene->AddLine(cb4, cb1, col);
			disposable_scene->AddLine(c1, cb1, col);
			disposable_scene->AddLine(c2, cb2, col);
			disposable_scene->AddLine(c3, cb3, col);
			disposable_scene->AddLine(c4, cb4, col);
			disposable_scene->AddLine(c4, cb4, col);

			COL col2 = COL(1,1,0);
			disposable_scene->AddLine(cb4_extra, cb4_extra2, col2);
			disposable_scene->AddLine(cb4_extra3, cb4_extra4, col2);
			disposable_scene->AddLine(cb4_extra5, cb4_extra6, col2);

			VC3 distToCamVec = pos - cameraPosition;
			float distToCamSq = distToCamVec.GetSquareLength();

			int textoffy = 0;

			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_PROJECTILES_EXTENDED)
				&& distToCamSq < (DEBUGPROJECTILEVISUALIZER_EXTENDED_MAX_DIST*DEBUGPROJECTILEVISUALIZER_EXTENDED_MAX_DIST))
			{
				sprintf(textbuf, "uh: %d", projectile->getHandle());
				DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
				textoffy += 16;

				if (projectile->getBulletType() == NULL)
				{
					sprintf(textbuf, "type: *** INVALID! ***");
				} else {
					sprintf(textbuf, "type: %s", projectile->getBulletType()->getPartTypeIdString());
				}
				DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
				textoffy += 16;

				if (projectile->getLifeTime() != 0)
				{
					sprintf(textbuf, "lifetime: %s", int2str(projectile->getLifeTime()));
					DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
					textoffy += 16;
				} else {
					sprintf(textbuf, "afterlifetime: %s", int2str(projectile->getAfterLifeTime()));
					DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
					textoffy += 16;
				}
			}

			//index++;
		}
	}

}
