// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_lightmap.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../util/jpak.h"
#include "storm.h"
#include "storm_texture.h"
#include "exporter.h"
#include "exporter_scene.h"
#include <istorm3d.h>
#include <istorm3d_scene.h>
#include <istorm3d_mesh.h>
#include <istorm3d_material.h>
#include <istorm3d_model.h>
#include <istorm3d_terrain_renderer.h>
#include <boost/scoped_array.hpp>
#include <fstream>

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace editor {
namespace {

	struct Pixel
	{
		TColor<unsigned char> color;
		TColor<unsigned char> light;

		void addColor(const COL &c)
		{
			int r = color.r + int(c.r * 255.f);
			int g = color.g + int(c.g * 255.f);
			int b = color.b + int(c.b * 255.f);

			if(r > 255)
				r = 255;
			if(g > 255)
				g = 255;
			if(b > 255)
				b = 255;

			color.r = r;
			color.g = g;
			color.b = b;
			light = color;
		}

		void addSunColor(const COL &c)
		{
			int r = light.r + int(c.r * 255.f);
			int g = light.g + int(c.g * 255.f);
			int b = light.b + int(c.b * 255.f);

			if(r > 255)
				r = 255;
			if(g > 255)
				g = 255;
			if(b > 255)
				b = 255;

			light.r = r;
			light.g = g;
			light.b = b;
		}
	};

	static const int SAMPLE_START = 0;
	static const int SAMPLE_END = 9;
	
	static const int TEXTURE_SIZE_FACTOR = 16;
	static const float MAGIC_FACTOR = 1.3f;
}

typedef vector<Pixel> ValueList;
typedef vector<TerrainLightMap::PointLight> LightList;

struct TerrainLightMap::Data
{
	Storm &storm;

	ValueList values;
	VC2I size;

	IStorm3D_Texture *t;
	IStorm3D_Material *m;

	Data(Storm &storm_)
	:	storm(storm_),
		t(0),
		m(0)
	{
	}

	void reset()
	{
		t = 0;
		m = 0;

		//int xBlocks = storm.heightmapResolution.x / IStorm3D_Terrain::BLOCK_SIZE;
		//int yBlocks = storm.heightmapResolution.y / IStorm3D_Terrain::BLOCK_SIZE;
		//size.x = xBlocks * (TEXTURE_SIZE_FACTOR * IStorm3D_Terrain::BLOCK_SIZE - 1) + 1;
		//size.y = yBlocks * (TEXTURE_SIZE_FACTOR * IStorm3D_Terrain::BLOCK_SIZE - 1) + 1;

		size.x = storm.heightmapResolution.x * TEXTURE_SIZE_FACTOR;
		size.y = storm.heightmapResolution.y * TEXTURE_SIZE_FACTOR;
		values.resize(size.x * size.y);
	}

	VC3 fudgePosition(const VC3 &pos, float xd, float yd, int index) const
	{
		VC3 tempPos = pos;

		float sampleFactor = .4f;
		//float sampleFactor = .25f;
		float xd1 = xd * sampleFactor;
		float xd2 = xd * sampleFactor * 2.1f;
		float yd1 = yd * sampleFactor;
		float yd2 = yd * sampleFactor * 1.9f;

		if(index == 0)
		{
			tempPos.x -= xd2 * sampleFactor;
			tempPos.z += yd1 * sampleFactor;
		}
		else if(index == 1)
		{
			tempPos.x -= xd1 * sampleFactor;
			tempPos.z -= yd1 * sampleFactor;
		}
		else if(index == 2)
		{
			tempPos.x += xd2 * sampleFactor;
			tempPos.z += yd2 * sampleFactor;
		}
		else if(index == 3)
		{
			tempPos.x -= xd1 * sampleFactor;
			tempPos.z += yd2 * sampleFactor;
		}
		else if(index == 4)
		{
			tempPos.x += xd1 * sampleFactor;
			tempPos.z -= yd2 * sampleFactor;
		}
		else if(index == 5)
		{
			tempPos.x += xd1 * sampleFactor;
			tempPos.z += yd1 * sampleFactor;
		}
		else if(index == 6)
		{
			tempPos.x -= xd2 * sampleFactor;
			tempPos.z -= yd2 * sampleFactor;
		}
		else if(index == 7)
		{
			tempPos.x += xd2 * sampleFactor;
			tempPos.z -= yd1 * sampleFactor;
		}

		return tempPos;
	}

	void fudgeLightPosition(VC3 &pos, int index)
	{
		//static float radius = 0.05f;
		static const float radius = 0.075f;
		if(index == 1)
			pos += VC3(0, radius, 0);
		else if(index == 2)
			pos += VC3(0, -radius, 0);
		else if(index == 3)
			pos += VC3(radius, 0, 0);
		else if(index == 4)
			pos += VC3(-radius, 0, 0);
		else if(index == 5)
			pos += VC3(0, 0, radius);
		else if(index == 6)
			pos += VC3(0, 0, -radius);
	}

	void create(const LightList &lights, int area, int quality, const VC3 &sunDir)
	{
		reset();
		
		bool calculateSun = false;
		if(sunDir.GetSquareLength() > 0.001f)
			calculateSun = true;

		if(lights.empty() && !calculateSun)
			return;

		int xr = size.x / 2;
		int yr = size.y / 2;
		int xBlocks = storm.heightmapResolution.x / IStorm3D_Terrain::BLOCK_SIZE;
		int yBlocks = storm.heightmapResolution.y / IStorm3D_Terrain::BLOCK_SIZE;
		float xd = storm.heightmapSize.x / size.x;
		float yd = storm.heightmapSize.z / size.y;

		IStorm3D_TerrainRenderer &renderer = storm.terrain->getRenderer();
		renderer.enableFeature(IStorm3D_TerrainRenderer::LightmappedCollision, true);

		int sampleStart = SAMPLE_START;
		int sampleEnd = SAMPLE_END;
		if(quality == 1 || quality == 2 || quality == 3)
			sampleStart = SAMPLE_END - 1;

		float bias = .01f;
		int xstart = -xr;
		int xend = xr;
		int ystart = -yr;
		int yend = yr;

		VC3 sunRayDir = sunDir;
		if(calculateSun)
			sunRayDir.Normalize();

		if(area > 0)
		{
			VC3 camera3D = storm.scene->GetCamera()->GetPosition();
			int cameraX = int(camera3D.x * size.x / storm.heightmapSize.x);
			int cameraY = int(camera3D.z * size.y / storm.heightmapSize.z);
			
			//float radius3D = 20.f;
			//if(area == 2)
			//	radius3D = 10.f;

			float radius3D = 100.f;
			if(area == 2)
				radius3D = 50.f;
			else if(area == 3)
				radius3D = 20.f;
			else if(area == 4)
				radius3D = 10.f;
			else if(area == 5)
				radius3D = 1.f;

			int radiusX = int(radius3D * size.x / storm.heightmapSize.x);
			int radiusY = int(radius3D * size.y / storm.heightmapSize.z);

			xstart = cameraX - radiusX;
			xend = cameraX + radiusX;
			ystart = cameraY - radiusY;
			yend = cameraY + radiusY;

			xstart = max(-xr, xstart);
			ystart = max(-yr, ystart);
			xend = min(xr, xend);
			yend = min(yr, yend);
		}

		// Map values
		{
			COL lightColor;
			COL sunColor;

			int smoothSamples = 1;
			if(quality == 4)
				smoothSamples = 7;
			float smoothFactor = 1.f / smoothSamples;

			for(int y = ystart; y < yend; ++y)
			for(int x = xstart; x < xend; ++x)
			{
				int xp = x + xr;
				int yp = y + yr;

				VC3 pos(x * xd + (xd * .5f), 0, y * yd + (yd * .5f));
				pos.y = storm.terrain->getHeight(VC2(pos.x, pos.z));

				int valueIndex = yp * size.x + xp;
				Pixel &p = values[valueIndex];
				p.color = TColor<unsigned char> ();
				p.light = TColor<unsigned char> ();

				VC3 terrainNormal;
				{
					float txf = float(storm.heightmapResolution.x) * xp / size.x;
					float tyf = float(storm.heightmapResolution.y) * yp / size.y;
					int tx = int(txf);
					int ty = int(tyf);

					VC3 n1 = storm.terrain->getNormal(VC2I(tx, ty));
					VC3 n2 = storm.terrain->getNormal(VC2I(tx + 1, ty));
					VC3 n3 = storm.terrain->getNormal(VC2I(tx, ty + 1));
					VC3 n4 = storm.terrain->getNormal(VC2I(tx + 1, ty + 1));

					float f1 = txf - float(tx);
					float f2 = tyf - float(ty);
					float f3 = sqrtf(f1*f1 + f2*f2) / 1.415f;
					n1 *= 1.f - f3;
					n2 *= f1;
					n3 *= f2;
					n4 *= f3;

					terrainNormal = n2;
					terrainNormal += n3;
					if(f3 < .5f)
						terrainNormal += n1;
					else
						terrainNormal += n4;

					terrainNormal.Normalize();

					//terrainNormal = n1 + n2 + n3 + n4;
					//terrainNormal.Normalize();
				}

				bool updateLight = true;
				if(quality == 2 && valueIndex % 4 != 0)
					updateLight = false;

				if(updateLight)
				{
					lightColor = COL();
					sunColor = COL();

					LightList::const_iterator it = lights.begin();
					for(; it != lights.end(); ++it)
					{
						float rangeSq = pos.GetSquareRangeTo(it->position);
						float lightRange = it->range * MAGIC_FACTOR;
						if(rangeSq > lightRange * lightRange)
							continue;

						float visStrengthScale = 1.f - it->strength;
						float visStrengthAdd = it->strength;

						float visFactor = 0.f;
						{
							for(int i = sampleStart; i < sampleEnd; ++i)
							{
								VC3 tempPos = fudgePosition(pos, xd, yd, i);
								rangeSq = pos.GetSquareRangeTo(it->position);

								Storm3D_CollisionInfo info;

								for(int j = 0; j < smoothSamples; ++j)
								{
									VC3 lightPos = it->position;
									fudgeLightPosition(lightPos, j);

									VC3 vec = (lightPos - pos);
									lightPos = tempPos + vec;
									VC3 dir = (tempPos - lightPos).GetNormalized();
									if(quality != 3)
										storm.scene->RayTrace(lightPos, dir , lightRange + 1.f, info);

									if(!info.hit || info.range * info.range > rangeSq - bias)
										visFactor += 1.f / float(sampleEnd - sampleStart);
									else if(info.object)
									{
										IStorm3D_Material::ATYPE alpha = info.object->GetMesh()->GetMaterial()->GetAlphaType();
										if(alpha != IStorm3D_Material::ATYPE_NONE)
										{
											// First make sure there isnt any solid stuff blocking
											Storm3D_CollisionInfo info;
											info.onlySolidSurfaces = true;

											if(quality != 3)
												storm.scene->RayTrace(lightPos, dir, lightRange + 1.f, info);

											if(!info.hit || info.range * info.range > rangeSq - bias)
											{
												if(alpha != IStorm3D_Material::ATYPE_ADD)
													visFactor += .60f * 1.f / float(sampleEnd - sampleStart);
												else
													visFactor += 1.f / float(sampleEnd - sampleStart);
											}
										}
									}
								}
							}
						}

						visFactor *= smoothFactor;
						visFactor *= visStrengthScale;
						visFactor += visStrengthAdd;

						if(visFactor < 0.001f)
							continue;


						VC3 dir = (it->position - pos).GetNormalized();
						float dot = terrainNormal.GetDotWith(dir);
						if(dot < 0)
							continue;

						float fadeFactor1 = 1.f - sqrtf(rangeSq) / lightRange;
						float fadeFactor2 = 1.f - cosf(3.14f * 0.5f - sqrtf(rangeSq) / lightRange * PI * .5f);
						float fadeFactor = 0.5f * (fadeFactor1 + fadeFactor2);

						COL c = it->color;
						c *= fadeFactor * dot * visFactor;
						//c *= fadeFactor * visFactor;

						lightColor += c;
					}

					if(calculateSun)
					{
						float visFactor = 0.f;
						{
							for(int i = sampleStart; i < sampleEnd; ++i)
							{
								VC3 tempPos = fudgePosition(pos, xd, yd, i);

								Storm3D_CollisionInfo info;
								ObstacleCollisionInfo oinfo;

								if(quality != 3)
									storm.scene->RayTrace(tempPos, sunRayDir, 100.f, info);
								tempPos += terrainNormal * .5f;
								if(quality != 3)
									storm.terrain->rayTrace(tempPos, sunRayDir, 100.f, info, oinfo, true);

								if(!info.hit)
									visFactor += 1.f / float(sampleEnd - sampleStart);
								else if(info.object)
								{
									IStorm3D_Material::ATYPE alpha = info.object->GetMesh()->GetMaterial()->GetAlphaType();
									if(alpha != IStorm3D_Material::ATYPE_NONE)
									{
										// First make sure there isnt any solid stuff blocking
										Storm3D_CollisionInfo info;
										info.onlySolidSurfaces = true;

										if(quality != 3)
											storm.scene->RayTrace(it->position, sunRayDir, it->range + 1.f, info);

										if(!info.hit)
										{
											if(alpha != IStorm3D_Material::ATYPE_ADD)
												visFactor += .60f * 1.f / float(sampleEnd - sampleStart);
											else
												visFactor += 1.f / float(sampleEnd - sampleStart);
										}
									}
								}
							}

							visFactor *= .5f;
							visFactor += .5f;
						}

						float dot = terrainNormal.GetDotWith(sunDir);
						if(dot > 0)
						{
							COL c(1.f, 1.f, 1.f);
							c *= dot * visFactor;

							//p.addSunColor(c);
							sunColor += c;
						}
					}
				}

				p.addColor(lightColor);
				p.addSunColor(sunColor);
			}
		}
/*
		// Temp
		{
			ofstream lightStream("lightmap_l.raw", ios::binary);
			ofstream colorStream("lightmap_c.raw", ios::binary);

			for(int y = 0; y < size.y; ++y)
			for(int x = 0; x < size.x; ++x)
			{
				Pixel &pixel = values[y * size.x + x];
				
				colorStream << pixel.color.r << pixel.color.g << pixel.color.b;
				lightStream << pixel.light.r << pixel.light.g << pixel.light.b;
			}
		}
*/

		renderer.enableFeature(IStorm3D_TerrainRenderer::LightmappedCollision, false);
		apply();
	}

	void apply()
	{
		if(!storm.terrain)
			return;

		int xBlocks = storm.heightmapResolution.x / IStorm3D_Terrain::BLOCK_SIZE;
		int yBlocks = storm.heightmapResolution.y / IStorm3D_Terrain::BLOCK_SIZE;
		int xSize = IStorm3D_Terrain::BLOCK_SIZE * TEXTURE_SIZE_FACTOR;
		int ySize = IStorm3D_Terrain::BLOCK_SIZE * TEXTURE_SIZE_FACTOR;
		vector<DWORD> buffer(xSize * ySize);

		for(int yb = 0; yb < yBlocks; ++yb)
		for(int xb = 0; xb < xBlocks; ++xb)
		{
			int block = yb * xBlocks + xb;

			if(!values.empty())
			{
				for(int yt = 0; yt < ySize; ++yt)
				for(int xt = 0; xt < xSize; ++xt)
				{
					int xIndex = xt + (xb * (xSize));
					int yIndex = yt + (yb * (ySize));
					if(xIndex >= size.x - 1)
						xIndex = size.x - 2;
					if(xIndex <= 0)
						xIndex = 1;
					if(yIndex >= size.y - 1)
						yIndex = size.y - 2;
					if(yIndex <= 0)
						yIndex = 1;

					int index = yIndex * size.x + xIndex;
					const TColor<unsigned char> &c = values[index].light;
					const TColor<unsigned char> &c1 = values[index - 1].light;
					const TColor<unsigned char> &c2 = values[index + 1].light;
					const TColor<unsigned char> &c3 = values[index - size.x].light;
					const TColor<unsigned char> &c4 = values[index + size.x].light;
					const TColor<unsigned char> &c5 = values[index - 1 - size.x].light;
					const TColor<unsigned char> &c6 = values[index + 1 + size.x].light;
					const TColor<unsigned char> &c7 = values[index + 1 - size.x].light;
					const TColor<unsigned char> &c8 = values[index - 1 + size.x].light;

					int r = (3 * c.r / 4) + (1 * (c1.r + c2.r + c3.r + c4.r + c5.r + c6.r + c7.r + c8.r) / 32);
					int g = (3 * c.g / 4) + (1 * (c1.g + c2.g + c3.g + c4.g + c5.g + c6.g + c7.g + c8.g) / 32);
					int b = (3 * c.b / 4) + (1 * (c1.b + c2.b + c3.b + c4.b + c5.b + c6.b + c7.b + c8.b) / 32);
					//int r = c.r;
					//int g = c.g;
					//int b = c.b;

					buffer[yt * xSize + xt] = (r << 16) | (g << 8) | (b);
				}
			}

			shared_ptr<IStorm3D_Texture> t = createTexture(xSize, ySize, storm);
			t->Copy32BitSysMembufferToTexture(&buffer[0]);

			storm.terrain->setLightMap(block, *t);
		}
	}

	TColor<unsigned char> getColor(const VC2 &position) const
	{
		if(!storm.terrain || values.empty())
			return TColor<unsigned char> ();

		const VC3 &hSize = storm.heightmapSize;
		VC2 pos(hSize.x * .5f, hSize.z * .5f);
		pos += position;
		pos.x /= hSize.x;
		pos.y /= hSize.z;

		if(pos.x <= 0 || pos.x >= 1.f || pos.y <= 0 || pos.y >= 1.f)
			return TColor<unsigned char> ();

		int x = int(pos.x * size.x);
		int y = int(pos.y * size.y);

		const Pixel &pixel = values[y * size.x + x];

		if(pixel.color.r || pixel.light.r)
			int a = 0;

		return pixel.color;
	}

	void setShadow(const VC2 &position, float value, const VC2 &rect, const LightList &lights, const VC3 &sunDir)
	{
		if(!storm.terrain || values.empty())
			return;

		const VC3 &hSize = storm.heightmapSize;
		VC2 pos(hSize.x * .5f, hSize.z * .5f);
		pos += position;
		pos.x /= hSize.x;
		pos.y /= hSize.z;

		int radius_x = int(rect.x / (float)hSize.x);
		int radius_y = int(rect.y / (float)hSize.y);

		float vis_factor = 255.0f;

		float merge_mul = value;
		if(merge_mul < 0.0f)
		{
			vis_factor = 0.0f;
			merge_mul = -value;
		}

		bool calculateSun = false;
		if(sunDir.GetSquareLength() > 0.001f)
			calculateSun = true;

		int start_x = int(pos.x * size.x) - radius_x;
		if(start_x < 0)
			start_x = 0;
		else if(start_x >= size.x)
			start_x = size.x - 1;

		int start_y = int(pos.y * size.y) - radius_y;
		if(start_y < 0)
			start_y = 0;
		else if(start_y >= size.y)
			start_y = size.y - 1;

		int end_x = int(pos.x * size.x) + radius_x;
		if(end_x < 0)
			end_x = 0;
		else if(end_x >= size.x)
			end_x = size.x - 1;

		int end_y = int(pos.y * size.y) + radius_y;
		if(end_y < 0)
			end_y = 0;
		else if(end_y >= size.y)
			end_y = size.y - 1;

		int center_x = int(pos.x * size.x);
		int center_y = int(pos.y * size.y);
		float fradius_x = 0.5f * (float)(radius_x * radius_x);
		float fradius_y = 0.5f * (float)(radius_y * radius_y);

		float xd = storm.heightmapSize.x / size.x;
		float yd = storm.heightmapSize.z / size.y;
		int xr = size.x / 2;
		int yr = size.y / 2;

		for(int y = start_y; y < end_y; y++)
		for(int x = start_x; x < end_x; x++)
		{
			int dx = (x - center_x);
			int dy = (y - center_y);
			float merge_factor = merge_mul * (1.0f - sqrtf((float)(dx*dx / fradius_x + dy*dy / fradius_y)));
			if(merge_factor < 0.0f)
				merge_factor = 0.0f;
			else if(merge_factor > 1.0f)
				merge_factor = 1.0f;

			VC3 terrainNormal;
			{
				float txf = float(storm.heightmapResolution.x) * x / size.x;
				float tyf = float(storm.heightmapResolution.y) * y / size.y;
				int tx = int(txf);
				int ty = int(tyf);

				VC3 n1 = storm.terrain->getNormal(VC2I(tx, ty));
				VC3 n2 = storm.terrain->getNormal(VC2I(tx + 1, ty));
				VC3 n3 = storm.terrain->getNormal(VC2I(tx, ty + 1));
				VC3 n4 = storm.terrain->getNormal(VC2I(tx + 1, ty + 1));

				float f1 = txf - float(tx);
				float f2 = tyf - float(ty);
				float f3 = sqrtf(f1*f1 + f2*f2) / 1.415f;
				n1 *= 1.f - f3;
				n2 *= f1;
				n3 *= f2;
				n4 *= f3;

				terrainNormal = n2;
				terrainNormal += n3;
				if(f3 < .5f)
					terrainNormal += n1;
				else
					terrainNormal += n4;

				terrainNormal.Normalize();

				//terrainNormal = n1 + n2 + n3 + n4;
				//terrainNormal.Normalize();
			}

			COL lightColor;
			{
				int xp = x - xr;
				int yp = y - yr;
				VC3 pos(xp * xd + (xd * .5f), 0, yp * yd + (yd * .5f));
				pos.y = storm.terrain->getHeight(VC2(pos.x, pos.z));
				LightList::const_iterator it = lights.begin();
				for(; it != lights.end(); ++it)
				{
					float rangeSq = pos.GetSquareRangeTo(it->position);
					float lightRange = it->range * MAGIC_FACTOR;
					if(rangeSq > lightRange * lightRange)
					{
						continue;
					}

					float visStrengthScale = 1.f - it->strength;
					float visStrengthAdd = it->strength;

					float visFactor = vis_factor / 255.0f;
					visFactor *= visStrengthScale;
					visFactor += visStrengthAdd;

					if(visFactor < 0.001f)
						continue;


					VC3 dir = (it->position - pos).GetNormalized();
					float dot = terrainNormal.GetDotWith(dir);
					if(dot < 0)
					{
						continue;
					}

					float fadeFactor1 = 1.f - sqrtf(rangeSq) / lightRange;
					float fadeFactor2 = 1.f - cosf(3.14f * 0.5f - sqrtf(rangeSq) / lightRange * PI * .5f);
					float fadeFactor = 0.5f * (fadeFactor1 + fadeFactor2);

					COL c = it->color;
					c *= fadeFactor * dot * visFactor;
					//c *= fadeFactor * visFactor;

					lightColor += c;
				}
			}

			lightColor *= 255.0f;
			if(lightColor.r > 255.0f) lightColor.r = 255.0f;
			else if(lightColor.r < 0.0f) lightColor.r = 0.0f;
			if(lightColor.g > 255.0f) lightColor.g = 255.0f;
			else if(lightColor.g < 0.0f) lightColor.g = 0.0f;
			if(lightColor.b > 255.0f) lightColor.b = 255.0f;
			else if(lightColor.b < 0.0f) lightColor.b = 0.0f;

			Pixel &pixel = values[y * size.x + x];

			float old_light_r = pixel.light.r;
			float old_light_g = pixel.light.g;
			float old_light_b = pixel.light.b;

			pixel.light.r = pixel.color.r = (int)(lightColor.r * merge_factor + pixel.color.r * (1.0f - merge_factor));
			pixel.light.g = pixel.color.g = (int)(lightColor.g * merge_factor + pixel.color.g * (1.0f - merge_factor));
			pixel.light.b = pixel.color.b = (int)(lightColor.b * merge_factor + pixel.color.b * (1.0f - merge_factor));

			float sun_light = 0.0f;
			if(calculateSun)
			{
				sun_light = vis_factor * 0.5f + 127.0f;
				float dot = terrainNormal.GetDotWith(sunDir);
				if(dot > 0)
				{
					sun_light *= dot;
				}
				else
				{
					sun_light = 0;
				}
			}
			pixel.light.r = (int)((lightColor.r + sun_light) * merge_factor + old_light_r * (1.0f - merge_factor));
			pixel.light.g = (int)((lightColor.g + sun_light) * merge_factor + old_light_g * (1.0f - merge_factor));
			pixel.light.b = (int)((lightColor.b + sun_light) * merge_factor + old_light_b * (1.0f - merge_factor));
		}
	}

	void doExport(Exporter &exporter) const
	{
		if(!storm.terrain || values.empty())
			return;

		ExporterScene &scene = exporter.getScene();
		scene.setLightmapSize(IStorm3D_Terrain::BLOCK_SIZE * TEXTURE_SIZE_FACTOR);

		{
			int xBlocks = storm.heightmapResolution.x / IStorm3D_Terrain::BLOCK_SIZE;
			int yBlocks = storm.heightmapResolution.y / IStorm3D_Terrain::BLOCK_SIZE;
			int xSize = IStorm3D_Terrain::BLOCK_SIZE * TEXTURE_SIZE_FACTOR;
			int ySize = IStorm3D_Terrain::BLOCK_SIZE * TEXTURE_SIZE_FACTOR;
			vector<unsigned char> buffer(xSize * ySize * 3);

			bool firstSample = true;
			unsigned char firstR = 0, firstG = 0, firstB = 0;

			for(int yb = 0; yb < yBlocks; ++yb)
			for(int xb = 0; xb < xBlocks; ++xb)
			{
				int block = yb * xBlocks + xb;
				bool hasData = false;

				for(int yt = 0; yt < ySize; ++yt)
				for(int xt = 0; xt < xSize; ++xt)
				{
					//int xIndex = xt + (xb * (xSize - 1));
					//int yIndex = yt + (yb * (ySize - 1));
					int xIndex = xt + (xb * xSize);
					int yIndex = yt + (yb * ySize);
					if(xIndex >= size.x - 1)
						xIndex = size.x - 2;
					if(xIndex <= 0)
						xIndex = 1;
					if(yIndex >= size.y - 1)
						yIndex = size.y - 2;
					if(yIndex <= 0)
						yIndex = 1;

					int index = yIndex * size.x + xIndex;
					const TColor<unsigned char> &c = values[index].light;
					const TColor<unsigned char> &c1 = values[index - 1].light;
					const TColor<unsigned char> &c2 = values[index + 1].light;
					const TColor<unsigned char> &c3 = values[index - size.x].light;
					const TColor<unsigned char> &c4 = values[index + size.x].light;
					const TColor<unsigned char> &c5 = values[index - 1 - size.x].light;
					const TColor<unsigned char> &c6 = values[index + 1 + size.x].light;
					const TColor<unsigned char> &c7 = values[index + 1 - size.x].light;
					const TColor<unsigned char> &c8 = values[index - 1 + size.x].light;

					int r = (3 * c.r / 4) + (1 * (c1.r + c2.r + c3.r + c4.r + c5.r + c6.r + c7.r + c8.r) / 32);
					int g = (3 * c.g / 4) + (1 * (c1.g + c2.g + c3.g + c4.g + c5.g + c6.g + c7.g + c8.g) / 32);
					int b = (3 * c.b / 4) + (1 * (c1.b + c2.b + c3.b + c4.b + c5.b + c6.b + c7.b + c8.b) / 32);
					//int r = c.r;
					//int g = c.g;
					//int b = c.b;

					buffer[(yt * xSize + xt) * 3] = r;
					buffer[(yt * xSize + xt) * 3 + 1] = g;
					buffer[(yt * xSize + xt) * 3 + 2] = b;

					if(r || g || b)
					{
						if (firstSample)
						{
							firstSample = false;
							firstR = r;
							firstG = g;
							firstB = b;
						}
						if (r != firstR || g != firstG || b != firstB)
						{
							hasData = true;
						}
					}
				}

				if(hasData)
					scene.setBlockLightmap(block, buffer);
			}
		}

		{
			VC2I colorSize = storm.heightmapResolution;
			colorSize *= 4;

			vector<TColor<unsigned char> > map(colorSize.x * colorSize.y);
			for(int y = 0; y < colorSize.y; ++y)
			for(int x = 0; x < colorSize.x; ++x)
			{
				int nx = x * size.x / colorSize.x;
				int ny = y * size.y / colorSize.y;

				map[y * colorSize.x + x] = values[ny * size.x + nx].color;
			}

			scene.addColorMap(colorSize, map);
		}
	}

	void debugRender()
	{
		//if(size.x == 0 || size.y == 0)
			return;

		if(!t || !m)
		{
			t = storm.storm->CreateNewTexture(size.x, size.y, IStorm3D_Texture::TEXTYPE_BASIC);
			m = storm.storm->CreateNewMaterial("..");
			m->SetBaseTexture(t);
		}

		static int i = 0;
		static int j = 0;
		if(++i % 20 == 0)
		{
			++j;

			scoped_array<DWORD> buffer(new DWORD[size.x * size.y]);
			for(int y = 0; y < size.y; ++y)
			for(int x = 0; x < size.x; ++x)
			{
				int index = y * size.x + x;
				Pixel &pixel = values[index];

				TColor<unsigned char> col = pixel.color;
				if(j % 2)
					col = pixel.light;

				DWORD value = (col.r << 16) | (col.g << 8) | (col.b);
				buffer[index] = value;
			}

			t->Copy32BitSysMembufferToTexture(buffer.get());
		}

		storm.scene->Render2D_Picture(m, VC2(1,1), VC2(512, 512));
	}

	void write(filesystem::OutputStream &stream) const
	{
		// version 2 is old unpacked lightmap, version 3 is new RLE packed lightmap
		// since the RLE packing has not really been tested to work with actual lightmaps that contain something,
		// enabled only for AOV at the moment.
#ifdef PROJECT_AOV
		int writeVersion = 3;
#else
		int writeVersion = 2;
#endif

		stream << int(writeVersion);
		stream << size.x << size.y;

		if (writeVersion == 3)
		{
			// new, slightly compacted version, specifically for cases where lightmap is not used (all single color)

			int unpackedSize = size.x * size.y * 6;
			// some extra allocation just to be safe
			unsigned char *packedBuf = new unsigned char[unpackedSize + 16];
			unsigned char *unpackedBuf = new unsigned char[unpackedSize + 16];

			int blockSize = size.x * size.y;
			for(int i = 0; i < unpackedSize / 6; ++i)
			{
				const Pixel &p = values[i];

				// notice, this is in non-interlaced order to allow proper compression in case of colored light fill
				unpackedBuf[i+blockSize*0] = p.color.r;
				unpackedBuf[i+blockSize*1] = p.color.g;
				unpackedBuf[i+blockSize*2] = p.color.b;
				unpackedBuf[i+blockSize*3] = p.light.r;
				unpackedBuf[i+blockSize*4] = p.light.g;
				unpackedBuf[i+blockSize*5] = p.light.b;

				// (this would be interlaced)
				//unpackedBuf[i*6+0] = p.color.r;
				//unpackedBuf[i*6+1] = p.color.g;
				//unpackedBuf[i*6+2] = p.color.b;
				//unpackedBuf[i*6+3] = p.light.r;
				//unpackedBuf[i*6+4] = p.light.g;
				//unpackedBuf[i*6+5] = p.light.b;
			}

			jpak_set_bits(8);
			int packedSize = jpak_pack(unpackedSize, unpackedBuf, packedBuf);

			int isPacked = 1;
			if (packedSize == 0)
			{
				isPacked = 0;
				packedSize = unpackedSize;
				memcpy(packedBuf, unpackedBuf, packedSize);
			}

			stream << isPacked;
			stream << packedSize;
			stream << unpackedSize;

			for(int i = 0; i < packedSize; ++i)
			{
				stream << packedBuf[i];
			}
			delete [] unpackedBuf;
			delete [] packedBuf;
		} else {
			// old, uncompressed.
			assert(writeVersion == 2);

			for(int i = 0; i < size.x * size.y; ++i)
			{
				const Pixel &p = values[i];
				stream << p.color.r << p.color.g << p.color.b;
				stream << p.light.r << p.light.g << p.light.b;
			}
		}
	}

	void read(filesystem::InputStream &stream)
	{
		reset();

		int version = 0;
		stream >> version;

		stream >> size.x >> size.y;
		values.resize(size.x * size.y);

		if(version >= 3)
		{
			// new, slightly compacted version, specifically for cases where lightmap is not used (all single color)

			// TODO: could optimize, if assumed that vector really contains an array 
			// then using &values[0] instead of the temporary unpackedBuffer

			int isPacked;
			int packedSize;
			int unpackedSize;
			stream >> isPacked;
			stream >> packedSize;
			stream >> unpackedSize;
			assert(unpackedSize == size.x * size.y * 6);
			// some extra allocation just to be safe
			unsigned char *packedBuf = new unsigned char[packedSize + 16];
			unsigned char *unpackedBuf = new unsigned char[unpackedSize + 16];
			for(int i = 0; i < packedSize; ++i)
			{
				stream >> packedBuf[i];
			}
			if (isPacked != 0)
			{
				jpak_set_bits(8);
				int resultSize = jpak_unpack(packedSize, packedBuf, unpackedBuf);
				assert(resultSize == unpackedSize);
			} else {
				assert(unpackedSize == packedSize);
				memcpy(unpackedBuf, packedBuf, packedSize);
			}
			int blockSize = size.x * size.y;
			for(int i = 0; i < unpackedSize / 6; ++i)
			{
				Pixel &p = values[i];

				// notice, this is in non-interlaced order to allow proper compression in case of colored light fill
				p.color.r = unpackedBuf[i+blockSize*0];
				p.color.g = unpackedBuf[i+blockSize*1];
				p.color.b = unpackedBuf[i+blockSize*2];
				p.light.r = unpackedBuf[i+blockSize*3];
				p.light.g = unpackedBuf[i+blockSize*4];
				p.light.b = unpackedBuf[i+blockSize*5];

				// (this would be interlaced)
				//p.color.r = unpackedBuf[i*6+0];
				//p.color.g = unpackedBuf[i*6+1];
				//p.color.b = unpackedBuf[i*6+2];
				//p.light.r = unpackedBuf[i*6+3];
				//p.light.g = unpackedBuf[i*6+4];
				//p.light.b = unpackedBuf[i*6+5];
			}
			delete [] unpackedBuf;
			delete [] packedBuf;
		} 
		else 
		{
			if(version >= 2)
			{
				assert(sizeof(Pixel) == 6);
				stream.read(&values[0].color.r, size.x * size.y * 6);
			}
			else
			{
				assert(sizeof(Pixel) == 3);
				stream.read(&values[0].color.r, size.x * size.y * 3);
			}

			/*
			for(int i = 0; i < size.x * size.y; ++i)
			{
				Pixel &p = values[i];
				stream >> p.color.r >> p.color.g >> p.color.b;
				
				if(version >= 2)
					stream >> p.light.r >> p.light.g >> p.light.b;
			}
			*/
		}
	}
};

TerrainLightMap::TerrainLightMap(Storm &storm)
{
	scoped_ptr<Data> tempData(new Data(storm));
	data.swap(tempData);
}

TerrainLightMap::~TerrainLightMap()
{
}

void TerrainLightMap::reset()
{
	data->reset();
}

void TerrainLightMap::create(const LightList &lights, int area, int quality, const VC3 &sunDir)
{
	data->create(lights, area, quality, sunDir);
}

void TerrainLightMap::debugRender()
{
	data->debugRender();
}

void TerrainLightMap::apply()
{
	data->apply();
}

COL TerrainLightMap::getColor(const VC2 &position) const
{
	TColor<unsigned char> color = data->getColor(position);

	float factor = .8f;
	return COL(factor * color.r / 255.f, factor * color.g / 255.f, factor * color.b / 255.f);
}

void TerrainLightMap::setShadow(const VC2 &position, float value, const VC2 &rect, const LightList &lights, const VC3 &sunDir)
{
	data->setShadow(position, value, rect, lights, sunDir);
}


void TerrainLightMap::doExport(Exporter &exporter) const
{
	data->doExport(exporter);
}

filesystem::OutputStream &TerrainLightMap::writeStream(filesystem::OutputStream &stream) const
{
	data->write(stream);
	return stream;
}

filesystem::InputStream &TerrainLightMap::readStream(filesystem::InputStream &stream)
{
	data->read(stream);
	return stream;
}

} // editor
} // frozenbyte
