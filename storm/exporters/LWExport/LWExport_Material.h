// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LWEXPORT_MATERIAL_H
#define INCLUDED_LWEXPORT_MATERIAL_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

#ifndef INCLUDED_LWSURF_H
#define INCLUDED_LWSURF_H
#include <lwsurf.h>
#endif

namespace frozenbyte {
namespace exporter {

/**  @class LWMaterial
  *  @brief Collects material data (textures / surface properties)
  *  @author Juha Hiekkamäki
  *  @version 1.0
  *  @date 2001
  */
class LWMaterial
{
	LWSurfaceID lwId;
	int exporterId;

	std::string baseTexture;
	std::string bumpTexture;
	std::string reflectionTexture;
	std::string distortionTexture;
	mutable std::string luminosityTexture;
	
	// Textures id's. Get uv's from here
	LWTextureID textureId;
	mutable LWTextureID lightmapId;
	LWTextureID reflectionId;
	LWImageID distortionId;
	LWTextureID alphaId;

	// Properties
	std::string name;
	std::string objectName;
	
	float smoothingAngle;
	float transparency;
	float glow;
	float reflectionFactor;

	bool doubleSided;
	
	Color diffuseColor;
	Color luminosityColor;
	bool additiveAlpha;

	VC2 scrollSpeed;
	bool scrollAutoStart;

public:
	explicit LWMaterial(LWSurfaceID id);
	~LWMaterial();

	const std::string &getName() const;
	const std::string &getObjectName() const;
	
	LWTextureID getTextureId() const;
	LWTextureID getLightmapId() const;
	float getSmoothingAngle() const;
	bool isDoubleSided() const { return doubleSided; }
	int getExporterId() const;

	bool collectProperties();

	void removeLightmap() const { luminosityTexture = ""; lightmapId = 0; }
	void add();
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
