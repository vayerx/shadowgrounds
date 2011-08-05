
#ifndef UPGRADETYPE_H
#define UPGRADETYPE_H

namespace game
{
  class UpgradeType
	{
		public:

			UpgradeType();

			~UpgradeType();

			inline const char *getName() const
			{
				return name;
			}

			inline const char *getScript() const
			{
				return script;
			}

			inline const char *getDescription() const
			{
				return description;
			}

			inline int getCost() const
			{
				return cost;
			}

			inline const char *getPart() const
			{
				return part;
			}

			void setName(const char *name);

			void setScript(const char *script);

			void setDescription(const char *description);

			void setCost(int cost);

			void setPart(const char *part);

		private:
			char *name;
			char *script;
			char *description;
			char *part;
			int cost;
	};
}

#endif

