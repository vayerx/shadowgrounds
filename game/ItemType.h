
#ifndef ITEMTYPE_H
#define ITEMTYPE_H

#include <string>

namespace game
{
  class ItemType
	{
		public:

			enum DISABLE_EFFECT
			{
				DISABLE_EFFECT_NONE = 1,
				DISABLE_EFFECT_DISAPPEAR = 2,
				DISABLE_EFFECT_NOBLINK = 3
			};

			ItemType();

			~ItemType();

			inline const char *getName() const
			{
				return name;
			}

			inline const char *getModelFilename() const
			{
				return model;
			}

			inline const char *getHalo() const
			{
				return halo;
			}

			inline const char *getScript() const
			{
				return script;
			}

			inline const char *getTipText() const
			{
				return tipText;
			}

			inline bool isWeaponType() const
			{
				return weaponType;
			}

			inline DISABLE_EFFECT getDisableEffect() const
			{
				return disableEffect;
			}

			inline bool isExecutable() const
			{
				return executable;
			}

			inline int getTipPriority() const
			{
				return tipPriority;
			}

			inline int getHighlightStyle() const
			{
				return highlightStyle;
			}

			inline std::string getHighlightText() const
			{
				return highlightText;
			}

			inline bool getBlinking() const
			{
				return blinking;
			}

			inline bool getPhysicsEnabled() const
			{
				return physicsEnabled;
			}

			inline float getPhysicsMass() const
			{
				return physicsMass;
			}

			void setName(const char *name);

			void setModelFilename(const char *model);

			void setHalo(const char *halo);

			void setScript(const char *script);

			void setTipText(const char *tipText);

			void setWeaponType(bool weaponType);

			void setExecutable(bool executable);

			void setTipPriority(int tipPriority);

			void setDisableEffect(DISABLE_EFFECT disableEffect);

			void setHighlightStyle( int style );

			void setHighlightText( const std::string& text );

			void setBlinking( bool blinking );

			// added by Pete
			void setPhysicsEnabled( bool physics );
			void setPhysicsMass( float mass );

		private:
			char *name;
			char *model;
			char *halo;
			char *script;
			char *tipText;
			bool weaponType;
			bool executable;
			int tipPriority;
			DISABLE_EFFECT disableEffect;

			int			highlightStyle;
			std::string highlightText;
			bool		blinking;

			bool		physicsEnabled;
			float		physicsMass;
	};
}

#endif

