
#ifndef ITEM_H
#define ITEM_H

#include <DatatypeDef.h>
#include <string>

namespace ui
{
  class VisualObject;
	class Spotlight;
}

namespace game
{
	class ProgressBar;
	class AbstractPhysicsObject;

	class Item
	{
		public:
			Item(int itemTypeId);
			
			~Item();

			void setCustomScript(const char *customScript);

			const char *getCustomScript() const;

			void setCustomTipText(const char *customTipText);

			const char *getCustomTipText() const;

			void setSpecialString(const char *specialString);

			const char *getSpecialString() const;

			void prepareForRender();

			void setPosition(const VC3 &position);

			void setRotation(const VC3 &rotation);

			const VC3 &getPosition();

			const VC3 &getRotation();

			void setVisualObject(ui::VisualObject *visualObject);

			ui::VisualObject *getVisualObject();

			void setHalo(ui::Spotlight *halo);

			void setWeaponType(bool weaponType);

			int getItemTypeId();

			//ui::VisualObject *getVisualObject();

			int getPickupDelay();

			void setPickupDelay(int pickupDelay);

			int getCount();

			void addCount();

			void decreaseCount();

			void setCount(int count);

			void setBlinking(bool blinking);

			bool isBlinking();

			void setEnabled(bool enabled);

			bool isEnabled() const;

			void setReEnableTime(int reEnableTime);

			int getReEnableTime();

			bool advanceReEnable();

			ProgressBar *getProgressBar();

			void createProgressBar();

			void deleteProgressBar();

			// added by Pete for the use of TargetDisplay
			// -1 is no highlight
			int getHighlightStyle() const;
			void setHighlightStyle( int style );

			bool hasHighlightText() const;
			
			std::string getHighlightText() const;
			void setHighlightText( const std::string& styletext );

			// hax hax physh-x
			void setGamePhysicsObject( AbstractPhysicsObject* obj ) { this->physicsObject = obj; }
			AbstractPhysicsObject* getGamePhysicsObject() const { return this->physicsObject; }


		private:
			VC3 position;
			ui::VisualObject *visualObject;
			ui::Spotlight *halo;
			VC3 rotation;
			bool weaponType;
			int itemTypeId;
			int pickupDelay;
			int count;
			bool blinking;
			bool enabled;
			int reEnableTime;
			char *customScript;
			char *specialString;
			char *customTipText;
			ProgressBar *progress;

			// Added by Pete for the use of gui highlight
			int			highlightStyle;
			std::string	highlightText;

			AbstractPhysicsObject* physicsObject;

		public:
			struct ItemSpawner *spawner;
	};
}

#endif

