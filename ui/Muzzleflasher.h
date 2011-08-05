
#ifndef MUZZLEFLASHER_H
#define MUZZLEFLASHER_H

namespace ui
{
	class IPointableObject;
	class VisualObject;

	class Muzzleflasher
	{
		public:

			// NOTICE: expects a _unit_, not just any IPointableObject
			// (will do a nasty cast to that)
			// should use some "muzzleflashable" interface instead.

			static void createMuzzleflash(IPointableObject *unit, 
				VisualObject *muzzleflash, int muzzleFlashBarrelNumber);


			static void deleteMuzzleflash(IPointableObject *unit);
			static void deleteMuzzleflash(IPointableObject *unit, const std::string &name);

			static void createMuzzleflash(IPointableObject *unit, VisualObject *muzzleflash, const std::string &name, const std::string &helper);
			static bool getMuzzleflash(IPointableObject *unit, VisualObject *muzzleflash, const std::string &name, const std::string &helper, VC3 &pos, VC3 &scale, QUAT &rot);
			static void updateMuzzleflash(IPointableObject *unit, VisualObject *muzzleflash, const std::string &name, const std::string &helper, const VC3 &pos, const VC3 &scale);

	};

}

#endif

