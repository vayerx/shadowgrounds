// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Object_Chopper.h"
#include "Export_Object.h"
#include <c2_qtree.h>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <map>
#include <windows.h>
#include <fstream>

namespace frozenbyte {
namespace exporter {
namespace {

	// 12, 7
	static const double GRID_SIZE = 12.0;
	static const double MAX_EDGE_LENGTH = 5.0;

	//static const double JEPSILON = 0.0001;
	static const double JEPSILON_FLOAT = 0.005f;
	static const double JEPSILON = 0.0001;
	static const double JEPSILON_SQ = JEPSILON * JEPSILON;

	FBVector maxVertexValues(Object &object, const FBMatrix &tm = FBMatrix())
	{
		FBVector result(-100000000, -100000000, -100000000);
		const std::vector<Vertex> &vertices = object.getVertices();

		for(unsigned int i = 0; i < vertices.size(); ++i)
		{
			FBVector pos = vertices[i].getPosition();
			tm.TransformVector(pos);

			result.x = max(result.x, pos.x);
			result.y = max(result.y, pos.y);
			result.z = max(result.z, pos.z);
		}

		return result;
	}

	FBVector minVertexValues(Object &object, const FBMatrix &tm = FBMatrix())
	{
		FBVector result(100000000.f, 100000000.f, 100000000.f);
		const std::vector<Vertex> &vertices = object.getVertices();

		for(unsigned int i = 0; i < vertices.size(); ++i)
		{
			FBVector pos = vertices[i].getPosition();
			tm.TransformVector(pos);

			result.x = min(result.x, pos.x);
			result.y = min(result.y, pos.y);
			result.z = min(result.z, pos.z);
		}

		return result;
	}

	double calculateSizeImp(Object &object, const FBMatrix &tm)
	{
		FBVector maxValue = maxVertexValues(object, tm);
		FBVector minValue = minVertexValues(object, tm);

		return maxValue.GetRangeTo(minValue);
	}

	double distance(double a, double b)
	{
		return sqrt(a*a + b*b);
	}

	double range2D(const FBVector &a, const FBVector &b)
	{
		double xd = a.x - b.x;
		double zd = a.z - b.z;
		
		return sqrt(xd*xd + zd*zd);
	}

	double range3D(const FBVector &a, const FBVector &b)
	{
		double xd = a.x - b.x;
		double yd = a.y - b.y;
		double zd = a.z - b.z;
		
		return sqrt(xd*xd + yd*yd + zd*zd);
	}

	int getChopEdge(const FBVector &v1, const FBVector &v2, const FBVector &v3)
	{
		/*
		if(range2D(v1, v2) > MAX_EDGE_LENGTH)
			return 0;
		if(range2D(v2, v3) > MAX_EDGE_LENGTH)
			return 1;
		if(range2D(v3, v1) > MAX_EDGE_LENGTH)
			return 2;
		*/
		/*
		double a = range2D(v1, v2);
		double b = range2D(v2, v3);
		double c = range2D(v3, v1);
		*/
		double a = range3D(v1, v2);
		double b = range3D(v2, v3);
		double c = range3D(v3, v1);

		if(a > b && a > c)
		{
			if(a > MAX_EDGE_LENGTH)
				return 0;

			//return -1;
		}
		if(b > a && b > c)
		{
			if(b > MAX_EDGE_LENGTH)
				return 1;

			//return -1;
		}

		if(c > MAX_EDGE_LENGTH)
			return 2;

		// In equality cases these cases could be skipped
		if(a > MAX_EDGE_LENGTH)
			return 0;
		if(b > MAX_EDGE_LENGTH)
			return 1;

		return -1;
	}

	double getArea(const Vertex &v1, const Vertex &v2, const Vertex &v3)
	{
		FBVector e1 = v2.getPosition() - v1.getPosition();
		FBVector e2 = v3.getPosition() - v1.getPosition();
		return e1.GetCrossWith(e2).GetSquareLength();
	}

	Vertex createNewVertex(const Vertex &v1, const Vertex &v2)
	{
		Vertex result = v1;

		FBVector position = v1.getPosition() + v2.getPosition();
		position *= .5f;

		FBVector normal = v1.getNormal() + v2.getNormal();
		if(normal.GetSquareLength() > 0.000000001f)
			normal.Normalize();

		FBVector2 uv1 = v1.getUv() + v2.getUv();
		uv1 *= .5f;
		FBVector2 uv2 = v1.getUv2() + v2.getUv2();
		uv2 *= .5f;

		// ToDo -- loses proper bone weights (just uses v1). 
		// Should choose largest weight and store that.

		result.setPosition(position);
		result.setNormal(normal);
		result.setUv(uv1);
		result.setUv2(uv2);

		return result;
	}

	void centerObject(Object &object, const FBVector &minValue)
	{
		FBVector center = (maxVertexValues(object) + minVertexValues(object)) / 2;
		int x = int(center.x / GRID_SIZE);
		int z = int(center.z / GRID_SIZE);

		center.x = x * GRID_SIZE;
		center.y = 0;
		center.z = z * GRID_SIZE;

		if(x > 0)
			center.x += GRID_SIZE/2;
		else if(x < 0)
			center.x -= GRID_SIZE/2;
		if(z > 0)
			center.z += GRID_SIZE/2;
		else if(z < 0)
			center.z -= GRID_SIZE/2;

		FBMatrix transform;
		transform.CreateTranslationMatrix(center);

		object.setTransform(transform);
		object.setPivotTransform(transform);

		std::vector<Vertex> &vertexBuffer = object.getVertices();
		for(unsigned int i = 0; i < vertexBuffer.size(); ++i)
		{
			const FBVector &position = vertexBuffer[i].getPosition();
			vertexBuffer[i].setPosition(position - center);
		}
	}

	void chopObject(Object &object, std::vector<boost::shared_ptr<Object> > &newObjects)
	{
		if(object.hasCollisionVisibility() && !object.hasCameraVisibility())
		{
			newObjects.push_back(boost::shared_ptr<Object> (new Object(object)));
			return;
		}
		if(!object.hasCameraVisibility())
		{
			newObjects.push_back(boost::shared_ptr<Object> (new Object(object)));
			return;
		}

		double size = calculateSize(object);
		VC2I grids = VC2I(int(size / GRID_SIZE), int(size / GRID_SIZE)) + VC2I(2,2);

		int objectAmount = grids.x * grids.y;
		std::vector<boost::shared_ptr<Object> > gridObjects(objectAmount);
		//std::vector<std::vector<int> > gridIndices(objectAmount);
		std::vector<std::map<int, int> > gridIndices(objectAmount);

		{
			for(int i = 0; i < objectAmount; ++i)
			{
				std::string name = boost::lexical_cast<std::string> (i);
				name += object.getName();

				gridObjects[i].reset(new Object());
				gridObjects[i]->setStrings(name, object.getParentName());
				gridObjects[i]->setProperties(object.hasCameraVisibility(), object.hasCollisionVisibility());

				gridObjects[i]->setTransform(object.getTransform());
				gridObjects[i]->setPivotTransform(object.getPivotTransform());

				if(object.isLightObject())
					gridObjects[i]->setLightObject();

				//gridIndices[i].resize(object.getVertexAmount());

				//for(int j = 0; j < object.getVertexAmount(); ++j)
				//	gridIndices[i][j] = -1;
			}
		}

		const std::vector<Face> &faces = object.getFaces();
		const std::vector<Vertex> &vertices = object.getVertices();

		FBVector minValue = minVertexValues(object);
		//minValue.x = minValue.x < 0 ? minValue.x : 0;
		//minValue.y = minValue.y < 0 ? minValue.y : 0;
		//minValue.z = minValue.z < 0 ? minValue.z : 0;

		// Move faces to correct objects
		{
			for(unsigned int i = 0; i < faces.size(); ++i)
			{
				const Face &face = faces[i];

				FBVector center;
				for(int j = 0; j < 3; ++j)
				{
					int vertexIndex = face.getVertexIndex(j);
					center += vertices[vertexIndex].getPosition();
				}

				center /= 3.f;
				center -= minValue;

				VC2I gridIndex(int(center.x / GRID_SIZE), int(center.z / GRID_SIZE));
				int index = (gridIndex.y * grids.x + gridIndex.x);

				Object &gridObject = *gridObjects[index];

				Face newFace = face;
				for(int k = 0; k < 3; ++k)
				{
					int vertexIndex = newFace.getVertexIndex(k);

					//if(gridIndices[index][vertexIndex] == -1)
					if(gridIndices[index].find(vertexIndex) == gridIndices[index].end())
					{
						gridIndices[index][vertexIndex] = gridObject.getVertexAmount();
						gridObject.addVertex(vertices[vertexIndex]);
					}

					int newIndex = gridIndices[index][vertexIndex];
					assert(newIndex >= 0 && int(vertices.size()));

					newFace.setVertexIndex(k, newIndex);
				}

				gridObject.addFace(newFace);
			}
		}

		// Finalize
		{
			for(int y = 0; y < grids.y; ++y)
			for(int x = 0; x < grids.x; ++x)
			{
				int index = y * grids.x + x;
				boost::shared_ptr<Object> &gridObject = gridObjects[index];

				if(gridObject->getFaceAmount() > 0 && gridObject->getVertexAmount() > 0)
				{
					//centerObject(gridObject, minValue);
					newObjects.push_back(gridObject);
				}
			}
		}
	}

	bool needLimitChop(const Object &object)
	{
		const std::vector<Face> &faces = object.getFaces();
		const std::vector<Vertex> &vertices = object.getVertices();

		if(vertices.size() > 65535)
			return true;
		if(faces.size() > 65535)
			return true;

		/*
		for(unsigned int i = 0; i < faces.size(); ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				if(faces[i].getVertexIndex(j) > 65535)
					return true;
			}
		}
		*/

		return false;
	}

	void chopObjectToLimits(Object &object, std::vector<boost::shared_ptr<Object> > &newObjects)
	{
		if(!object.hasCameraVisibility())
		{
			newObjects.push_back(boost::shared_ptr<Object> (new Object(object)));
			return;
		}

		const std::vector<Face> &faces = object.getFaces();
		const std::vector<Vertex> &vertices = object.getVertices();

		if(!needLimitChop(object))
		{
			newObjects.push_back(boost::shared_ptr<Object> (new Object(object)));
			return;
		}

		std::vector<boost::shared_ptr<Object> > gridObjects(4);
		//std::vector<std::vector<int> > gridIndices(4);
		std::vector<std::map<int, int> > gridIndices(4);
		{
			for(int i = 0; i < 4; ++i)
			{
				std::string name = boost::lexical_cast<std::string> (i);
				name += "_";
				name += object.getName();

				gridObjects[i].reset(new Object());
				gridObjects[i]->setStrings(name, object.getParentName());

				gridObjects[i]->setTransform(object.getTransform());
				gridObjects[i]->setPivotTransform(object.getPivotTransform());

				//gridIndices[i].resize(object.getVertexAmount());
				//for(int j = 0; j < object.getVertexAmount(); ++j)
				//	gridIndices[i][j] = -1;
			}
		}

		FBVector minValue = minVertexValues(object);
		FBVector maxValue = maxVertexValues(object);
		FBVector origo = (minValue + maxValue) * .5f;

		// Move faces to correct objects
		{
			for(unsigned int i = 0; i < faces.size(); ++i)
			{
				const Face &face = faces[i];

				FBVector center;
				for(int j = 0; j < 3; ++j)
				{
					int vertexIndex = face.getVertexIndex(j);
					center += vertices[vertexIndex].getPosition();
				}

				center /= 3.f;
				center -= minValue;

				VC2I gridIndex;
				if(center.x > origo.x)
					gridIndex.x = 1;
				if(center.z > origo.z)
					gridIndex.y = 1;
				int index = (gridIndex.y * 2 + gridIndex.x);

				Object &gridObject = *gridObjects[index];
				gridObject.setProperties(object.hasCameraVisibility(), object.hasCollisionVisibility());

				if(object.isLightObject())
					gridObject.setLightObject();

				Face newFace = face;
				for(int k = 0; k < 3; ++k)
				{
					int vertexIndex = newFace.getVertexIndex(k);

					//if(gridIndices[index][vertexIndex] == -1)
					if(gridIndices[index].find(vertexIndex) == gridIndices[index].end())
					{
						gridIndices[index][vertexIndex] = gridObject.getVertexAmount();
						gridObject.addVertex(vertices[vertexIndex]);
					}

					int newIndex = gridIndices[index][vertexIndex];
					assert(newIndex >= 0 && int(vertices.size()));

					newFace.setVertexIndex(k, newIndex);
				}

				gridObject.addFace(newFace);
			}
		}

		// Finalize
		{
			for(int y = 0; y < 2; ++y)
			for(int x = 0; x < 2; ++x)
			{
				int index = y * 2 + x;
				Object &gridObject = *gridObjects[index];

				if(gridObject.getFaceAmount() > 0 && gridObject.getVertexAmount() > 0)
				{
					//newObjects.push_back(gridObject);
					chopObjectToLimits(gridObject, newObjects);
				}
			}
		}
	}

	// ------

	bool contains2D(const AABB &area, const FBVector &position)
	{
		if(position.x < area.mmin.x)
			return false;
		if(position.x > area.mmax.x)
			return false;

		if(position.z < area.mmin.z)
			return false;
		if(position.z > area.mmax.z)
			return false;

		return true;
	}

	bool isJunction(const FBVector &p1, const FBVector &p2, const FBVector &q)
	{
		FBVector qMinusP1 = q;
		qMinusP1 -= p1;
		FBVector p2MinusP1 = p2;
		p2MinusP1 -= p1;

		// Point q on edge line
		double qProj = qMinusP1.GetDotWith(p2MinusP1);
		double dSq = qMinusP1.GetSquareLength() - (qProj * qProj) / p2MinusP1.GetSquareLength();
		if(dSq > JEPSILON_SQ)
			return false;

		// Point q between endpoints
		double edgeLength = p2MinusP1.GetLength();
		double t = qMinusP1.GetDotWith(p2MinusP1) / edgeLength;
		if(t < -JEPSILON || t > edgeLength - JEPSILON)
			return false;

		if(fabs(q.x - p1.x) < JEPSILON && fabs(q.y - p1.y) < JEPSILON && fabs(q.z - p1.z) < JEPSILON)
			return false;
		if(fabs(q.x - p2.x) < JEPSILON && fabs(q.y - p2.y) < JEPSILON && fabs(q.z - p2.z) < JEPSILON)
			return false;

		return true;
	}

	Vertex createVertex(const Vertex &v1, const Vertex &v2, const FBVector &p)
	{
		double range1 = v1.getPosition().GetRangeTo(p);
		double range2 = v1.getPosition().GetRangeTo(v2.getPosition());
		double factor2 = range1 / range2;
		double factor1 = 1.f - factor2;

		FBVector normal = v1.getNormal() * factor1;
		normal += v2.getNormal() * factor2;
		normal.Normalize();

		FBVector2 tex1 = v1.getUv() * factor1;
		tex1 += v2.getUv() * factor2;
		FBVector2 tex2 = v1.getUv2() * factor1;
		tex2 += v2.getUv2() * factor2;

		Vertex result = v1;
		result.setPosition(p);
		result.setNormal(normal);
		result.setUv(tex1);
		result.setUv2(tex2);

		return result;
	}

	struct JunctionFace
	{
		TPlane<double> facePlane;
		TPlane<double> edgePlane1;
		TPlane<double> edgePlane2;
		TPlane<double> edgePlane3;

		int objectIndex;
		int faceIndex;

		Vertex v1;
		Vertex v2;
		Vertex v3;

		FBVector center;
		double radius;

		Quadtree<JunctionFace>::Entity *entity;

		JunctionFace(int objectIndex_, int faceIndex_)
		:	objectIndex(objectIndex_),
			faceIndex(faceIndex_),
			radius(0.f),
			entity(0)
		{
		}

		void transform(const FBMatrix &matrix)
		{
			FBVector p1 = v1.getPosition();
			matrix.TransformVector(p1);
			v1.setPosition(p1);

			FBVector p2 = v2.getPosition();
			matrix.TransformVector(p2);
			v2.setPosition(p2);

			FBVector p3 = v3.getPosition();
			matrix.TransformVector(p3);
			v3.setPosition(p3);
		}

		void update()
		{
			const FBVector &p1 = v1.getPosition();
			const FBVector &p2 = v2.getPosition();
			const FBVector &p3 = v3.getPosition();

			center = p1;
			center += p2;
			center += p3;
			center /= 3.f;

			double f = center.GetRangeTo(p1);
			radius = f;
			f = center.GetRangeTo(p2);
			if(f > radius)
				radius = f;
			f = center.GetRangeTo(p3);
			if(f > radius)
				radius = f;

			// Create planes for quadtree lookup optimization
			facePlane.MakeFromPoints(p1, p2, p3);
			FBVector edge1 = p2 - p1;
			FBVector edge2 = p3 - p2;
			FBVector edge3 = p1 - p3;
			edge1.Normalize();
			edge2.Normalize();
			edge3.Normalize();
			FBVector edgeNormal1 = edge1.GetCrossWith(facePlane.planenormal);
			FBVector edgeNormal2 = edge2.GetCrossWith(facePlane.planenormal);
			FBVector edgeNormal3 = edge3.GetCrossWith(facePlane.planenormal);

			edgePlane1.MakeFromNormalAndPosition(edgeNormal1, p1);
			edgePlane2.MakeFromNormalAndPosition(edgeNormal2, p2);
			edgePlane3.MakeFromNormalAndPosition(edgeNormal3, p3);
		}

		int getJunction(const FBVector &point)
		{
			const FBVector &p1 = v1.getPosition();
			const FBVector &p2 = v2.getPosition();
			const FBVector &p3 = v3.getPosition();

			if(isJunction(p1, p2, point))
				return 0;
			else if(isJunction(p2, p3, point))
				return 1;
			else if(isJunction(p3, p1, point))
				return 2;

			return -1;
		}

		bool fits(const AABB &area) const
		{
			if(!contains2D(area, v1.getPosition()))
				return false;
			if(!contains2D(area, v2.getPosition()))
				return false;
			if(!contains2D(area, v3.getPosition()))
				return false;

			return true;
		}

		void SphereCollision(const VC3 &position, double radius, Storm3D_CollisionInfo &info, bool accurate) const
		{
			FBVector positiond(position.x, position.y, position.z);
			double range = facePlane.GetPointRange(positiond);
			if(range < -JEPSILON_FLOAT * 10|| range > JEPSILON_FLOAT * 10)
				return;

			double edgeRange1 = edgePlane1.GetPointRange(positiond);
			if(edgeRange1 > 10 * JEPSILON_FLOAT)
				return;
			double edgeRange2 = edgePlane2.GetPointRange(positiond);
			if(edgeRange2 > 10 * JEPSILON_FLOAT)
				return;
			double edgeRange3 = edgePlane3.GetPointRange(positiond);
			if(edgeRange3 > 10 * JEPSILON_FLOAT)
				return;

			const FBVector &p1 = v1.getPosition();
			if(position.x == p1.x && position.y == p1.y && position.z == p1.z)
				return;
			const FBVector &p2 = v2.getPosition();
			if(position.x == p2.x && position.y == p2.y && position.z == p2.z)
				return;
			const FBVector &p3 = v3.getPosition();
			if(position.x == p3.x && position.y == p3.y && position.z == p3.z)
				return;

			info.hit = true;
		}
	};

	struct JunctionObject
	{
		std::deque<JunctionFace> faces;
		FBMatrix tm;
		FBMatrix inverseTm;
	};

	typedef Quadtree<JunctionFace> JunctionTree;

	struct SnapVertex
	{
		int objectIndex;
		int vertexIndex;
		FBVector position;

		Quadtree<SnapVertex>::Entity *entity;

		SnapVertex(int objectIndex_, int vertexIndex_)
		:	objectIndex(objectIndex_),
			vertexIndex(vertexIndex_),
			entity(0)
		{
		}

		void transform(const FBMatrix &matrix)
		{
			matrix.TransformVector(position);
		}

		bool fits(const AABB &area) const
		{
			if(!contains2D(area, position))
				return false;

			return true;
		}

		void SphereCollision(const VC3 &otherPosition, double radius, Storm3D_CollisionInfo &info, bool accurate) const
		{
			FBVector positiond(otherPosition.x, otherPosition.y, otherPosition.z);
			if(position.GetRangeTo(positiond) > radius)
				return;

			info.hit = true;
		}
	};

	struct SnapObject
	{
		std::deque<SnapVertex> vertices;
		FBMatrix tm;
		FBMatrix inverseTm;
	};

	typedef Quadtree<SnapVertex> SnapTree;

	/*
	void removeJunctions(Object &object)
	{
		std::vector<Vertex> &vertices = object.getVertices();
		std::vector<Face> &faces = object.getFaces();

		// SLOOOOOW!!!!

		int vertexAmount = vertices.size();
		for(int i = 0; i < vertexAmount; ++i)
		{
			FBVector vp = vertices[i].getPosition();

			for(unsigned int j = 0; j < faces.size(); ++j)
			{
				int fi1 = faces[j].getVertexIndex(0);
				int fi2 = faces[j].getVertexIndex(1);
				int fi3 = faces[j].getVertexIndex(2);

				if(i == fi1 || i == fi2 || i == fi3)
					continue;

				const Vertex &fv1 = vertices[fi1];
				const Vertex &fv2 = vertices[fi2];
				const Vertex &fv3 = vertices[fi3];
				const FBVector &fp1 = fv1.getPosition();
				const FBVector &fp2 = fv2.getPosition();
				const FBVector &fp3 = fv3.getPosition();

				int edge = -1;
				if(isJunction(fp1, fp2, vp))
				{
					Face f1 = faces[j];
					Face f2 = faces[j];

					Vertex nvv = createVertex(fv1, fv2, vp);
					int nvi = vertices.size();
					vertices.push_back(nvv);

					f1.setVertexIndex(0, fi1);
					f1.setVertexIndex(1, nvi);
					f1.setVertexIndex(2, fi3);
					f2.setVertexIndex(0, nvi);
					f2.setVertexIndex(1, fi2);
					f2.setVertexIndex(2, fi3);

					faces[j] = f1;
					faces.push_back(f2);
				}
				else if(isJunction(fp2, fp3, vp))
				{
					Face f1 = faces[j];
					Face f2 = faces[j];

					Vertex nvv = createVertex(fv2, fv3, vp);
					int nvi = vertices.size();
					vertices.push_back(nvv);

					f1.setVertexIndex(0, fi1);
					f1.setVertexIndex(1, fi2);
					f1.setVertexIndex(2, nvi);
					f2.setVertexIndex(0, fi1);
					f2.setVertexIndex(1, nvi);
					f2.setVertexIndex(2, fi3);

					faces[j] = f1;
					faces.push_back(f2);
				}
				else if(isJunction(fp3, fp1, vp))
				{
					Face f1 = faces[j];
					Face f2 = faces[j];

					Vertex nvv = createVertex(fv3, fv1, vp);
					int nvi = vertices.size();
					vertices.push_back(nvv);

					f1.setVertexIndex(0, fi1);
					f1.setVertexIndex(1, fi2);
					f1.setVertexIndex(2, nvi);
					f2.setVertexIndex(0, nvi);
					f2.setVertexIndex(1, fi2);
					f2.setVertexIndex(2, fi3);

					faces[j] = f1;
					faces.push_back(f2);
				}
			}
		}
	}
	*/

} // unnamed namespace

double calculateSize(Object &object)
{
	FBVector maxValue = maxVertexValues(object);
	FBVector minValue = minVertexValues(object);

	return maxValue.GetRangeTo(minValue);
}

void chopFaces(Object &object)
{
	if(!object.hasCameraVisibility() && object.hasCollisionVisibility())
		return;

	std::vector<Vertex> &vertices = object.getVertices();
	std::vector<Face> &faces = object.getFaces();

	for(unsigned int i = 0; i < faces.size(); )
	{
		int faceAmount = faces.size();
		Face face = faces[i];

		if(i > 100000)
			int a = 0;

		int i1 = face.getVertexIndex(0);
		int i2 = face.getVertexIndex(1);
		int i3 = face.getVertexIndex(2);
		Vertex v1 = vertices[i1];
		Vertex v2 = vertices[i2];
		Vertex v3 = vertices[i3];

		assert(i1 >= 0 && i1 < int(vertices.size()));
		assert(i2 >= 0 && i2 < int(vertices.size()));
		assert(i3 >= 0 && i3 < int(vertices.size()));
		assert(i1 != i2 && i1 != i3 && i2 != i3);

		int chopEdge = getChopEdge(v1.getPosition(), v2.getPosition(), v3.getPosition());
		if(chopEdge == -1)
		{
			++i;
			continue;
		}

		int i4 = vertices.size();
		Vertex v4;

		Face f1 = face;
		Face f2 = face;

		double originalArea = getArea(v1, v2, v3);

		if(chopEdge == 0)
		{
			v4 = createNewVertex(v1, v2);

			f1.setVertexIndex(0, i1);
			f1.setVertexIndex(1, i4);
			f1.setVertexIndex(2, i3);
			f2.setVertexIndex(0, i4);
			f2.setVertexIndex(1, i2);
			f2.setVertexIndex(2, i3);

			double v1v2 = v1.getPosition().GetRangeTo(v2.getPosition());
			double v1v4 = v1.getPosition().GetRangeTo(v4.getPosition());
			double v2v4 = v2.getPosition().GetRangeTo(v4.getPosition());
			assert(v1v4 < v1v2);
			assert(v2v4 < v1v2);

			if(originalArea > 0.00001f)
			{
				assert(getArea(v1, v4, v3) <= originalArea);
				assert(getArea(v4, v2, v3) <= originalArea);
			}
		}
		else if(chopEdge == 1)
		{
			v4 = createNewVertex(v2, v3);

			f1.setVertexIndex(0, i1);
			f1.setVertexIndex(1, i2);
			f1.setVertexIndex(2, i4);
			//f2.setVertexIndex(0, i1);
			//f2.setVertexIndex(1, i4);
			//f2.setVertexIndex(2, i3);
			f2.setVertexIndex(0, i4);
			f2.setVertexIndex(1, i3);
			f2.setVertexIndex(2, i1);

			double v2v3 = v2.getPosition().GetRangeTo(v3.getPosition());
			double v2v4 = v2.getPosition().GetRangeTo(v4.getPosition());
			double v3v4 = v3.getPosition().GetRangeTo(v4.getPosition());
			assert(v2v4 < v2v3);
			assert(v3v4 < v2v3);

			if(originalArea > 0.00001f)
			{
				assert(getArea(v1, v2, v4) <= originalArea);
				assert(getArea(v1, v4, v3) <= originalArea);
			}
		}
		else if(chopEdge == 2)
		{
			v4 = createNewVertex(v3, v1);

			f1.setVertexIndex(0, i1);
			f1.setVertexIndex(1, i2);
			f1.setVertexIndex(2, i4);
			f2.setVertexIndex(0, i2);
			f2.setVertexIndex(1, i3);
			f2.setVertexIndex(2, i4);

			double v3v1 = v3.getPosition().GetRangeTo(v1.getPosition());
			double v1v4 = v1.getPosition().GetRangeTo(v4.getPosition());
			double v3v4 = v3.getPosition().GetRangeTo(v4.getPosition());
			assert(v1v4 < v3v1);
			assert(v3v4 < v3v1);

			if(originalArea > 0.00001f)
			{
				assert(getArea(v1, v2, v4) <= originalArea);
				assert(getArea(v2, v3, v4) <= originalArea);
			}
		}
		else
		{
			assert(!"...");
			continue;
		}

		assert(!(f1.getVertexIndex(0) == face.getVertexIndex(0) && f1.getVertexIndex(1) == face.getVertexIndex(1) && f1.getVertexIndex(2) == face.getVertexIndex(2)));
		assert(!(f2.getVertexIndex(0) == face.getVertexIndex(0) && f2.getVertexIndex(1) == face.getVertexIndex(1) && f2.getVertexIndex(2) == face.getVertexIndex(2)));

		vertices.push_back(v4);
		faces[i] = f1;

		faces.push_back(f2);
	}
}

void chopObjects(std::vector<boost::shared_ptr<Object> > &objects)
{
	/*
	// Do this only if our model is big enough
	{
		bool needChop = false;
		for(unsigned int i = 0; i < objects.size(); ++i)
		{
			if(objects[i].hasCollisionVisibility())
				continue;
			if(!objects[i].hasCameraVisibility())
				continue;

			double size = calculateSize(objects[i]);
			if(size > 10.f)
			{
				needChop = true;
				break;
			}
		}

		if(!needChop)
			return;
	}
	*/
	if(!needChop(objects))
		return;

	std::vector<boost::shared_ptr<Object> > newObjects;
	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		Object &object = *objects[i];
		chopObject(object, newObjects);

		object = Object();
	}

	objects = newObjects;
}

bool needChop(std::vector<boost::shared_ptr<Object> > &objects)
{
	bool needChop = false;
	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		if(objects[i]->hasCameraVisibility() || !objects[i]->hasCollisionVisibility())
		{
			double size = calculateSize(*objects[i]);
			if(size > 10.f)
			{
				needChop = true;
				break;
			}
		}
	}

	return needChop;
}

void chopObjectToLimits(std::vector<boost::shared_ptr<Object> > &objects)
{
	std::vector<boost::shared_ptr<Object> > newObjects;

	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		Object &object = *objects[i];
		chopObjectToLimits(object, newObjects);

		object = Object();
	}

	objects = newObjects;
}

void removeJunctions(std::vector<boost::shared_ptr<Object> > &objects)
{
#if FB_FAST_BUILD
	return;
#endif

	// Get size
	double size = 0.f;
	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];

		double tempSize = calculateSizeImp(object, object.getTm());
		if(tempSize > size)
			size = tempSize;
	}

	// Build tree
	JunctionTree tree(VC2(-float(size),-float(size)), VC2(float(size), float(size)));
	std::vector<JunctionObject> junctionObjects(objects.size());

	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];
		std::vector<Face> &faces = object.getFaces();
		std::vector<Vertex> &vertices = object.getVertices();

		JunctionObject &junctionObject = junctionObjects[o];
		junctionObject.tm = object.getTm();
		junctionObject.inverseTm = junctionObject.tm.GetInverse();
		//junctionObject.faces.reserve(faces.size());

		for(unsigned int f = 0; f < faces.size(); ++f)
		{
			junctionObject.faces.push_back(JunctionFace(o, f));
			JunctionFace *face = &junctionObject.faces[f];

			face->v1 = vertices[faces[f].getVertexIndex(0)];
			face->v2 = vertices[faces[f].getVertexIndex(1)];
			face->v3 = vertices[faces[f].getVertexIndex(2)];
			face->transform(object.getTm());
			face->update();

			face->entity = tree.insert(face, convert(face->center), float(face->radius));
		}
	}

	std::vector<JunctionFace *> collisions;
	collisions.reserve(100);

	// Do the magic
	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];
		std::vector<Vertex> &vertices = object.getVertices();

		int vertexAmount = vertices.size();
		for(int v = 0; v < vertexAmount; ++v)
		//for(unsigned int v = 0; v < vertices.size(); ++v)
		{
			const Vertex &vertex = vertices[v];
			FBVector point = vertex.getPosition();
			junctionObjects[o].tm.TransformVector(point);

			collisions.clear();
			tree.collectSphere(collisions, convert(point), 0.02f);

			int collisionAmount = collisions.size();
			for(int f = 0; f < collisionAmount; ++f)
			{
				JunctionFace *face = collisions[f];
				int edge = face->getJunction(point);
				if(edge == -1)
					continue;

				Object &targetObject = *objects[face->objectIndex];
				std::vector<Vertex> &targetVertices = targetObject.getVertices();
				std::vector<Face> &targetFaces = targetObject.getFaces();
				Face &realFace = targetFaces[face->faceIndex];

				int newVertexIndex = targetVertices.size();
				int fi1 = realFace.getVertexIndex(0);
				int fi2 = realFace.getVertexIndex(1);
				int fi3 = realFace.getVertexIndex(2);

				int i10 = 0;
				int i11 = 0;
				int i12 = 0;
				int i20 = 0;
				int i21 = 0;
				int i22 = 0;

				Vertex newVertex;
				if(edge == 0)
				{
					newVertex = createVertex(face->v1, face->v2, point);
					i10 = fi1;
					i11 = newVertexIndex;
					i12 = fi3;
					i20 = newVertexIndex;
					i21 = fi2;
					i22 = fi3;
				}
				else if(edge == 1)
				{
					newVertex = createVertex(face->v2, face->v3, point);
					i10 = fi1;
					i11 = fi2;
					i12 = newVertexIndex;
					i20 = fi1;
					i21 = newVertexIndex;
					i22 = fi3;
				}
				else if(edge == 2)
				{
					newVertex = createVertex(face->v3, face->v1, point);
					i10 = fi1;
					i11 = fi2;
					i12 = newVertexIndex;
					i20 = newVertexIndex;
					i21 = fi2;
					i22 = fi3;
				}

				// Create extra vertex
				JunctionObject &junctionObject = junctionObjects[face->objectIndex];
				FBVector newPosition = newVertex.getPosition();
				junctionObject.inverseTm.TransformVector(newPosition);
				newVertex.setPosition(newPosition);
				targetVertices.push_back(newVertex);

				// Update original face to first part
				realFace.setVertexIndex(0, i10);
				realFace.setVertexIndex(1, i11);
				realFace.setVertexIndex(2, i12);
				face->v1 = targetVertices[i10];
				face->v2 = targetVertices[i11];
				face->v3 = targetVertices[i12];
				face->transform(junctionObject.tm);
				face->update();
				face->entity->setPosition(convert(face->center));
				face->entity->setRadius(float(face->radius));

				// Create new face for second part
				int newFaceIndex = targetFaces.size();
				Face newRealFace = realFace;
				newRealFace.setVertexIndex(0, i20);
				newRealFace.setVertexIndex(1, i21);
				newRealFace.setVertexIndex(2, i22);
				targetFaces.push_back(newRealFace);

				junctionObject.faces.push_back(JunctionFace(face->objectIndex, newFaceIndex));
				JunctionFace *newFace = &junctionObject.faces[newFaceIndex];
				newFace->v1 = targetVertices[i20];
				newFace->v2 = targetVertices[i21];
				newFace->v3 = targetVertices[i22];
				newFace->transform(junctionObject.tm);
				newFace->update();
				newFace->entity = tree.insert(newFace, convert(newFace->center), float(newFace->radius));
			}
		}
	}
}

void snapVertices(std::vector<boost::shared_ptr<Object> > &objects)
{

#if FB_FAST_BUILD
	return;
#endif

	double size = 0.f;

	// Get size
	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		double tempSize = calculateSize(*objects[o]);
		if(tempSize > size)
			size = tempSize;
	}

	// Build tree

	SnapTree tree(VC2(-float(size),-float(size)), VC2(float(size), float(size)));
	std::vector<SnapObject> snapObjects(objects.size());

	std::ofstream stream("snap.txt");

	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];
		std::vector<Vertex> &vertices = object.getVertices();

		SnapObject &snapObject = snapObjects[o];
		snapObject.tm = object.getTm();
		snapObject.inverseTm = snapObject.tm.GetInverse();

		for(unsigned int v = 0; v < vertices.size(); ++v)
		{
			snapObject.vertices.push_back(SnapVertex(o, v));
			SnapVertex *vertex = &snapObject.vertices[v];

			vertex->position = vertices[v].getPosition();
			vertex->transform(snapObject.tm);
			vertex->entity = tree.insert(vertex, convert(vertex->position), 0.01f);
		}
	}

	std::vector<SnapVertex *> collisions;
	collisions.reserve(100);

	// Do the magic
	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];
		SnapObject &snapObject = snapObjects[o];
		std::vector<Vertex> &vertices = object.getVertices();

		int vertexAmount = vertices.size();
		for(int v = 0; v < vertexAmount; ++v)
		{
			FBVector point = vertices[v].getPosition();
			snapObject.tm.TransformVector(point);

			collisions.clear();
			tree.collectSphere(collisions, convert(point), 0.0005f); // half millimeter

			int collisionAmount = collisions.size();
			for(int i = 0; i < collisionAmount; ++i)
			{
				SnapVertex *vertex = collisions[i];
				if(vertex->position.GetRangeTo(point) > 0.0005f)
					continue;

				vertex->position = point;
				vertex->entity->setPosition(convert(vertex->position));
			}
		}
	}

	for(unsigned int o = 0; o < objects.size(); ++o)
	{
		Object &object = *objects[o];
		SnapObject &snapObject = snapObjects[o];

		std::vector<Vertex> &vertices = object.getVertices();
		std::deque<SnapVertex> &snapVertices = snapObject.vertices;

		int vertexAmount = vertices.size();
		for(int v = 0; v < vertexAmount; ++v)
		{
			FBVector pos = snapVertices[v].position;
			snapObject.inverseTm.TransformVector(pos);

			vertices[v].setPosition(pos);
		}
	}
}

} // end of namespace export
} // end of namespace frozenbyte
