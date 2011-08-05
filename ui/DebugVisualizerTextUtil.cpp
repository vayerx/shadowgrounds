
#include "precompiled.h"

#include "DebugVisualizerTextUtil.h"
#include "../ui/uidefaults.h"
#include "../ogui/OguiStormDriver.h"
#include "../ogui/orvgui2.h"

#include <Storm3D_UI.h>

extern IStorm3D_Scene *disposable_scene;

extern int scr_width;
extern int scr_height;

namespace ui
{

	void DebugVisualizerTextUtil::renderText(const VC3 &position, int offsetX, int offsetY, const char *text)
	{
		if (ui::defaultIngameFont != NULL)
		{
			IStorm3D_Font *f = ((OguiStormFont *)ui::defaultIngameFont)->fnt;

			if (text == NULL)
				text = "(null)";

			VC3 pos = position;
			VC3 result = VC3(0,0,0);
			float rhw = 0;
			float real_z = 0;
			IStorm3D_Camera *cam = disposable_scene->GetCamera();
			bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);

			if (infront)
			{
				bool offscreen = false;
				int x = (int)(result.x * scr_width);
				int y = (int)(result.y * scr_height);
				if (x < 0 || y < 0 || x >= scr_width || y >= scr_height)
					offscreen = true;

				if (!offscreen)
				{
					float scrfloatx = (float)(x + offsetX);
					float scrfloaty = (float)(y + offsetY);
					float alpha = 1.0f;
					COL col = COL(1,1,1);
					disposable_scene->Render2D_Text(f, VC2(scrfloatx, scrfloaty), VC2(16, 16), text, alpha, col);
				}
			}
		}
	}

}
