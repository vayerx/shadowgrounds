// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "height_modifier.h"

namespace frozenbyte {
namespace editor {

struct HeightModifier::Data
{
	unsigned short *buffer;
	const VC2I &resolution;
	const VC3 &size;

	Data(unsigned short *buffer_, const VC2I &resolution_, const VC3 &size_)
	:	buffer(buffer_),
		resolution(resolution_),
		size(size_)
	{
	}

	VC2I getCenter(const VC3 &position) const
	{
		VC2I result;

		result.x = resolution.x / 2;
		result.y = resolution.y / 2;

		//result.x += int(position.x * resolution.x / size.x - .5f);
		//result.y += int(position.z * resolution.y / size.z + .5f);
		//result.x += int((position.x * resolution.x / size.x) - (1.f * resolution.x / size.x));
		//result.y += int((position.z * resolution.y / size.z) + (1.f * resolution.x / size.x));

		float fx = position.x * resolution.x / size.x;
		float fy = position.z * resolution.y / size.z;
		if(fx > 0)
			fx += .5f;
		if(fx < 0)
			fx -= .5f;
		if(fy > 0)
			fy += .5f;
		if(fy < 0)
			fy -= .5f;

		result.x += int(fx);
		result.y += int(fy);
		return result;
	}

	VC2I getRadius(float radius) const
	{
		VC2I result;

		result.x = int(radius * resolution.x / size.x);
		result.y = int(radius * resolution.y / size.z);

		return result;
	}

	VC2 heightmapToWorld(const VC2I &position) const
	{
		VC2 result;

		result.x = (position.x * size.x / resolution.x) - size.x/2;
		result.y = (position.y * size.z / resolution.y) - size.z/2;

		return result;
	}

	float getFade(const VC2I &center, const VC2I &pixel, float radius, HeightModifier::Shape shape) const
	{
		VC2 worldCenter = heightmapToWorld(center);
		VC2 worldPixel = heightmapToWorld(pixel);

		if(shape == HeightModifier::Circle)
		{
			float range = worldCenter.GetRangeTo(worldPixel);
			if(range > radius)
				return 0;

			float result = 1.f - range / radius;
			return result;
		}

		return 1.f;
	}

	void changeHeight(const VC3 &position, float worldRadius, float strength, HeightModifier::Shape shape)
	{
		VC2I center = getCenter(position);
		VC2I radius = getRadius(worldRadius);
		int delta = int(strength * 200.f);

		for(int y = -radius.y; y <= radius.y; ++y)
		for(int x = -radius.x; x <= radius.x; ++x)
		{
			int xx = x + center.x;
			int yy = y + center.y;

			if(xx < 0 || xx >= resolution.x)
				continue;
			if(yy < 0 || yy >= resolution.y)
				continue;

			int value = buffer[yy * resolution.x + xx];
			float fade = getFade(center, VC2I(xx, yy), worldRadius, shape);
			value += int(fade * delta);

			if(value > 65500)
				value = 65500;
			if(value < 0)
				value = 0;

			buffer[yy * resolution.x + xx] = value;
		}
	}

	void flatten(const VC3 &position, float worldRadius, float strength, float height, HeightModifier::Shape shape)
	{
		VC2I center = getCenter(position);
		VC2I radius = getRadius(worldRadius);
		int delta = int(fabsf(strength) * 200.f);

		int newHeight = int(height / size.y * 65535);

		for(int y = -radius.y; y <= radius.y; ++y)
		for(int x = -radius.x; x <= radius.x; ++x)
		{
			int xx = x + center.x;
			int yy = y + center.y;

			if(xx < 0 || xx >= resolution.x)
				continue;
			if(yy < 0 || yy >= resolution.y)
				continue;

			int value = buffer[yy * resolution.x + xx];
			float fade = getFade(center, VC2I(xx, yy), worldRadius, shape);

			int diff = newHeight - value;
			int maxDiff = int(fade * delta);

			if(abs(diff) < maxDiff)
				value = newHeight;
			else
			{
				if(diff > 0)
					value += maxDiff;
				else
					value -= maxDiff;
			}

			buffer[yy * resolution.x + xx] = value;
		}
	}

	void smoothen(const VC3 &position, float worldRadius, float strength, HeightModifier::Shape shape)
	{
		VC2I center = getCenter(position);
		VC2I radius = getRadius(worldRadius);

		// x-axis blur
		for(int y = -radius.y; y <= radius.y; ++y)
		for(int x = -radius.x; x <= radius.x - 1; ++x)
		{
			// neighbour height
			int neighbour;
			{
				int xx = x + 1 + center.x;
				int yy = y + center.y;

				if(xx < 0 || xx >= resolution.x)
					continue;
				if(yy < 0 || yy >= resolution.y)
					continue;

				neighbour = buffer[yy * resolution.x + xx];
			}

			// current height
			int xx = x + center.x;
			int yy = y + center.y;

			if(xx < 0 || xx >= resolution.x)
				continue;
			if(yy < 0 || yy >= resolution.y)
				continue;

			int value = buffer[yy * resolution.x + xx];


			// move towards average
			float fade = getFade(center, VC2I(xx, yy), worldRadius, shape);
			int average = (neighbour + value) / 2;
			value += int(fade * strength * 0.1f * (average - value));

			if(value > 65500)
				value = 65500;
			if(value < 0)
				value = 0;

			buffer[yy * resolution.x + xx] = value;
		}

		// y-axis blur
		for(int y = -radius.y; y <= radius.y - 1; ++y)
		for(int x = -radius.x; x <= radius.x; ++x)
		{
			// neighbour height
			int neighbour;
			{
				int xx = x + center.x;
				int yy = y + 1 + center.y;

				if(xx < 0 || xx >= resolution.x)
					continue;
				if(yy < 0 || yy >= resolution.y)
					continue;

				neighbour = buffer[yy * resolution.x + xx];
			}

			// current height
			int xx = x + center.x;
			int yy = y + center.y;

			if(xx < 0 || xx >= resolution.x)
				continue;
			if(yy < 0 || yy >= resolution.y)
				continue;

			int value = buffer[yy * resolution.x + xx];


			// move towards average
			float fade = getFade(center, VC2I(xx, yy), worldRadius, shape);
			int average = (neighbour + value) / 2;
			value += int(fade * strength * 0.1f * (average - value));

			if(value > 65500)
				value = 65500;
			if(value < 0)
				value = 0;

			buffer[yy * resolution.x + xx] = value;
		}
	}
};

HeightModifier::HeightModifier(unsigned short *buffer, const VC2I &resolution, const VC3 &size)
:	data(new Data(buffer, resolution, size))
{
}

HeightModifier::~HeightModifier()
{
}

void HeightModifier::changeHeight(const VC3 &position, float radius, float strength, Shape shape)
{
	data->changeHeight(position, radius, strength, shape);
}

void HeightModifier::flatten(const VC3 &position, float radius, float strength, float height, Shape shape)
{
	data->flatten(position, radius, strength, height, shape);
}

void HeightModifier::smoothen(const VC3 &position, float radius, float strength, Shape shape)
{
	data->smoothen(position, radius, strength, shape);
}

} // editor
} // frozenbyte
