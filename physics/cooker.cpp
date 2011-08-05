
#include "precompiled.h"

#include "cooker.h"
#include "file_stream.h"
#include <IStorm3D_Model.h>
#include <IStorm3D_Mesh.h>

#include "NxPhysics.h"
#include "NxCooking.h"
#include <vector>
#include <deque>
#include "physics_lib.h"

#include "../system/Logger.h"

#if defined PHYSICS_PHYSX && defined _MSC_VER
#if NX_SDK_VERSION_BUGFIX > 1
#pragma comment(lib, "PhysXCooking.lib")
#else
#pragma comment(lib, "NxCooking.lib")
#endif
#endif

namespace frozenbyte {
namespace physics {
namespace {

	//const int CYLINDER_CIRCLE_POINTS = 10;
	const int CYLINDER_CIRCLE_POINTS = 9;
	const int CONVEX_PLANES = 14;
	const float CONVEX_OFFSET = 0.005f;
	const float MIN_CONVEX_THICKNESS = 0.1f;

	NxVec3 getIntersection(const Plane &p1, const Plane &p2, const Plane &p3)
	{
		VC3 m1 = VC3(p1.planenormal.x, p2.planenormal.x, p3.planenormal.x);
		VC3 m2 = VC3(p1.planenormal.y, p2.planenormal.y, p3.planenormal.y);
		VC3 m3 = VC3(p1.planenormal.z, p2.planenormal.z, p3.planenormal.z);

		VC3 u = m2.GetCrossWith(m3);
		float denom = m1.GetDotWith(u);
		if(fabsf(denom) < 0.001f)
		{
			assert(!"Whops. Illegal planes");
			return NxVec3();
		}

		VC3 d(p1.range_to_origin, p2.range_to_origin, p3.range_to_origin);
		VC3 v = m1.GetCrossWith(d);
		float ood = 1.f / denom;
		
		NxVec3 result;
		result.x = d.GetDotWith(u) * ood;
		result.y = m3.GetDotWith(v) * ood;
		result.z = -m2.GetDotWith(v) * ood;

		return result;
	}

	void generateVertices(std::vector<NxVec3> &vertices, const Plane *p)
	{
/*		// Bottom corners
		vertices.push_back(getIntersection(p[0], p[1], p[4]));
		vertices.push_back(getIntersection(p[1], p[2], p[4]));
		vertices.push_back(getIntersection(p[2], p[3], p[4]));
		vertices.push_back(getIntersection(p[3], p[0], p[4]));

		// Top corners
		vertices.push_back(getIntersection(p[0], p[1], p[5]));
		vertices.push_back(getIntersection(p[1], p[2], p[5]));
		vertices.push_back(getIntersection(p[2], p[3], p[5]));
		vertices.push_back(getIntersection(p[3], p[0], p[5]));
*/

		// Bottom corners
		vertices.push_back(getIntersection(p[6], p[4], p[0]));
		vertices.push_back(getIntersection(p[6], p[4], p[1]));
		vertices.push_back(getIntersection(p[6], p[0], p[1]));

		vertices.push_back(getIntersection(p[7], p[4], p[1]));
		vertices.push_back(getIntersection(p[7], p[4], p[2]));
		vertices.push_back(getIntersection(p[7], p[1], p[2]));

		vertices.push_back(getIntersection(p[8], p[4], p[2]));
		vertices.push_back(getIntersection(p[8], p[4], p[3]));
		vertices.push_back(getIntersection(p[8], p[2], p[3]));

		vertices.push_back(getIntersection(p[9], p[4], p[3]));
		vertices.push_back(getIntersection(p[9], p[4], p[0]));
		vertices.push_back(getIntersection(p[9], p[3], p[0]));

		// Top corners

		vertices.push_back(getIntersection(p[10], p[5], p[0]));
		vertices.push_back(getIntersection(p[10], p[5], p[1]));
		vertices.push_back(getIntersection(p[10], p[0], p[1]));

		vertices.push_back(getIntersection(p[11], p[5], p[1]));
		vertices.push_back(getIntersection(p[11], p[5], p[2]));
		vertices.push_back(getIntersection(p[11], p[1], p[2]));

		vertices.push_back(getIntersection(p[12], p[5], p[2]));
		vertices.push_back(getIntersection(p[12], p[5], p[3]));
		vertices.push_back(getIntersection(p[12], p[2], p[3]));

		vertices.push_back(getIntersection(p[13], p[5], p[3]));
		vertices.push_back(getIntersection(p[13], p[5], p[0]));
		vertices.push_back(getIntersection(p[13], p[3], p[0]));
	}

	bool testConvexity(const std::vector<NxVec3> &vertices, const Plane &p, int skipIndex, const MAT &tm)
	{
		int first = skipIndex * 3;
		int last = first + 3;

		for(int i = 0; i < int(vertices.size()); ++i)
		{
			if(i >= first && i < last)
				continue;

			//const NxVec3 &pos = vertices[i];
			//float range = p.GetPointRange(VC3(pos.x, pos.y, pos.z));
			const NxVec3 &nxPos = vertices[i];
			VC3 pos(nxPos.x, nxPos.y, nxPos.z);
			tm.TransformVector(pos);
			float range = p.GetPointRange(pos);

			if(range <= 0)
				return false;
		}

		return true;
	}

	struct ConvexCorner
	{
		NxVec3 cut[3];
		NxVec3 corner;
		float value;

		ConvexCorner()
		:	value(0.f)
		{
		}
	};

	struct ConvexCornerSorter
	{
		bool operator () (const ConvexCorner &a, const ConvexCorner &b) const
		{
			return a.value > b.value;
		}
	};

	void generateFinalVertices(std::vector<NxVec3> &vertices, const Plane *p)
	{
		std::vector<ConvexCorner> corners(8);

		// Corners
		corners[0].corner = getIntersection(p[0], p[1], p[4]);
		corners[1].corner =	getIntersection(p[1], p[2], p[4]);
		corners[2].corner = getIntersection(p[2], p[3], p[4]);
		corners[3].corner = getIntersection(p[3], p[0], p[4]);

		corners[4].corner = getIntersection(p[0], p[1], p[5]);
		corners[5].corner = getIntersection(p[1], p[2], p[5]);
		corners[6].corner = getIntersection(p[3], p[0], p[5]);
		corners[7].corner = getIntersection(p[3], p[0], p[5]);

		// Cut corners

		corners[0].cut[0] = getIntersection(p[6], p[4], p[0]);
		corners[0].cut[1] = getIntersection(p[6], p[4], p[1]);
		corners[0].cut[2] = getIntersection(p[6], p[0], p[1]);

		corners[1].cut[0] = getIntersection(p[7], p[4], p[1]);
		corners[1].cut[1] = getIntersection(p[7], p[4], p[2]);
		corners[1].cut[2] = getIntersection(p[7], p[1], p[2]);

		corners[2].cut[0] = getIntersection(p[8], p[4], p[2]);
		corners[2].cut[1] = getIntersection(p[8], p[4], p[3]);
		corners[2].cut[2] = getIntersection(p[8], p[2], p[3]);

		corners[3].cut[0] = getIntersection(p[9], p[4], p[3]);
		corners[3].cut[1] = getIntersection(p[9], p[4], p[0]);
		corners[3].cut[2] = getIntersection(p[9], p[3], p[0]);

		corners[4].cut[0] = getIntersection(p[10], p[5], p[0]);
		corners[4].cut[1] = getIntersection(p[10], p[5], p[1]);
		corners[4].cut[2] = getIntersection(p[10], p[0], p[1]);

		corners[5].cut[0] = getIntersection(p[11], p[5], p[1]);
		corners[5].cut[1] = getIntersection(p[11], p[5], p[2]);
		corners[5].cut[2] = getIntersection(p[11], p[1], p[2]);

		corners[6].cut[0] = getIntersection(p[12], p[5], p[2]);
		corners[6].cut[1] = getIntersection(p[12], p[5], p[3]);
		corners[6].cut[2] = getIntersection(p[12], p[2], p[3]);

		corners[7].cut[0] = getIntersection(p[13], p[5], p[3]);
		corners[7].cut[1] = getIntersection(p[13], p[5], p[0]);
		corners[7].cut[2] = getIntersection(p[13], p[3], p[0]);

		// Calculate how much plane has cut out from the original box volume
		for(int i = 0; i < 8; ++i)
		{
			ConvexCorner &c = corners[i];
			VC3 cut1(c.cut[0].x, c.cut[0].y, c.cut[0].z);
			VC3 cut2(c.cut[1].x, c.cut[1].y, c.cut[1].z);
			VC3 cut3(c.cut[2].x, c.cut[2].y, c.cut[2].z);

			Plane p;
			p.MakeFromPoints(cut1, cut2, cut3);

			VC3 corner(c.corner.x, c.corner.y, c.corner.z);
			c.value = fabsf(p.GetPointRange(corner));
		}

		// Sort corners based on previous metric -- leave 3 worst cuts as original corner point
		std::sort(corners.begin(), corners.end(), ConvexCornerSorter());
		for(int i = 0; i < 8; ++i)
		{
			const ConvexCorner &c = corners[i];

			if(i < 5)
			{
				vertices.push_back(c.cut[0]);
				vertices.push_back(c.cut[1]);
				vertices.push_back(c.cut[2]);
			}
			else
			{
				vertices.push_back(c.corner);
			}
		}
	}

	int getClippedVertexValue(const unsigned short *heightmap, const unsigned char *clipmap, const VC2I &resolution, int x, int y)
	{
		int xmin = (x > 0) ? x - 1 : 0;
		int xmax = (x < resolution.x - 1) ? x + 1 : resolution.x - 1;
		int ymin = (y > 0) ? y - 1 : 0;
		int ymax = (y < resolution.y - 1) ? y + 1 : resolution.y - 1;
		
		int clippedVertices = 0;
		for(int j = ymin; j <= ymax; ++j)
		for(int i = xmin; i <= xmax; ++i)
		{
			if(clipmap[j * resolution.x + i])
				++clippedVertices;
		}

		return clippedVertices;
	}

	bool hasJunction(const unsigned short *heightmap, const VC2I &resolution, int x, int y)
	{
		int xmin = (x > 0) ? x - 1 : 0;
		int xmax = (x < resolution.x - 1) ? x + 1 : resolution.x - 1;
		int ymin = (y > 0) ? y - 1 : 0;
		int ymax = (y < resolution.y - 1) ? y + 1 : resolution.y - 1;
		
		int height = heightmap[y * resolution.x + x];
		for(int j = ymin; j <= ymax; ++j)
		for(int i = xmin; i <= xmax; ++i)
		{
			if(heightmap[j * resolution.x + i] != height)
				return true;
		}

		return false;
	}

} // unnamed

NxUserOutputStream *getLogger();

struct Cooker::Data
{
	PhysicsLogger logger;
	bool cookingInitialized;

	NxCookingInterface *cooker;

	Data()
	:	cookingInitialized(false),
		cooker(0)
	{
	}

	~Data()
	{
		if(cookingInitialized)
		{
			//NxCloseCooking();
			if(cooker)
				cooker->NxCloseCooking();
		}
	}

	bool initCooking()
	{
		if(!cookingInitialized)
		{
			cooker = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
			if(!cooker)
				return false;

			cookingInitialized = true;

			//cooker->NxInitCooking(0, &logger);
			cooker->NxInitCooking(0, getLogger());

			//NxCookingParams parameters = cooker->NxGetCookingParams();
			//parameters.skinWidth = 0.20f;
			//cooker->NxSetCookingParams(parameters);
		}

		return true;
	}

};

Cooker::Cooker()
:	data(new Data())
{
}

Cooker::~Cooker()
{
}

bool Cooker::cookMesh(const char *filename, IStorm3D_Model *model)
{
	if(!data->initCooking())
	{
		::Logger::getInstance()->error("Cooker::cookMesh - cooking init failed.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	// HAX -- The thing that should not be ...
	bool useHeightHax = false;
#ifdef PROJECT_SHADOWGROUDS
	{
		std::string filenameString = filename;
		for(std::string::size_type i = 0; i < filenameString.size(); ++i)
			filenameString[i] = tolower(filenameString[i]);

		if(strstr(filenameString.c_str(), "transformergrid."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "transformergrid@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "repairhall."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "repairhall@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "storagehall."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "storagehall@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "securityoutpost_small."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "securityoutpost_small@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "wtf_offices."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "wtf_offices@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "emergencypowerplant."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "emergencypowerplant@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "radarfacility."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "radarfacility@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "radarfacility_toplevel."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "radarfacility_toplevel@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "conference_building."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "conference_building@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "entrance."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "entrance@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "greenhouses."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "greenhouses@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "landingplatform."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "landingplatform@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "living_quarters@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "living_quarters."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "living_quarters_part2."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "living_quarters_part2@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "messhall."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "messhall@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "researchcenter_highsecurity."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "researchcenter_highsecurity@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "researchcenter."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "researchcenter@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "elevatorentrancehall."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "elevatorentrancehall@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienhive_part2."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienhive_part2@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienhive_part4."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienhive_part4@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level1."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level1@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level2."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level2@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level3."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "provectus_level3@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "smc_part2."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "smc_part2@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienstoragechamper."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienstoragechamper@"))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienlifesupportchamper."))
			useHeightHax = true;
		else if(strstr(filenameString.c_str(), "alienlifesupportchamper@"))
			useHeightHax = true;
	}
#endif

	// Create geometry
	std::vector<NxVec3> vertices;
	std::vector<unsigned int> indices;

	boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
	for(; !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		if(!object)
			continue;

		bool firethrough = false;
		bool noPhysics = false;
		if(strstr(object->GetName(), "FireThrough") != 0)
			firethrough = true;

		if(strstr(object->GetName(), "NoPhysics") != 0)
			noPhysics = true;

		if(!firethrough && object->GetNoCollision())
			continue;

		if(noPhysics)
			continue;

		IStorm3D_Mesh *mesh = object->GetMesh();
		if(!mesh)
			continue;

		//MAT tm = object->GetMXG();

		const Storm3D_Face *faceBuffer = mesh->GetFaceBufferReadOnly();
		const Storm3D_Vertex *vertexBuffer = mesh->GetVertexBufferReadOnly();

		int faceAmount = mesh->GetFaceCount();
		int vertexAmount = mesh->GetVertexCount();
		int vertexOffset = vertices.size();

		for(int i = 0; i < vertexAmount; ++i)
		{
			VC3 p = vertexBuffer[i].position;
			//tm.TransformVector(p);

			NxVec3 vec(p.x, p.y, p.z);			
			vertices.push_back(vec);
		}

		for(int i = 0; i < faceAmount; ++i)
		{
			const Storm3D_Face &f = faceBuffer[i];

			if(useHeightHax)
			{
				const VC3 &p1 = vertexBuffer[f.vertex_index[0]].position;
				const VC3 &p2 = vertexBuffer[f.vertex_index[1]].position;
				const VC3 &p3 = vertexBuffer[f.vertex_index[2]].position;

				float hackValue = -5.0f;
				if(p1.y < hackValue && p2.y < hackValue && p3.y < hackValue)
				{
					VC3 normal = (p1 - p2).GetCrossWith(p2 - p3);
					normal.Normalize();

					if(fabsf(normal.y - 1.f) < 0.1f)
						continue;
				}
			}

			indices.push_back(f.vertex_index[0] + vertexOffset);
			indices.push_back(f.vertex_index[1] + vertexOffset);
			indices.push_back(f.vertex_index[2] + vertexOffset);

			if(firethrough)
			{
				indices.push_back(f.vertex_index[0] + vertexOffset);
				indices.push_back(f.vertex_index[2] + vertexOffset);
				indices.push_back(f.vertex_index[1] + vertexOffset);
			}
		}
	}

	if(vertices.size() < 1)
	{
		// No vertices to cook
		return false;
	}

	for(unsigned int i = 0; i < indices.size(); ++i)
	{
		assert(indices[i] >= 0 && indices[i] < vertices.size());
	}

	// Cook
	NxTriangleMeshDesc meshDesc;
	meshDesc.numVertices				= vertices.size();
	meshDesc.pointStrideBytes		= sizeof(NxVec3);
	meshDesc.points					= &vertices[0];
	meshDesc.numTriangles			= indices.size() / 3;
	meshDesc.triangleStrideBytes	= 3 * sizeof(unsigned int);
	meshDesc.triangles				= &indices[0];
	meshDesc.flags					= NX_MF_HARDWARE_MESH; //NX_MF_FLIPNORMALS;

	//if(meshDesc.numTriangles > 50)
	//	meshDesc.numTriangles = 50;

	if(meshDesc.numVertices > 0 && meshDesc.numTriangles > 0)
	{
		OutputPhysicsStream physStream(filename);
		return data->cooker->NxCookTriangleMesh(meshDesc, physStream);
	}
	else
	{
		::Logger::getInstance()->error("Cooker::cookMesh - cannot cook model without collision mesh.");
		::Logger::getInstance()->error(filename);
		return false;
	}
}

bool Cooker::cookHeightmap(const unsigned short *heightmap, const unsigned char *clipmap, const VC2I &resolution, const VC3 &size, const char *filename)
{
	if(!data->initCooking())
	{
		::Logger::getInstance()->error("Cooker::cookHeightmap - cooking init failed.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	// Create geometry
	std::vector<NxVec3> vertices;
	std::vector<unsigned int> indices;

	VC3 scale;
	scale.x = 1.f / float(resolution.x) * size.x;
	scale.y = size.y / 65535.f;
	scale.z = 1.f / float(resolution.y) * size.z;

/*
	//--------------------------------------
	// Stupid brute version
	// Add all heightmap samples as polygons
	//--------------------------------------

	std::vector<int> realVertexIndex(resolution.x * resolution.y);
	int currentIndex = 0;

	// Vertices
	for(int y = 0; y < resolution.y; ++y)
	for(int x = 0; x < resolution.x; ++x)
	{
		int index = y * resolution.x + x;

		int xmin = (x > 0) ? x - 1 : 0;
		int xmax = (x < resolution.x - 1) ? x + 1 : resolution.x - 1;
		int ymin = (y > 0) ? y - 1 : 0;
		int ymax = (y < resolution.y - 1) ? y + 1 : resolution.y - 1;
		
		bool needed = false;
		for(int j = ymin; j <= ymax; ++j)
		for(int i = xmin; i <= xmax; ++i)
		{
			if(!clipmap[j * resolution.x + i])
				needed = true;
		}

		//if(needed)
		{
			NxVec3 v;
			v.x = float(x) * scale.x - (size.x * 0.5f);
			v.y = float(heightmap[index]) * scale.y;
			v.z = float(y) * scale.z - (size.z * 0.5f);

			vertices.push_back(v);
			realVertexIndex[index] = currentIndex++;
		}
	}

	// Indices
	for(int y = 1; y < resolution.y; ++y)
	for(int x = 1; x < resolution.x; ++x)
	{
		int index = y * resolution.x + x;
		int i1 = index - resolution.x - 1;
		int i2 = index - resolution.x;
		int i3 = index - 1;
		int i4 = index;

		if(!clipmap[i1] || !clipmap[i4] || !clipmap[i2])
		{
			indices.push_back(realVertexIndex[i1]);
			indices.push_back(realVertexIndex[i4]);
			indices.push_back(realVertexIndex[i2]);
		}

		if(!clipmap[i4] || !clipmap[i1] || !clipmap[i3])
		{
			indices.push_back(realVertexIndex[i4]);
			indices.push_back(realVertexIndex[i1]);
			indices.push_back(realVertexIndex[i3]);
		}
	}
*/
	//-------------------------------------------------------------------
	// Better one:
	// Find all discontinuities and create polygons to these line by line
	//-------------------------------------------------------------------

	std::vector<int> realVertexIndex(resolution.x * resolution.y, -1);
	int currentIndex = 0;

	// Vertices
	for(int y = 0; y < resolution.y; ++y)
	for(int x = 0; x < resolution.x; ++x)
	{
		int index = y * resolution.x + x;
		/*
		bool needed = false;

		if(hasJunction(heightmap, resolution, x, y))
			needed = true;
		if(x == 0 || x == resolution.x - 1 || y == 0 || y == resolution.y - 1)
			needed = true;

		int clippedValue = getClippedVertexValue(heightmap, clipmap, resolution, x, y);
		if(clippedValue == 8)
			needed = false;
		else if(clippedValue)
			needed = true;

		if(needed)
		*/

		// Could insert vertices as needed on index creation but there is no point
		// as cooker seems to remove unused ones anyway.
		{
			NxVec3 v;
			v.x = float(x) * scale.x - (size.x * 0.5f);
			v.y = float(heightmap[index]) * scale.y;
			v.z = float(y) * scale.z - (size.z * 0.5f);

			realVertexIndex[index] = currentIndex++;
			vertices.push_back(v);
		}
	}

	// Indices
	for(int y = 0; y < resolution.y - 1; ++y)
	for(int x = 0; x < resolution.x - 1; )
	{
		int index = y * resolution.x + x;
		int value1 = heightmap[index];
		int value2 = heightmap[index + resolution.x];
		int e = x + 1;
		int e1 = e;
		int e2 = e;

		std::deque<VC2I> pointsUp;
		pointsUp.push_back(VC2I(x, y));
		std::deque<VC2I> pointsDown;
		pointsDown.push_back(VC2I(x, y + 1));

		// How far do we have same heights
		{
			bool startClipped = false;
			if(clipmap[index] || clipmap[index + resolution.x])
				startClipped = true;

			if(!startClipped)
			{
				for(e1 = x + 1; e1 < resolution.x - 1; ++e1)
				{
					if(heightmap[y * resolution.x + e1] != value1)
					{
						if(e1 > x + 1)
							--e1;
						break;
					}

					if(clipmap[y * resolution.x + e1])
						break;
				}

				for(e2 = x + 1; e2 < resolution.x - 1; ++e2)
				{
					if(heightmap[(y + 1) * resolution.x + e2] != value2)
					{
						if(e2 > x + 1)
							--e2;
						break;
					}

					if(clipmap[(y + 1) * resolution.x + e2])
						break;
				}

				//e = std::min(e1, e2);
				e = e1;
				if(e2 < e)
					e = e2;
				assert(e < resolution.x);

				for(int i = x + 1; i < e; ++i)
				{
					if(hasJunction(heightmap, resolution, i, y))
						pointsUp.push_back(VC2I(i, y));
					if(hasJunction(heightmap, resolution, i, y + 1))
						pointsDown.push_back(VC2I(i, y + 1));
				}
			}

			pointsUp.push_back(VC2I(e, y));
			pointsDown.push_back(VC2I(e, y + 1));
		}

		for(; pointsUp.size() > 1 || pointsDown.size() > 1; )
		{
			assert(!pointsUp.empty() && !pointsDown.empty());
			int uDelta = 1000000;
			int dDelta = 1000000;

			// Decide which way to do polygon
			{
				if(pointsDown.size() >= 2)
				{
					const VC2I &p1 = pointsUp[0];
					const VC2I &p2 = pointsDown[1];
					uDelta = p2.x - p1.x;
				}
				else if(pointsUp.size() >= 2)
				{
					const VC2I &p1 = pointsDown[0];
					const VC2I &p2 = pointsUp[1];
					dDelta = p2.x - p1.x;
				}
			}

			const VC2I &pU1 = pointsUp[0];
			const VC2I &pD1 = pointsDown[0];

			if(uDelta <= dDelta)
			{
				const VC2I &pD2 = pointsDown[1];
				int i1 = pU1.y * resolution.x + pU1.x;
				int i2 = pD1.y * resolution.x + pD1.x;
				int i3 = pD2.y * resolution.x + pD2.x;

				if(!(clipmap[i1] && clipmap[i2] && clipmap[i3]))
				{
					int real1 = realVertexIndex[i1];
					int real2 = realVertexIndex[i2];
					int real3 = realVertexIndex[i3];
					assert(real1 >= 0 && real2 >= 0 && real3 >= 0);

					indices.push_back(real1);
					indices.push_back(real2);
					indices.push_back(real3);
				}

				pointsDown.pop_front();
			}
			else
			{
				const VC2I &pU2 = pointsUp[1];
				int i1 = pU1.y * resolution.x + pU1.x;
				int i2 = pD1.y * resolution.x + pD1.x;
				int i3 = pU2.y * resolution.x + pU2.x;

				if(!(clipmap[i1] && clipmap[i2] && clipmap[i3]))
				{
					int real1 = realVertexIndex[i1];
					int real2 = realVertexIndex[i2];
					int real3 = realVertexIndex[i3];
					assert(real1 >= 0 && real2 >= 0 && real3 >= 0);

					indices.push_back(real1);
					indices.push_back(real2);
					indices.push_back(real3);
				}

				pointsUp.pop_front();
			}
		}

		x = e;
	}

	// Cook
	NxTriangleMeshDesc meshDesc;
	meshDesc.numVertices			= vertices.size();
	meshDesc.pointStrideBytes		= sizeof(NxVec3);
	meshDesc.points					= &vertices[0];
	meshDesc.numTriangles			= indices.size() / 3;
	meshDesc.triangleStrideBytes	= 3 * sizeof(unsigned int);
	meshDesc.triangles				= &indices[0];
	meshDesc.flags					= NX_MF_HARDWARE_MESH; //NX_MF_FLIPNORMALS;

	OutputPhysicsStream s(filename);
	data->cooker->NxCookTriangleMesh(meshDesc, s);

	return true;
}

bool Cooker::cookCylinder(const char *filename, float height, float radius, float offset, int upvectorAxis)
{
	if(!data->initCooking())
	{
		::Logger::getInstance()->error("Cooker::cookCylinder - cooking init failed.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	// Create points
	int vertexAmount = CYLINDER_CIRCLE_POINTS * 2;
	std::vector<NxVec3> vertices(vertexAmount);

	for(int i = 0; i < CYLINDER_CIRCLE_POINTS; ++i)
	{
		float angle = 6.28f * float(i)/ float(CYLINDER_CIRCLE_POINTS);

		NxVec3 v;
		if (upvectorAxis == 0)
		{
			// x-axis
			v.x = 0.f + offset;
			v.y = cosf(angle) * radius;
			v.z = sinf(angle) * radius;

			vertices[i] = v;
			v.x = height + offset;
			vertices[i + CYLINDER_CIRCLE_POINTS] = v;
		}
		else if (upvectorAxis == 1)
		{
			// y-axis
			v.x = cosf(angle) * radius;
			v.y = 0.f + offset;
			v.z = sinf(angle) * radius;

			vertices[i] = v;
			v.y = height + offset;
			vertices[i + CYLINDER_CIRCLE_POINTS] = v;
		}
		else if (upvectorAxis == 2)
		{
			// z-axis
			v.x = cosf(angle) * radius;
			v.y = sinf(angle) * radius;
			v.z = 0.f + offset;

			vertices[i] = v;
			v.z = height + offset;
			vertices[i + CYLINDER_CIRCLE_POINTS] = v;
		}
	}

	// Cook
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= vertexAmount;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= &vertices[0];
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX;

	OutputPhysicsStream s(filename);
	/*bool status = */data->cooker->NxCookConvexMesh(convexDesc, s);

	// TODO: check status

	return true;
}

bool Cooker::cookApproxConvex(const char *filename, IStorm3D_Model_Object *object)
{
	if(!data->initCooking())
	{
		::Logger::getInstance()->error("Cooker::cookApproxConvex - cooking init failed.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	if(!object)
	{
		::Logger::getInstance()->error("Cooker::cookApproxConvex - null object parameter.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	IStorm3D_Mesh *mesh = object->GetMesh();
	if(!mesh)
	{
		::Logger::getInstance()->error("Cooker::cookApproxConvex - object has no mesh.");
		::Logger::getInstance()->debug(filename);
		return false;
	}

	MAT tm;
	float radius = mesh->GetRadius();
	Plane p[CONVEX_PLANES];

	// -z,-x,z,x,-y,y
	p[0].MakeFromNormalAndRange(VC3(0, 0, 1.f), -radius);
	p[1].MakeFromNormalAndRange(VC3(1, 0, 0), -radius);
	p[2].MakeFromNormalAndRange(VC3(0, 0, -1.f), -radius);
	p[3].MakeFromNormalAndRange(VC3(-1, 0, 0), -radius);
	p[4].MakeFromNormalAndRange(VC3(0, 1, 0), -radius);
	p[5].MakeFromNormalAndRange(VC3(0, -1, 0), -radius);

	// Bottom corners -z-x,z-x,zx,-z+x
	p[6].MakeFromNormalAndRange(VC3(1.f, 1.f, 1.f).GetNormalized(), -radius);
	p[7].MakeFromNormalAndRange(VC3(1.f, 1.f, -1.f).GetNormalized(), -radius);
	p[8].MakeFromNormalAndRange(VC3(-1.f, 1.f, -1.f).GetNormalized(), -radius);
	p[9].MakeFromNormalAndRange(VC3(-1.f, 1.f, 1.f).GetNormalized(), -radius);

	// Top corners -z-x,z-x,zx,-z+x
	p[10].MakeFromNormalAndRange(VC3(1.f, -1.f, 1.f).GetNormalized(), -radius);
	p[11].MakeFromNormalAndRange(VC3(1.f, -1.f, -1.f).GetNormalized(), -radius);
	p[12].MakeFromNormalAndRange(VC3(-1.f, -1.f, -1.f).GetNormalized(), -radius);
	p[13].MakeFromNormalAndRange(VC3(-1.f, -1.f, 1.f).GetNormalized(), -radius);

	std::vector<NxVec3> vertices;
	{
		int vertexAmount = mesh->GetVertexCount();
		const Storm3D_Vertex *vbuffer = mesh->GetVertexBufferReadOnly();

		for(;;)
		{
			bool anyMoved = false;

			for(int i = 0; i < 6; ++i)
			{
				Plane &current = p[i];

				Plane tempPlane = current;
				tempPlane.range_to_origin += CONVEX_OFFSET;

				bool ok = true;
				for(int j = 0; j < vertexAmount; ++j)
				{
					VC3 pos = vbuffer[j].position;
					tm.TransformVector(pos);

					if(tempPlane.GetPointRange(pos) <= 0)
					{
						ok = false;
						break;
					}
				}

				if(!ok)
					continue;

				if(i == 0)
				{
					float thick = -tempPlane.range_to_origin - p[2].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}
				else if(i == 2)
				{
					float thick = -tempPlane.range_to_origin - p[0].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}
				else if(i == 1)
				{
					float thick = -tempPlane.range_to_origin - p[3].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}
				else if(i == 3)
				{
					float thick = -tempPlane.range_to_origin - p[1].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}
				else if(i == 4)
				{
					float thick = -tempPlane.range_to_origin - p[5].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}
				else if(i == 5)
				{
					float thick = -tempPlane.range_to_origin - p[4].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						continue;
				}

				current = tempPlane;
				anyMoved = true;
			}

			if(!anyMoved)
				break;
		}

		/*
		// First move basic cube in place
		for(int i = 0; i < 6; ++i)
		{
			Plane &current = p[i];

			for(;;)
			{
				Plane tempPlane = current;
				tempPlane.range_to_origin += CONVEX_OFFSET;

				bool ok = true;
				for(int j = 0; j < vertexAmount; ++j)
				{
					VC3 pos = vbuffer[j].position;
					tm.TransformVector(pos);

					if(tempPlane.GetPointRange(pos) <= 0)
					{
						ok = false;
						break;
					}
				}

				if(!ok)
					break;

				if(i == 0)
				{
					float thick = -tempPlane.range_to_origin - p[2].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						break;
				}
				if(i == 2)
				{
					float thick = -tempPlane.range_to_origin - p[0].range_to_origin;
					if(thick < MIN_CONVEX_THICKNESS)
						break;
				}

				current = tempPlane;
			}
		}
		*/

		// Move corner planes to new cube positions
		{
			// Bottom corners
			p[6].range_to_origin = -getIntersection(p[0], p[1], p[4]).magnitude();
			p[7].range_to_origin = -getIntersection(p[1], p[2], p[4]).magnitude();
			p[8].range_to_origin = -getIntersection(p[2], p[3], p[4]).magnitude();
			p[9].range_to_origin = -getIntersection(p[3], p[0], p[4]).magnitude();
			// Top corners
			p[10].range_to_origin = -getIntersection(p[0], p[1], p[5]).magnitude();
			p[11].range_to_origin = -getIntersection(p[1], p[2], p[5]).magnitude();
			p[12].range_to_origin = -getIntersection(p[2], p[3], p[5]).magnitude();
			p[13].range_to_origin = -getIntersection(p[3], p[0], p[5]).magnitude();
		}

		Plane newPlanes[CONVEX_PLANES];
		for(int i = 0; i < CONVEX_PLANES; ++i)
			newPlanes[i] = p[i];

		// Move corners while retaining convexity
		for(;;)
		{
			bool moved = false;
			for(int i = 6; i < CONVEX_PLANES; ++i)
			{
				Plane &current = newPlanes[i];

				Plane tempPlane = current;
				tempPlane.range_to_origin += CONVEX_OFFSET;

				bool ok = true;
				for(int j = 0; j < vertexAmount; ++j)
				{
					VC3 pos = vbuffer[j].position;
					tm.TransformVector(pos);

					if(tempPlane.GetPointRange(pos) <= 0)
					{
						ok = false;
						break;
					}
				}

				if(ok)
				{
					current = tempPlane;
					moved = true;
				}
			}

			if(!moved)
				break;

			std::vector<NxVec3> testVertices;
			generateVertices(testVertices, newPlanes);

			if(!testConvexity(testVertices, newPlanes[6], 0, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[7], 1, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[8], 2, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[9], 3, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[10], 4, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[11], 5, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[12], 6, tm))
				break;
			if(!testConvexity(testVertices, newPlanes[13], 7, tm))
				break;

			for(int i = 0; i < CONVEX_PLANES; ++i)
				p[i] = newPlanes[i];
		}

		generateFinalVertices(vertices, p);
	}

#if 0

	// Move planes to their normal direction a small step at the time, revert if any vertex behind plane
	{
		int vertexAmount = mesh->GetVertexCount();
		const Storm3D_Vertex *vbuffer = mesh->GetVertexBufferReadOnly();

		for(int i = 0; i < CONVEX_PLANES; ++i)
		{
			Plane &current = p[i];

			for(;;)
			{
				float offset = 0.03f;

				Plane tempPlane = current;
				tempPlane.range_to_origin += offset;

				bool ok = true;
				for(int j = 0; j < vertexAmount; ++j)
				{
					const VC3 &pos = vbuffer[j].position;	
					if(tempPlane.GetPointRange(pos) <= offset)
					{
						ok = false;
						break;
					}
				}

				if(!ok)
					break;

				current = tempPlane;
			}
		}
	}

	// Create vertices
	std::vector<NxVec3> vertices;
	{
/*		// Bottom corners
		vertices.push_back(data->getIntersection(p[0], p[1], p[4]));
		vertices.push_back(data->getIntersection(p[1], p[2], p[4]));
		vertices.push_back(data->getIntersection(p[2], p[3], p[4]));
		vertices.push_back(data->getIntersection(p[3], p[0], p[4]));

		// Top corners
		vertices.push_back(data->getIntersection(p[0], p[1], p[5]));
		vertices.push_back(data->getIntersection(p[1], p[2], p[5]));
		vertices.push_back(data->getIntersection(p[2], p[3], p[5]));
		vertices.push_back(data->getIntersection(p[3], p[0], p[5]));
*/

		// Bottom corners
		vertices.push_back(data->getIntersection(p[6], p[4], p[0]));
		vertices.push_back(data->getIntersection(p[6], p[4], p[1]));
		vertices.push_back(data->getIntersection(p[6], p[0], p[1]));

		vertices.push_back(data->getIntersection(p[7], p[4], p[1]));
		vertices.push_back(data->getIntersection(p[7], p[4], p[2]));
		vertices.push_back(data->getIntersection(p[7], p[1], p[2]));

		vertices.push_back(data->getIntersection(p[8], p[4], p[2]));
		vertices.push_back(data->getIntersection(p[8], p[4], p[3]));
		vertices.push_back(data->getIntersection(p[8], p[2], p[3]));

		vertices.push_back(data->getIntersection(p[9], p[4], p[3]));
		vertices.push_back(data->getIntersection(p[9], p[4], p[0]));
		vertices.push_back(data->getIntersection(p[9], p[3], p[0]));

		// Top corners

		vertices.push_back(data->getIntersection(p[10], p[5], p[0]));
		vertices.push_back(data->getIntersection(p[10], p[5], p[1]));
		vertices.push_back(data->getIntersection(p[10], p[0], p[1]));

		vertices.push_back(data->getIntersection(p[11], p[5], p[1]));
		vertices.push_back(data->getIntersection(p[11], p[5], p[2]));
		vertices.push_back(data->getIntersection(p[11], p[1], p[2]));

		vertices.push_back(data->getIntersection(p[12], p[5], p[2]));
		vertices.push_back(data->getIntersection(p[12], p[5], p[3]));
		vertices.push_back(data->getIntersection(p[12], p[2], p[3]));

		vertices.push_back(data->getIntersection(p[13], p[5], p[3]));
		vertices.push_back(data->getIntersection(p[13], p[5], p[0]));
		vertices.push_back(data->getIntersection(p[13], p[3], p[0]));
	}

#endif

	// Cook
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= vertices.size();
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= &vertices[0];
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX;

	OutputPhysicsStream s(filename);
	/*bool status = */data->cooker->NxCookConvexMesh(convexDesc, s);

	// TODO: check status

	return true;
}

} // physics
} // frozenbyte
