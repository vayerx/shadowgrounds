// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Material.h"
#include "LWExport_Manager.h"

#include "..\Shared\Export_Material.h"
#include "..\Shared\Export_Exporter.h"
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace frozenbyte {
namespace exporter {

namespace {

	void getScroll(const std::string &name, const std::string &key, VC2 &speed, bool &autostart)
	{
		string::size_type start = name.find(key);

		speed.x = speed.y = 0.f;
		autostart = false;

		if(start != name.npos)
		{
			start += key.size() + 1;
			string::size_type end = name.find(")", start);

			if(end != name.npos)
			{
				string values = name.substr(start, end - start);
				values += "\0";

				try
				{
					char *value = strtok(&values[0], ",");
					if(value)
					{
						speed.x = boost::lexical_cast<float> (value);
					
						value = strtok(0, ",");
						if(value)
						{
							speed.y = boost::lexical_cast<float> (value);
							autostart = true;

							value = strtok(0, ",");
							if(value)
								autostart = boost::lexical_cast<bool> (value);
						}
					}
				}
				catch(...)
				{
				}
			}
		}
	}


}

// Helper function
bool getTextureFileName(LWTextureID textureID, std::string *result, LWTextureFuncs *textureFunctions, LWImageList *imageList, bool onlyUvMapped);

LWMaterial::LWMaterial(LWSurfaceID id)
:	lwId(id),
	exporterId(-1),

	textureId(0),
	lightmapId(0),
	reflectionId(0),
	distortionId(0),

	smoothingAngle(3.15f),
	transparency(0.f),
	glow(0),
	doubleSided(false),

	diffuseColor(1,1,1),
	additiveAlpha(false),
	scrollAutoStart(false)
{
}

LWMaterial::~LWMaterial()
{
}

const std::string &LWMaterial::getName() const
{
	return name;
}

const std::string &LWMaterial::getObjectName() const
{
	return objectName;
}

bool LWMaterial::collectProperties()
{
	LWSurfaceFuncs *surfaceFunctions = Manager::getSingleton()->getSurfaceFunctions();
	if(surfaceFunctions == 0)
		return false;

	// Properties
	name = surfaceFunctions->name(lwId);
	objectName = surfaceFunctions->sceneObject(lwId);

	smoothingAngle = (float) *(surfaceFunctions->getFlt(lwId, SURF_SMAN));
	transparency = (float) *(surfaceFunctions->getFlt(lwId, SURF_TRAN));
	double *color = surfaceFunctions->getFlt(lwId, SURF_COLR);
	diffuseColor.r = static_cast<float> (color[0]);
	diffuseColor.g = static_cast<float> (color[1]);
	diffuseColor.b = static_cast<float> (color[2]);

	color = surfaceFunctions->getFlt(lwId, SURF_LUMI);
	if(color)
		luminosityColor.r = luminosityColor.g = luminosityColor.b = float(color[0]);

	additiveAlpha = false;
	double *additiveBuffer = surfaceFunctions->getFlt(lwId, SURF_ADTR);
	if(additiveBuffer && additiveBuffer[0] > 0.001)
		additiveAlpha = true;

	double *glowValue = surfaceFunctions->getFlt(lwId, SURF_GVAL);
	if(glowValue)
		glow = float(glowValue[0]);

	double *reflectionValue = surfaceFunctions->getFlt(lwId, SURF_REFL);
	if(reflectionValue)
		reflectionFactor = float(reflectionValue[0]);

	int sidedness = (surfaceFunctions->getInt(lwId, SURF_SIDE));
	if(sidedness > 1)
		doubleSided = true;

	getScroll(name, "Scroll", scrollSpeed, scrollAutoStart);

	// Textures
	LWTextureFuncs *textureFunctions = Manager::getSingleton()->getTextureFunctions();
	LWImageList *imageList = Manager::getSingleton()->getImageList();

	if(textureFunctions && imageList)
	{
		textureId = surfaceFunctions->getTex(lwId, SURF_COLR);
		lightmapId = surfaceFunctions->getTex(lwId, SURF_DIFF);
		reflectionId = surfaceFunctions->getTex(lwId, SURF_REFL);
		distortionId = surfaceFunctions->getImg(lwId, SURF_TIMG);
		alphaId = surfaceFunctions->getTex(lwId, SURF_TRAN);

		getTextureFileName(textureId, &baseTexture, textureFunctions, imageList, false);
		getTextureFileName(lightmapId, &luminosityTexture, textureFunctions, imageList, true);
		getTextureFileName(reflectionId, &reflectionTexture, textureFunctions, imageList, false);
		//getTextureFileName(distortionId, &distortionTexture, textureFunctions, imageList, false);

		if(distortionId > 0)
		{
			const char *distName = imageList->filename(distortionId, 0);
			if(distName)
				distortionTexture = distName;
		}
	}

	//add();
	return true;
}

LWTextureID LWMaterial::getTextureId() const
{
	return textureId;
}

LWTextureID LWMaterial::getLightmapId() const
{
	return lightmapId;
}

float LWMaterial::getSmoothingAngle() const
{
	return smoothingAngle;
}

int LWMaterial::getExporterId() const
{
	return exporterId;
}

bool getTextureFileName(LWTextureID textureId, std::string *result, LWTextureFuncs *textureFunctions, LWImageList *imageList, bool onlyUvMapped)
{
	if(textureId == 0)
		return false;

	// Find first layer which contain texture
	LWTLayerID layerId = textureFunctions->firstLayer(textureId);
	while(layerId != NULL)
	{
		// Not an image texture
		if(textureFunctions->layerType(layerId) != TLT_IMAGE)
		{
			//layerId = textureFunctions->nextLayer(textureId, layerId);
			//continue;

			break;
		}

		// Can't get image id
		LWImageID imageId = 0;
		if(!textureFunctions->getParam(layerId, TXTAG_IMAGE, &imageId))
		{
			//layerId = textureFunctions->nextLayer(textureId, layerId);
			//continue;
			break;
		}

		if(imageId == 0)
			break;

		// No texture name
		const char *fname = imageList->filename(imageId, 0);
		if(fname == 0)
		{
			//layerId = textureFunctions->nextLayer(textureId, layerId);
			//continue;

			break;
		}

		/*
		if(onlyUvMapped)
		{
			int projection = 0;
			textureFunctions->getParam(layerId, TXTAG_PROJ, &projection);
			if(projection != TXPRJ_UVMAP)
				break;
		}
		*/

		// Finally
		*result = fname;
		return true;
	}

	return false;
}

void LWMaterial::add()
{
	Material material(name);
	//if(baseTexture.empty())
		material.setDiffuseColor(diffuseColor);

	material.setBaseTexture(baseTexture);
	material.setLuminosityTexture(luminosityTexture);
	material.setBumpTexture(bumpTexture);
	material.setReflectionTexture(reflectionTexture, reflectionFactor);
	material.setDistortionTexture(distortionTexture);
	material.setTransparency(transparency);
	material.setDoubleSided(doubleSided);
	material.setLuminosityColor(luminosityColor);
	material.setGlow(glow);
	material.setScroll(scrollSpeed, scrollAutoStart);

	if(alphaId)
		material.useAlphaTexture();
	if(additiveAlpha)
		material.useAdditiveAlpha();

	exporterId = Manager::getSingleton()->getExporter()->getModel().addMaterial(material);
}

} // end of namespace export
} // end of namespace frozenbyte
