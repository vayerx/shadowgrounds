// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Material.h"
#include <cstdio>

namespace frozenbyte {
namespace exporter {

Material::Material(const std::string &name_)
:	name(name_),
	alphaTexture(false),
	additiveAlpha(false),

	doubleSided(false),
	diffuseColor(1,1,1),
	transparency(0.f),
	glow(0),
	reflectionFactor(0),
	scrollAutoStart(false)
{
}

Material::~Material()
{
}

const std::string &Material::getName() const
{
	return name;
}

const std::string &Material::getBaseTexture() const
{
	return baseTexture;
}

const std::string &Material::getLuminosityTexture() const
{
	return luminosityTexture;
}

const std::string &Material::getBumpTexture() const
{
	return bumpTexture;
}

const std::string &Material::getReflectionTexture() const
{
	return reflectionTexture;
}

const std::string &Material::getDistortionTexture() const
{
	return distortionTexture;
}

const Color &Material::getDiffuseColor() const
{
	return diffuseColor;
}

const Color &Material::getLuminosityColor() const
{
	return luminosityColor;
}

void Material::setBaseTexture(const std::string &name)
{
	baseTexture = name;
}

void Material::setLuminosityTexture(const std::string &name)
{
	luminosityTexture = name;
}

void Material::setBumpTexture(const std::string &name)
{
	bumpTexture = name;
}

void Material::setDistortionTexture(const std::string &name)
{
	distortionTexture = name;
}

void Material::useAlphaTexture()
{
	alphaTexture = true;
}

void Material::useAdditiveAlpha()
{
	additiveAlpha = true;
}

void Material::setReflectionTexture(const std::string &name, float factor)
{
	reflectionTexture = name;
	reflectionFactor = factor;
}

void Material::setDiffuseColor(const Color &color)
{
	diffuseColor = color;
}

void Material::setLuminosityColor(const Color &color)
{
	luminosityColor = color;
}

void Material::setDoubleSided(bool doubleSided_)
{
	doubleSided = doubleSided_;
}

void Material::setTransparency(float value)
{
	transparency = value;
}

void Material::setGlow(float value)
{
	glow = value;
}

void Material::setScroll(const VC2 &scrollSpeed_, bool scrollAutoStart_)
{
	scrollSpeed = scrollSpeed_;
	scrollAutoStart = scrollAutoStart_;
}

void Material::writeTexturesToFile(FILE *fp, std::vector<std::string> *saveTextures) const
{
	std::string textureName;
	for(int i = 0; i < 5; ++i)
	{
		if(i == 0)
			textureName = baseTexture;
		else if(i == 1)
			textureName = luminosityTexture;
		else if(i == 2)
			textureName = reflectionTexture;
		else if(i == 3)
			textureName = bumpTexture;
		else if(i == 4)
			textureName = distortionTexture;

		if(textureName.size() == 0)
			continue;

		std::vector<std::string>::iterator it = std::find(saveTextures->begin(), saveTextures->end(), textureName);
		if(it == saveTextures->end())
			continue;

		// Remove from list
		*it = "";

		// Get filename without dirs
		for(int j = textureName.size() - 1; j > 1; --j)
		{
			if(textureName[j] == '\\')
			{
				textureName = textureName.substr(j+1, textureName.size() - 1);
				break;
			}
		}

		// Save
		fwrite(textureName.c_str(), 1, textureName.size() + 1, fp);

		// Not used
		DWORD dw0 = 0;
		fwrite(&dw0, 1, sizeof(DWORD), fp); // ident
		WORD w0 = 0;
		fwrite(&w0, 1, sizeof(WORD), fp); // Anim params
		fwrite(&w0, 1, sizeof(WORD), fp); // Anim params
		BYTE b0 = 0;
		fwrite(&b0, 1, 1, fp); // Not dynamic
	}
}

void Material::writeToFile(FILE *fp, const std::vector<std::string> &textures) const
{
	fwrite(name.c_str(), 1, name.size() + 1, fp);

	short int baseIndex = getTextureIndex(textures, baseTexture);
	short int lightIndex = getTextureIndex(textures, luminosityTexture);
	short int bumpIndex = getTextureIndex(textures, bumpTexture);
	short int reflectionIndex = getTextureIndex(textures, reflectionTexture);
	short int distortionIndex = getTextureIndex(textures, distortionTexture);

	fwrite(&baseIndex, sizeof(short int), 1, fp);
	fwrite(&lightIndex, sizeof(short int), 1, fp);
	fwrite(&bumpIndex, sizeof(short int), 1, fp);
	fwrite(&reflectionIndex, sizeof(short int), 1, fp);
	fwrite(&distortionIndex, sizeof(short int), 1, fp);

	fwrite(&diffuseColor, sizeof(float), 3, fp); // Diffuse
	Color selfIllumination = luminosityColor;
	fwrite(&selfIllumination, sizeof(float), 3, fp); // Self illumination
	Color specularColor;
	fwrite(&specularColor, sizeof(float), 3, fp); // Specular color
	float specularSharpness;
	fwrite(&specularSharpness, sizeof(float), 1, fp); // Specular sharpness

	char doubleSidedC = 0;
	if(doubleSided == true)
		doubleSidedC = 1;
	fwrite(&doubleSidedC, sizeof(char), 1, fp); // Doublesided

	char wireFrame = 0;
	fwrite(&wireFrame, sizeof(char), 1, fp); // Wireframe

	int texGen = 0;
	fwrite(&texGen, sizeof(int), 1, fp); // Texture generation

	int alphaType = 0;
	if(transparency > 0.001f)
		alphaType = 1;
	if(alphaTexture)
		alphaType = 2;
	if(additiveAlpha)
		alphaType = 3;

	fwrite(&alphaType, sizeof(int), 1, fp); // Alpha type
	fwrite(&transparency, sizeof(float), 1, fp); // Transparency
	fwrite(&glow, sizeof(float), 1, fp); // Glow

	// Scroll
	fwrite(&scrollSpeed, sizeof(float), 2, fp);
	char scrollStart = 0;
	if(scrollAutoStart)
		scrollStart = 1;
	fwrite(&scrollStart, sizeof(char), 1, fp);

	// Additional infos
	if(lightIndex >= 0)
	{
		int i = 0; // BOP_MUL
		fwrite(&i, sizeof(int), 1, fp);
		float f = 0.5f; // Blend factor
		fwrite (&f, sizeof(float), 1, fp);
	}
	if(reflectionIndex >= 0)
	{
		int i = 3; // BOP_ADD
		fwrite(&i, sizeof(int), 1, fp);
		float f = .5f; // Blend factor
		fwrite (&reflectionFactor, sizeof(float), 1, fp);
	}
}

bool Material::isDoubleSided() const
{
	return doubleSided;
}

bool Material::hasAlpha() const
{
	if(transparency > 0.01f)
		return true;

	return false;
}

bool Material::hasAdditiveAlpha() const
{
	return additiveAlpha;
}

bool Material::hasAlphaTexture() const
{
	return alphaTexture;
}

float Material::getTransparency() const
{
	return transparency;
}

float Material::getGlow() const
{
	return glow;
}

float Material::getReflectionFactor() const
{
	return reflectionFactor;
}

const VC2 &Material::getScrollSpeed() const
{
	return scrollSpeed;
}

bool Material::scrollAutoStartEnabled() const
{
	return scrollAutoStart;
}

bool Material::operator == (const Material &rhs) const
{
	if(name.find("EffectTexture") != name.npos)
		return false;
	if(rhs.name.find("EffectTexture") != rhs.name.npos)
		return false;

	if(baseTexture != rhs.baseTexture)
		return false;
	if(luminosityTexture != rhs.luminosityTexture)
		return false;
	if(bumpTexture != rhs.bumpTexture)
		return false;
	if(reflectionTexture != rhs.reflectionTexture)
		return false;
	if(distortionTexture != rhs.distortionTexture)
		return false;
	if(diffuseColor != rhs.diffuseColor)
		return false;
	if(luminosityColor != rhs.luminosityColor)
		return false;

	if(alphaTexture != rhs.alphaTexture)
		return false;
	if(additiveAlpha != rhs.additiveAlpha)
		return false;
	if(doubleSided != rhs.doubleSided)
		return false;
	if(fabs(transparency - rhs.transparency) > 0.1f)
		return false;
	if(fabs(glow - rhs.glow) > 0.01f)
		return false;
	if(fabs(reflectionFactor - rhs.reflectionFactor) > 0.01f)
		return false;

	if(fabs(scrollSpeed.x - rhs.scrollSpeed.x) > 0.001f)
		return false;
	if(fabs(scrollSpeed.y - rhs.scrollSpeed.y) > 0.001f)
		return false;
	if(scrollAutoStart != rhs.scrollAutoStart)
		return false;

	if(fabs(diffuseColor.r - rhs.diffuseColor.r) > 0.1f)
		return false;
	if(fabs(diffuseColor.g - rhs.diffuseColor.g) > 0.1f)
		return false;
	if(fabs(diffuseColor.b - rhs.diffuseColor.b) > 0.1f)
		return false;
	if(fabs(luminosityColor.r - rhs.luminosityColor.r) > 0.1f)
		return false;
	if(fabs(luminosityColor.g - rhs.luminosityColor.g) > 0.1f)
		return false;
	if(fabs(luminosityColor.b - rhs.luminosityColor.b) > 0.1f)
		return false;

	return true;
}

void Material::copy(const Material &m)
{
	baseTexture = m.baseTexture;
	luminosityTexture = m.luminosityTexture;
	reflectionTexture = m.reflectionTexture;
	distortionTexture = m.distortionTexture;

	diffuseColor = m.diffuseColor;
	luminosityColor = m.luminosityColor;

	alphaTexture = m.alphaTexture;
	additiveAlpha = m.additiveAlpha;

	doubleSided = m.doubleSided,
	transparency = m.transparency;
	glow = m.glow;

	scrollSpeed = m.scrollSpeed;
	scrollAutoStart = m.scrollAutoStart;
}

short int Material::getTextureIndex(const std::vector<std::string> &textures, const std::string &texture) const
{
	for(unsigned int i = 0; i < textures.size(); ++i)
		if(textures[i] == texture)
			return static_cast<short int> (i);
	
	return -1;
}

} // end of namespace export
} // end of namespace frozenbyte
