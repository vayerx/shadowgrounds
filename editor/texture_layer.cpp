// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "texture_layer.h"
#include "common_dialog.h"
#include "terrain_textures.h"
#include "terrain_texture_generator.h"

#include <map>
#include <cassert>
#include <windows.h>
#include "resource/resource.h"

namespace frozenbyte {
namespace editor {

struct TextureLayerData
{
	TerrainTextures &terrainTextures;
	Storm &storm;

	TextureLayerData(TerrainTextures &terrainTextures_, Storm &storm_)
	:	terrainTextures(terrainTextures_),
		storm(storm_)
	{
	}

	~TextureLayerData()
	{
	}

	void initComboBox(HWND windowHandle, int id)
	{
		SendDlgItemMessage(windowHandle, id, CB_RESETCONTENT, 0, 0);

		if(id == IDC_BOTTOM_TEXTURE)
			SendDlgItemMessage(windowHandle, id, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("(none)"));

		for(int i = 0; i < terrainTextures.getTextureCount(); ++i)
		{
			std::string string = getFileName(terrainTextures.getTexture(i));
			SendDlgItemMessage(windowHandle, id, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
		}

		SendDlgItemMessage(windowHandle, id, CB_SETCURSEL, 0, 0);
	}

	int getTexture(HWND windowHandle, int id)
	{
		int index = SendDlgItemMessage(windowHandle, id, CB_GETCURSEL, 0, 0);
		if(id == IDC_BOTTOM_TEXTURE)
			return index - 1;
		
		return index;
	}

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		TextureLayerData *data = reinterpret_cast<TextureLayerData *> (GetWindowLong(windowHandle, GWL_USERDATA));

		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			data = reinterpret_cast<TextureLayerData *> (lParam);

			data->initComboBox(windowHandle, IDC_TERRAIN_TEXTURE1);
			data->initComboBox(windowHandle, IDC_TERRAIN_TEXTURE2);
			data->initComboBox(windowHandle, IDC_BOTTOM_TEXTURE);
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY)
				EndDialog(windowHandle, 0);

			if(command == IDC_GENERATE)
			{
				TerrainTextureGenerator generator(data->storm, data->terrainTextures);

				generator.setTerrainTexture(0, data->getTexture(windowHandle, IDC_TERRAIN_TEXTURE1));
				generator.setTerrainTexture(1, data->getTexture(windowHandle, IDC_TERRAIN_TEXTURE2));
				generator.setWater(data->getTexture(windowHandle, IDC_BOTTOM_TEXTURE), 0);
				
				generator.generate();
				EndDialog(windowHandle, 1);
			}
		}

		return 0;
	}

	bool show()
	{
		if(terrainTextures.getTextureCount() == 0)
			return false;

		if(DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TEXTURELAYERS), 0, DialogHandler, reinterpret_cast<LPARAM> (this)) == 1)
			return true;

		return false;
	}
};

TextureLayer::TextureLayer(TerrainTextures &textures, Storm &storm)
{
	boost::scoped_ptr<TextureLayerData> tempData(new TextureLayerData(textures, storm));
	data.swap(tempData);
}

TextureLayer::~TextureLayer()
{
}

bool TextureLayer::show()
{
	return data->show();
}

} // end of namespace editor
} // end of namespace frozenbyte
