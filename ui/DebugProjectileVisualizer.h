
#ifndef DEBUGPROJECTILEVISUALIZER_H
#define DEBUGPROJECTILEVISUALIZER_H

namespace game
{
	class ProjectileList;
}

namespace ui
{
	class DebugProjectileVisualizer
	{
	public:
		static void visualizeProjectiles(game::ProjectileList *projectiles, const VC3 &cameraPosition);
	};
}

#endif
