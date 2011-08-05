// Copyright 2003-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_QTREE_H
#define INCLUDED_C2_QTREE_H

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
// Stores pointers, does not delete them

namespace {
	static const float LOOSE_FACTOR = 1.5;
	static const float MIN_SIZE = 1.f;
} // unnamed

template<class T>
struct QuadtreeNode;
template<class T>
class QuadtreeEntity;
template<class T>
class Quadtree;
template<class T>
class QuadtreeRaytraceCollision;
template<class T>
class QuadtreeSphereCollision;
template<class T>
class QuadtreeFrustumIterator;

template<class T>
class QuadtreeEntity
{
	struct EntityImp
	{
		Sphere sphere;
		T *instance;

		QuadtreeNode<T> *node;
		QuadtreeEntity<T> *entity;

		EntityImp(const Sphere &sphere_, T *instance_)
		:	sphere(sphere_),
			instance(instance_),
			node(0),
			entity(0)
		{
		}

		~EntityImp()
		{
			delete entity;
		}

		// Not implemented
		EntityImp(const EntityImp &other);
		EntityImp &operator = (const EntityImp &other);
	};

	EntityImp *implementation;

	// Not implemented
	QuadtreeEntity(const QuadtreeEntity &other);
	QuadtreeEntity &operator = (const QuadtreeEntity &other);

	void reinsert()
	{
		if(contains(implementation->node->looseArea, implementation->sphere) || implementation->instance->fits(implementation->node->looseArea))
		{
			QuadtreeNode<T> *parent = implementation->node;
			if(!parent->root)
				return;

			parent->insert(*implementation);
			parent->erase(implementation);

			return;
		}

		// Reinsert to tree

		QuadtreeNode<T> *oldNode = implementation->node;
		if(!oldNode->root)
			return;

		oldNode->root->insert(*implementation);
		oldNode->erase(implementation);

		//assert(oldNode != implementation->node);
	}

public:
	QuadtreeEntity(EntityImp &implementation_)
	:	implementation(&implementation_)
	{
	}

	~QuadtreeEntity()
	{
	}

	void setPosition(const VC3 &position)
	{
		implementation->sphere.position = position;
		reinsert();
	}

	void setRadius(float radius)
	{
		implementation->sphere.radius = radius;
		reinsert();
	}

	T &operator *()
	{
		return implementation->instance;
	}

	const T &operator *() const
	{
		return implementation->instance;
	}

	friend struct QuadtreeNode<T>;
	friend class Quadtree<T>;
	friend class QuadtreeRaytraceCollision<T>;
	friend class QuadtreeSphereCollision<T>;
	friend class QuadtreeFrustumIterator<T>;
};

template<class T>
struct QuadtreeNode
{
	AABB area;
	AABB looseArea;
	VC3 position;

	QuadtreeNode<T> *childs[4];
	QuadtreeNode<T> *root;

	std::vector<typename QuadtreeEntity<T>::EntityImp *> entities;
	float data;

	QuadtreeNode(const AABB &area_, const AABB &looseArea_, QuadtreeNode<T> *root_)
	:	area(area_),
		looseArea(looseArea_),
		root(root_)
	{
		position = (area.mmin + area.mmax) * .5f;
		
		for(int i = 0; i < 4; ++i)
			childs[i] = 0;
	}

	~QuadtreeNode()
	{
		for(int i = 0; i < 4; ++i)
			delete childs[i];

		for(unsigned int j = 0; j < entities.size(); ++j)
			delete entities[j];
	}

	QuadtreeEntity<T> *insert(T *instance, const VC3 &position, float radius)
	{
		Sphere sphere(position, radius);

		typename QuadtreeEntity<T>::EntityImp *imp = new typename QuadtreeEntity<T>::EntityImp(sphere, instance);
		insert(*imp);

		QuadtreeEntity<T> *e = new QuadtreeEntity<T> (*imp);
		imp->entity = e;

		assert(e);
		return e;
	}

	static void calculateLooseArea(const AABB &area, AABB &looseArea)
	{
		float xDelta = (area.mmax.x - area.mmin.x) * LOOSE_FACTOR;
		float zDelta = (area.mmax.z - area.mmin.z) * LOOSE_FACTOR;

		looseArea.mmin.x = area.mmax.x - xDelta;
		looseArea.mmin.y = area.mmin.y;
		looseArea.mmin.z = area.mmax.z - zDelta;

		looseArea.mmax.x = area.mmin.x + xDelta;
		looseArea.mmax.y = area.mmax.y;
		looseArea.mmax.z = area.mmin.z + zDelta;
	}

	void insert(typename QuadtreeEntity<T>::EntityImp &imp)
	{
		const Sphere &sphere = imp.sphere;

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

		if(area.mmax.x - area.mmin.x > MIN_SIZE)
		if(area.mmax.z - area.mmin.z > MIN_SIZE)
		{
			AABB area[4];
			AABB looseArea[4];
			
			// Fits to normal area?

			QuadtreeNode<T> *realRoot = root;
			if(!realRoot)
				realRoot = this;

			for(int i = 0; i < 4; ++i)
			{
				getChildArea(i, area[i]);
				calculateLooseArea(area[i], looseArea[i]);

				if(contains(area[i], sphere) || imp.instance->fits(area[i]))
				{
					if(!childs[i])
						childs[i] = new QuadtreeNode<T>(area[i], looseArea[i], realRoot);

					childs[i]->insert(imp);
					return;
				}
			}

			// Fits to loose area?

			for(int j = 0; j < 4; ++j)
			{
				if(contains(looseArea[j], sphere) || imp.instance->fits(area[j]))
				{
					if(!childs[j])
						childs[j] = new QuadtreeNode<T>(area[j], looseArea[j], realRoot);

					childs[j]->insert(imp);
					return;
				}
			}
		}

		// Store

		imp.node = this;		
		entities.push_back(&imp);
	}

	void erase(typename QuadtreeEntity<T>::EntityImp *entity)
	{
		for(unsigned int i = 0; i < entities.size(); ++i)
		{
			if(entities[i] == entity)
			{
				entities.erase(entities.begin() + i);
				return;
			}
		}

		assert(!"Entity not found!");
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

	/*
	void getChildLooseArea(int index, VC2 &min, VC2 &max) const
	{
		assert(index >= 0 && index < 4);
	}
	*/

	friend class QuadtreeEntity<T>;
	friend class Quadtree<T>;
	friend class QuadtreeRaytraceCollision<T>;
	friend class QuadtreeSphereCollision<T>;
};

template<class T>
class QuadtreeRaytraceCollision
{
	Ray ray;
	Storm3D_CollisionInfo &info;
	bool accurate;

	struct RaySorter: public std::binary_function<QuadtreeNode<T> *, QuadtreeNode<T> *, bool>
	{
		bool operator() (const QuadtreeNode<T> *a, const QuadtreeNode<T> *b) const
		{
			return a->data < b->data;
		}
	};

	QuadtreeRaytraceCollision(Quadtree<T> &tree, const Ray &ray_, Storm3D_CollisionInfo &info_, bool accurate_)
	:	ray(ray_),
		info(info_),
		accurate(accurate_)
	{
		if(!collision(tree.root->looseArea, ray))
			return;

		iterateNode(tree.root);
	}

	void iterateNode(QuadtreeNode<T> *node)
	{
		if(!node)
			return;

		// Entities or child nodes first?

		// Childs
		{
			unsigned int childNodes = 0;
			QuadtreeNode<T> *childs[4] = { 0 };

			for(int i = 0; i < 4; ++i)
			{
				QuadtreeNode<T> *child = node->childs[i];
				if(!child)
					continue;

				childs[childNodes] = child;
				child->data = ray.origin.GetSquareRangeTo(child->position);
				++childNodes;
			}

			if(childNodes)
			{
				std::sort(&childs[0], &childs[childNodes], RaySorter());
				for(unsigned int j = 0; j < childNodes; ++j)
				{
					QuadtreeNode<T> *child = childs[j];
					
					if(collision(ray, child->looseArea))
						iterateNode(child);
				}
			}
		}

		/*
		// Childs
		{
			QuadtreeNode<T> *childs[4] = { 0 };
			int childNodes = 0;

			for(int i = 0; i < 4; ++i)
			{
				QuadtreeNode<T> *child = node->childs[i];
				if(child && collision(ray, child->looseArea))
					childs[childNodes++] = child;
			}

			if(childNodes)
			{
				// Sort the list based on distance to ray origin
				std::stable_sort(&childs[0], &childs[childNodes], RaySorter(ray.origin));

				for(int j = 0; j < childNodes; ++j)
					iterateNode(childs[j]);
			}
		}
		*/

		// Entities
		{
			for(unsigned int i = 0; i < node->entities.size(); ++i)
			{
				typename QuadtreeEntity<T>::EntityImp *imp = node->entities[i];
				if(!collision(ray, imp->sphere))
					continue;

				imp->instance->RayTrace(ray.origin, ray.direction, ray.range, info, accurate);
				if(info.hit && info.range < ray.range)
					ray.range = info.range;
			}
		}
	}

	friend class Quadtree<T>;
};

template<class T>
class QuadtreeSphereCollision
{
	Sphere sphere;
	Storm3D_CollisionInfo &info;
	bool accurate;

	struct SphereSorter: public std::binary_function<QuadtreeNode<T> *, QuadtreeNode<T> *, bool>
	{
		const VC3 &position;

		explicit SphereSorter(const VC3 &position_)
		:	position(position_)
		{
		}

		bool operator() (const QuadtreeNode<T> *a, const QuadtreeNode<T> *b) const
		{
			float rangeA = position.GetSquareRangeTo(a->position);
			float rangeB = position.GetSquareRangeTo(b->position);

			return rangeA < rangeB;
		}
	};

	QuadtreeSphereCollision(Quadtree<T> &tree, const Sphere &sphere_, Storm3D_CollisionInfo &info_, bool accurate_)
	:	sphere(sphere_),
		info(info_),
		accurate(accurate_)
	{
		if(!collision(tree.root->looseArea, sphere))
			return;

		iterateNode(tree.root);
	}

	void iterateNode(QuadtreeNode<T> *node)
	{
		if(!node)
			return;

		// Entities or child nodes first?

		// Childs
		{
			QuadtreeNode<T> *childs[4] = { 0 };
			int childNodes = 0;

			for(int i = 0; i < 4; ++i)
			{
				QuadtreeNode<T> *child = node->childs[i];
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
				typename QuadtreeEntity<T>::EntityImp *imp = node->entities[i];
				if(!collision(sphere, imp->sphere))
					continue;

				imp->instance->SphereCollision(sphere.position, sphere.radius, info, accurate);
				if(info.hit && info.range < sphere.radius)
					sphere.radius = info.range;
			}

		}
	}

	friend class Quadtree<T>;
};

template<class T>
class QuadtreeFrustumIterator
{
	Frustum frustum;

	struct Node
	{
		Frustum::VisibilityValue visibility;
		QuadtreeNode<T> *node;

		Node(Frustum::VisibilityValue visibility_, QuadtreeNode<T> *node_)
		:	visibility(visibility_),
			node(node_)
		{
		}
	};

	std::stack<Node> stack;
	int entityIndex;

public:
	QuadtreeFrustumIterator(Quadtree<T> &tree, const Frustum &frustum_)
	:	frustum(frustum_),
		entityIndex(-1)
	{
		QuadtreeNode<T> *node = tree.root;
		Frustum::VisibilityValue visibility = frustum.visibilityValue(node->looseArea);

		if(visibility != Frustum::None)
			stack.push(Node(visibility, node));

		next();
	}

	void next()
	{
		while(!end())
		{
			const Node &node = stack.top();
			const std::vector<typename QuadtreeEntity<T>::EntityImp *> &entities = node.node->entities;

			++entityIndex;
			if(entityIndex >= int(entities.size()))
			{
				Node oldNode = node;
				stack.pop();

				for(int i = 0; i < 4; ++i)
				{
					QuadtreeNode<T> *child = oldNode.node->childs[i];
					if(!child)
						continue;

					Frustum::VisibilityValue visibility = oldNode.visibility;
					assert(visibility != Frustum::None);

					if(visibility == Frustum::Partial)
					{
						visibility = frustum.visibilityValue(child->looseArea);
	
						if(visibility == Frustum::None)
							continue;
					}

					stack.push(Node(visibility, child));
				}

				entityIndex = -1;
				continue;
			}

			typename QuadtreeEntity<T>::EntityImp *imp = entities[entityIndex];
			assert(imp->sphere.radius >= 0);

			bool visible = true;
			assert(node.visibility != Frustum::None);

			if(node.visibility == Frustum::Partial)
				visible = frustum.visibility(imp->sphere);

			if(visible)
				break;
		}
	}

	bool end() const
	{
		if(stack.empty())
			return true;

		return false;
	}

	T *operator *()
	{
		assert(!end());
		return stack.top().node->entities[entityIndex]->instance;
	}

	const T *operator *() const
	{
		assert(!end());
		return stack.top().node->entities[entityIndex]->instance;
	}
};

template<class T>
class Quadtree
{
	QuadtreeNode<T> *root;

	// Not implemented
	Quadtree<T> (const Quadtree<T> &);
	Quadtree<T> &operator = (const Quadtree<T> &);

	float distance2D(const VC2 &a, const VC3 &b)
	{
		float xd = a.x - b.x;
		float yd = a.y - b.z;

		return sqrtf(xd*xd + yd*yd);
	}

public:
	typedef QuadtreeNode<T> Node;
	typedef QuadtreeEntity<T> Entity;
	typedef QuadtreeFrustumIterator<T> FrustumIterator;

	Quadtree(const VC2 &min, const VC2 &max);
	~Quadtree();

	typename Quadtree<T>::Entity *insert(T *instance, const VC3 &position, float radius);
	void erase(typename Quadtree<T>::Entity *entity);

	void RayTrace(Ray &ray, Storm3D_CollisionInfo &info, bool accurate);
	void SphereCollision(const Sphere &sphere, Storm3D_CollisionInfo &info, bool accurate);

	void collectSphere(Node *node, std::vector<T *> &list, const VC3 &position, float radius)
	{
		typename std::vector<typename QuadtreeEntity<T>::EntityImp *>::iterator it = node->entities.begin();
		for(; it != node->entities.end(); ++it)
		{
			Storm3D_CollisionInfo info;
			(*it)->instance->SphereCollision(position, radius, info, true);

			if(info.hit)
				list.push_back((*it)->instance);

			//const Sphere &sphere = (*it)->sphere;
			//float distance = distance2D(position, sphere.position);
			//if(distance - radius - sphere.radius <= 0)
			//	list.push_back((*it)->instance);
		}

		for(int i = 0; i < 4; ++i)
		{
			if(!node->childs[i])
				continue;

			const AABB &area = node->childs[i]->looseArea;
			if(position.x > area.mmin.x - radius)
			if(position.x < area.mmax.x + radius)
			if(position.z > area.mmin.z - radius)
			if(position.z < area.mmax.z + radius)
				collectSphere(node->childs[i], list, position, radius);
		}
	}

	void collectSphere(std::vector<T *> &list, const VC3 &position, float radius)
	{
		collectSphere(root, list, position, radius);
	}

	friend struct QuadtreeNode<T>;
	friend class QuadtreeEntity<T>;
	friend class QuadtreeRaytraceCollision<T>;
	friend class QuadtreeSphereCollision<T>;
	friend class QuadtreeFrustumIterator<T>;
};

// Definitions

template<class T>
Quadtree<T>::Quadtree(const VC2 &min_, const VC2 &max_)
:	root(0)
{
	VC3 mmin(min_.x, 0, min_.y);
	VC3 mmax(max_.x, 1.f, max_.y);

	AABB area(mmin, mmax);
	AABB looseArea;

	Quadtree<T>::Node::calculateLooseArea(area, looseArea);	
	root = new typename Quadtree<T>::Node(area, looseArea, 0);
}

template<class T>
Quadtree<T>::~Quadtree()
{
	delete root;
}

template<class T>
typename Quadtree<T>::Entity *Quadtree<T>::insert(T *instance, const VC3 &position, float radius)
{
	if(!instance)
		return 0;

	return root->insert(instance, position, radius);
}

template<class T>
void Quadtree<T>::erase(typename Quadtree<T>::Entity *entity)
{
	entity->implementation->node->erase(entity->implementation);
	delete entity->implementation;
}

template<class T>
void Quadtree<T>::RayTrace(Ray &ray, Storm3D_CollisionInfo &info, bool accurate)
{
	QuadtreeRaytraceCollision<T> rayTracer(*this, ray, info, accurate);
}

template<class T>
void Quadtree<T>::SphereCollision(const Sphere &sphere, Storm3D_CollisionInfo &info, bool accurate)
{
	QuadtreeSphereCollision<T> sphereCollision(*this, sphere, info, accurate);
}

#endif
