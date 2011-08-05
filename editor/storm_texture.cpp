// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "storm_texture.h"
#include "storm.h"
#include <istorm3d_texture.h>
#include <istorm3d.h>

namespace frozenbyte {
namespace editor {

boost::shared_ptr<IStorm3D_Texture> loadTexture(const std::string &fileName, Storm &storm)
{
	IStorm3D_Texture *t = storm.storm->CreateNewTexture(fileName.c_str());
	if(!t)
	{
#ifdef LEGACY_FILES
		t = storm.storm->CreateNewTexture("Missing.dds");
#else
		t = storm.storm->CreateNewTexture("missing.dds");
#endif
	}

	return boost::shared_ptr<IStorm3D_Texture> (t, std::mem_fun(&IStorm3D_Texture::Release));
}

boost::shared_ptr<IStorm3D_Texture> createTexture(int width, int height, Storm &storm)
{
	IStorm3D_Texture *t = storm.storm->CreateNewTexture(width, height, IStorm3D_Texture::TEXTYPE_BASIC);
	Storm3D_SurfaceInfo info = t->GetSurfaceInfo();
	
	assert(info.width == width && info.height == height);
	return boost::shared_ptr<IStorm3D_Texture> (t, std::mem_fun(&IStorm3D_Texture::Release));
}

} // end of namespace editor
} // end of namespace frozenbyte
