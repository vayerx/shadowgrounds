
#ifndef IGAMECONTROLLERKEYREADER_H
#define IGAMECONTROLLERKEYREADER_H

namespace ui
{
	class IGameControllerKeyreader
	{
		public:
			virtual void readKey(char ascii, int keycode, 
				const char *keycodeName) = 0;

			virtual ~IGameControllerKeyreader() {};
	};
}

#endif

