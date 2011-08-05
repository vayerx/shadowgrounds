// Copyright 2003-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_QTREE_STATIC_H
#define INCLUDED_C2_QTREE_STATIC_H

#pragma once
#include "c2_sphere.h"
#include "c2_ray.h"
#include "c2_aabb.h"
#include "c2_frustum.h"
#include "c2_vectors.h"
#include "c2_collisioninfo.h"
#include "c2_collision.h"
#include <vector>
#include <stack>
#include <functional>
#include <algorithm>
#include <cassert>

#include "../../util/Debug_MemoryManager.h"

// Loose quadtree, extended to 3d space
// Static, stores instances directly

namespace {
	static const float STATIC_LOOSE_FACTOR = 1.5;
	static const float STATIC_MIN_SIZE = 1.0f;
} // unnamed

template<class T>
struct StaticQuadtreeNode;
template<class T>
class StaticQuadtree;
template<class T>
class StaticQuadtreeRaytraceCollision;
template<class T>
class StaticQuadtreeSphereCollision;

template<class T>
struct StaticQuadtreeNode
{
	AABB area;
	AABB looseArea;
	VC3 position;

	StaticQuadtreeNode<T> *childs[4];
	StaticQuadtreeNode<T> *root;

	std::vector<T> entities;
	float data;

	StaticQuadtreeNode(const AABB &area_, const AABB &looseArea_, StaticQuadtreeNode<T> *root_)
	:	area(area_),
		looseArea(looseArea_),
		root(root_)
	{
		position = (area.mmin + area.mmax) * .5f;
		
		for(int i = 0; i < 4; ++i)
			childs[i] = 0;
	}

	~StaticQuadtreeNode()
	{
		for(int i = 0; i < 4; ++i)
			delete childs[i];
	}

	void insert( T &instance, const VC3 &position, float radius, float heightMin, float heightMax)
	{
		Sphere sphere(position, radius);
		insert(instance, sphere, heightMin, heightMax);
	}

	static void calculateLooseArea(const AABB &area, AABB &looseArea)
	{
		float xDelta = (area.mmax.x - area.mmin.x) * STATIC_LOOSE_FACTOR;
		float zDelta = (area.mmax.z - area.mmin.z) * STATIC_LOOSE_FACTOR;

		looseArea.mmin.x = area.mmax.x - xDelta;
		looseArea.mmin.y = area.mmin.y;
		looseArea.mmin.z = area.mmax.z - zDelta;

		looseArea.mmax.x = area.mmin.x + xDelta;
		looseArea.mmax.y = area.mmax.y;
		looseArea.mmax.z = area.mmin.z + zDelta;
	}

	void insert(T &instance, const Sphere &sphere, float heightMin, float heightMax)
	{
		/*
		// Update heights
		if(sphere.position.y + sphere.radius > area.mmax.y)
		{
			area.mmax.y = sphere.position.y + sphere.radius;
			looseArea.mmax.y = area.mmax.y + .01f;
		}
		if(sphere.position.y - sphere.radius < area.mmin.y)
		{
			area.mmin.y = sphere.position.y - sphere.radius;
			looseArea.mmin.y = area.mmin.y - .01f;
		}
		*/
		if(heightMax > area.mmax.y)
		{
			area.mmax.y = heightMax;
			looseArea.mmax.y = heightMax + .01f;
		}
		if(heightMin < area.mmin.y)
		{
			area.mmin.y = heightMin;
			looseArea.mmin.y = heightMin - .01f;
		}

		if(area.mmax.x - area.mmin.x > STATIC_MIN_SIZE)
		if(area.mmax.z - area.mmin.z > STATIC_MIN_SIZE)
		{
			AABB area[4];
			AABB looseArea[4];
			
			// Fits to normal area?

			StaticQuadtreeNode<T> *realRoot = root;
			if(!realRoot)
				realRoot = this;

			for(int i = 0; i < 4; ++i)
			{
				getChildArea(i, area[i]);
				calculateLooseArea(area[i], looseArea[i]);

				if(looseArea[i].mmax.x - looseArea[i].mmin.x < STATIC_MIN_SIZE)
					continue;
				if(looseArea[i].mmax.y - looseArea[i].mmin.y < STATIC_MIN_SIZE)
					continue;

				if(instance.fits(area[i]))
				{
					if(!childs[i])
						childs[i] = new StaticQuadtreeNode<T>(area[i], looseArea[i], realRoot);

					childs[i]->insert(instance, sphere, heightMin, heightMax);
					return;
				}
			}

			// Fits to loose area?

			for(int j = 0; j < 4; ++j)
			{
				if(instance.fits(area[j]))
				{
					if(!childs[j])
						childs[j] = new StaticQuadtreeNode<T>(area[j], looseArea[j], realRoot);

					childs[j]->insert(instance, sphere, heightMin, heightMax);
					return;
				}
			}
		}

		/*
		Entity entity;
		entity.sphere = sphere;
		entity.instance = &instance;

		entities.push_back(entity);
		*/
		entities.push_back(instance);
	}

	void getChildArea(int index, AABB &result) const
	{
		assert(index >= 0 && index < 4);

		float minX = area.mmin.x;
		float minZ = area.mmin.z;
		float maxX = area.mmax.x;
		float maxZ = area.mmax.z;
		float xDelta = maxX - minX;
		float zDelta = maxZ - minZ;

		if(index == 0 || index == 1)
		{
			result.mmin.x = minX;
			result.mmax.x = minX + xDelta * .5f;
		}
		else
		{
			result.mmin.x = minX + xDelta * .5f;
			result.mmax.x = maxX;
		}

		if(index == 1 || index == 3)
		{
			result.mmin.z = minZ;
			result.mmax.z = minZ + zDelta * .5f;
		}
		else
		{
			result.mmin.z = minZ + zDelta * .5f;
			result.mmax.z = maxZ;
		}

		result.mmin.y = area.mmin.y;
		result.mmax.y = area.mmax.y;

	}

	friend class StaticQuadtree<T>;
	friend class StaticQuadtreeRaytraceCollision<T>;
	friend class StaticQuadtreeSphereCollision<T>;
};

template<class T>
class StaticQuadtreeRaytraceCollision
{
	Ray ray;
	Storm3D_CollisionInfo &info;
	bool accurate;

	struct RaySorter: public std::binary_function<StaticQuadtreeNode<T> *, StaticQuadtreeNode<T> *, bool>
	{
		bool operator() (const StaticQuadtreeNode<T> *a, const StaticQuadtreeNode<T> *b) const
		{
			/*
			float rangeA = position.GetSquareRangeTo(a->position);
			float rangeB = position.GetSquareRangeTo(b->position);

			return rangeA < rangeB;
			*/
			return a->data < b->data;
		}
	};

	StaticQuadtreeRaytraceCollision(StaticQuadtree<T> &tree, const Ray &ray_, Storm3D_CollisionInfo &info_, bool accurate_)
	:	ray(ray_),
		info(info_),
		accurate(accurate_)
	{
		if(!collision(tree.root->looseArea, ray))
			return;

		iterateNode(tree.root);
	}

	void iterateNode(StaticQuadtreeNode<T> *node)
	{
		if(!node)
			return;

		// Entities or child nodes first?

		// Childs
		{
			/*
			StaticQuadtreeNode<T> *childs[4] = { 0 };
			int childNodes = 0;

			for(int i = 0; i < 4; ++i)
			{
				StaticQuadtreeNode<T> *child = node->childs[i];
				if(child && collision(ray, child->looseArea))
					childs[childNodes++] = child;
			}

			if(childNodes)
			{
				// Sort the list based on distance to ray origin
				//std::stable_sort(&childs[0], &childs[childNodes], RaySorter(ray.origin));

				for(int j = 0; j < childNodes; ++j)
					iterateNode(childs[j]);
			}
			*/

			int childNodes = 0;
			StaticQuadtreeNode<T> *childs[4] = { 0 };

			for(int i = 0; i < 4; ++i)
			{
				StaticQuadtreeNode<T> *child = node->childs[i];
				if(!child)
					continue;

				childs[childNodes] = child;
				child->data = ray.origin.GetSquareRangeTo(child->position);
				++childNodes;
			}

			if(childNodes)
			{
				std::stable_sort(&childs[0], &childs[childNodes], RaySorter());
				for(int j = 0; j < childNodes; ++j)
				{
					StaticQuadtreeNode<T> *child = childs[j];
					
					if(collision(ray, child->looseArea))
						iterateNode(child);
				}
			}
		}

		// Entities
		{
			for(unsigned int i = 0; i < node->entities.size(); ++i)
			{
				T &instance = node->entities[i];
				if(!collision(ray, instance.sphere))
					continue;
			
				instance.RayTrace(ray.origin, ray.direction, ray.range, info, accurate);
				if(info.hit && info.range < ray.range)
					ray.range = info.range;

				/*
				StaticQuadtreeEntity<T>::EntityImp *imp = node->entities[i];
				if(!collision(ray, imp->sphere))
					continue;

				imp->instance->RayTrace(ray.origin, ray.direction, ray.range, info, accurate);
				if(info.hit && info.range < ray.range)
					ray.range = info.range;
				*/
			}
		}
	}

	friend class StaticQuadtree<T>;
};

template<class T>
class StaticQuadtreeSphereCollision
{
	Sphere sphere;
	Storm3D_CollisionInfo &info;
	bool accurate;

	struct SphereSorter: public std::binary_function<StaticQuadtreeNode<T> *, StaticQuadtreeNode<T> *, bool>
	{
		const VC3 &position;

		explicit SphereSorter(const VC3 &position_)
		:	position(position_)
		{
		}

		bool operator() (const StaticQuadtreeNode<T> *a, const StaticQuadtreeNode<T> *b) const
		{
			float rangeA = position.GetSquareRangeTo(a->position);
			float rangeB = position.GetSquareRangeTo(b->position);

			return rangeA < rangeB;
		}
	};

	StaticQuadtreeSphereCollision(StaticQuadtree<T> &tree, const Sphere &sphere_, Storm3D_CollisionInfo &info_, bool accurate_)
	:	sphere(sphere_),
		info(info_),
		accurate(accurate_)
	{
		if(!collision(tree.root->looseArea, sphere))
			return;

		iterateNode(tree.root);
	}

	void iterateNode(StaticQuadtreeNode<T> *node)
	{
		if(!node)
			return;

		// Entities or child nodes first?

		// Childs
		{
			StaticQuadtreeNode<T> *childs[4] = { 0 };
			int childNodes = 0;

			for(int i = 0; i < 4; ++i)
			{
				StaticQuadtreeNode<T> *child = node->childs[i];
				if(child && collision(sphere, child->looseArea))
					childs[childNodes++] = child;
			}

			if(childNodes)
			{
				// Sort the list based on distance to ray origin
				//std::stable_sort(&childs[0], &childs[childNodes], SphereSorter(sphere.position));

				for(int j = 0; j < childNodes; ++j)
					iterateNode(childs[j]);
			}
		}

		// Entities
		{
			for(unsigned int i = 0; i < node->entities.size(); ++i)
			{
				T &instance = node->entities[i];
				if(!collision(sphere, instance.sphere))
					continue;

				instance.SphereCollision(sphere.position, sphere.radius, info, accurate);
				if(info.hit && info.range < sphere.radius)
					sphere.radius = info.range;

				/*
				StaticQuadtreeEntity<T>::EntityImp *imp = node->entities[i];
				if(!collision(sphere, imp->sphere))
					continue;

				imp->instance->SphereCollision(sphere.position, sphere.radius, info, accurate);
				if(info.hit && info.range < sphere.radius)
					sphere.radius = info.range;
				*/
			}

		}
	}

	friend class StaticQuadtree<T>;
};


template<class T>
class StaticQuadtree
{
	StaticQuadtreeNode<T> *root;

	// Not implemented
	StaticQuadtree<T> (const StaticQuadtree<T> &);
	StaticQuadtree<T> &operator = (const StaticQuadtree<T> &);

public:
	typedef StaticQuadtreeNode<T> Node;

	StaticQuadtree(const VC2 &min, const VC2 &max);
	~StaticQuadtree();

	void insert(T &instance, const VC3 &position, float radius, float heightMin, float heightMax);

	void RayTrace(Ray &ray, Storm3D_CollisionInfo &info, bool accurate);
	void SphereCollision(Sphere &sphere, Storm3D_CollisionInfo &info, bool accurate);

	friend struct StaticQuadtreeNode<T>;
	friend class StaticQuadtreeRaytraceCollision<T>;
	friend class StaticQuadtreeSphereCollision<T>;
};

// Definitions

template<class T>
StaticQuadtree<T>::StaticQuadtree(const VC2 &min_, const VC2 &max_)
:	root(0)
{
	VC3 mmin(min_.x, 0, min_.y);
	VC3 mmax(max_.x, 1.f, max_.y);

	AABB area(mmin, mmax);
	AABB looseArea;

	StaticQuadtree<T>::Node::calculateLooseArea(area, looseArea);	
	root = new Node(area, looseArea, 0);
}

template<class T>
StaticQuadtree<T>::~StaticQuadtree()
{
	delete root;
}

template<class T>
void StaticQuadtree<T>::insert(T &instance, const VC3 &position, float radius, float heightMin, float heightMax)
{
	root->insert(instance, position, radius, heightMin, heightMax);
}

template<class T>
void StaticQuadtree<T>::RayTrace(Ray &ray, Storm3D_CollisionInfo &info, bool accurate)
{
	StaticQuadtreeRaytraceCollision<T> rayTracer(*this, ray, info, accurate);
}

template<class T>
void StaticQuadtree<T>::SphereCollision(Sphere &sphere, Storm3D_CollisionInfo &info, bool accurate)
{
	StaticQuadtreeSphereCollision<T> sphereCollision(*this, sphere, info, accurate);
}

#endif
