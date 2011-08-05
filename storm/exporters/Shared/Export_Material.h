// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EXPORT_MATERIAL_H
#define INCLUDED_EXPORT_MATERIAL_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
#pragma warning(disable: 4710) // function not inlined
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

namespace frozenbyte {
namespace exporter {

class Material
{
	std::string name;

	std::string baseTexture;
	std::string luminosityTexture;
	std::string bumpTexture;
	std::string reflectionTexture;
	std::string distortionTexture;

	Color diffuseColor;
	Color luminosityColor;

	bool alphaTexture;
	bool additiveAlpha;

	bool doubleSided;
	float transparency;
	float glow;
	float reflectionFactor;

	VC2 scrollSpeed;
	bool scrollAutoStart;

public:
	explicit Material(const std::string &name);
	~Material();

	const std::string &getName() const;
	const std::string &getBaseTexture() const;
	const std::string &getLuminosityTexture() const;
	const std::string &getBumpTexture() const;
	const std::string &getReflectionTexture() const;
	const std::string &getDistortionTexture() const;
	const Color &getDiffuseColor() const;
	const Color &getLuminosityColor() const;

	void setBaseTexture(const std::string &name);
	void setLuminosityTexture(const std::string &name);
	void setBumpTexture(const std::string &name);
	void setReflectionTexture(const std::string &name, float factor);
	void setDistortionTexture(const std::string &name);
	
	void useAlphaTexture();
	void useAdditiveAlpha();
	void setDiffuseColor(const Color &color);
	void setLuminosityColor(const Color &color);
	void setDoubleSided(bool doubleSided);
	void setTransparency(float value);
	void setGlow(float value);
	void setScroll(const VC2 &scrollSpeed, bool scrollAutoStart);

	void writeTexturesToFile(FILE *fp, std::vector<std::string> *saveTextures) const;
	void writeToFile(FILE *fp, const std::vector<std::string> &textures) const;

	bool isDoubleSided() const;
	bool hasAlpha() const;
	bool hasAdditiveAlpha() const;
	bool hasAlphaTexture() const;
	float getTransparency() const;
	float getGlow() const;
	float getReflectionFactor() const;
	const VC2 &getScrollSpeed() const;
	bool scrollAutoStartEnabled() const;
	
	bool operator == (const Material &hs) const;
	void copy(const Material &m);

private:
	short int getTextureIndex(const std::vector<std::string> &textures, const std::string &texture) const;
};

//bool isEqualMaterial(const Material &lhs, const Material &rhs);

} // end of namespace export
} // end of namespace frozenbyte

#endif
