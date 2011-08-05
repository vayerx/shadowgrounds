
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "SelfIlluminationChanger.h"
#include <map>
#include <IStorm3D_Model.h>
#include <IStorm3D_Scene.h>
#include <IStorm3D_Material.h>
#include <IStorm3D_Mesh.h>

using namespace std;
using namespace boost;

namespace util {

struct Material
{
	COL illumination;
	float glow;

	Material()
	:	glow(0)
	{
	}

	explicit Material(const COL &color, float glow_)
	:	illumination(color),
		glow(glow_)
	{
	}
};

typedef map<IStorm3D_Material *, Material> MaterialList;


struct SelfIlluminationChanger::Data
{
	MaterialList materials;

	void add(IStorm3D_Model *model)
	{
		scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			IStorm3D_Mesh *mesh = object->GetMesh();
			if(!mesh)
				continue;

			IStorm3D_Material *material = mesh->GetMaterial();
			if(!material)
				continue;

			COL illum = material->GetSelfIllumination();
			if(illum.r < 0.01f && illum.g < 0.01f && illum.b < 0.01f)
				continue;

			float glow = material->GetGlow();

			// ToDo: 
			// Choose only certain materials -> name flag?

			MaterialList::iterator it = materials.find(material);
			if(it != materials.end())
				continue;

			materials[material] = Material(illum, glow);
		}
	}

	void setFactor(const COL &color)
	{
		float factor = (color.r + color.g + color.b) / 3.f;

		MaterialList::iterator it = materials.begin();
		for(; it != materials.end(); ++it)
		{
			IStorm3D_Material *m = it->first;
			const Material &ma = it->second;

			m->SetSelfIllumination(ma.illumination * color);
			m->SetGlow(ma.glow * factor);
		}
	}
};

SelfIlluminationChanger::SelfIlluminationChanger()
{
	scoped_ptr<Data> tempData(new Data());
	data.swap(tempData);
}

SelfIlluminationChanger::~SelfIlluminationChanger()
{
}

void SelfIlluminationChanger::addModel(IStorm3D_Model *model)
{
	if(model)
		data->add(model);
}

void SelfIlluminationChanger::addAllModels(IStorm3D_Scene *scene)
{
	if(!scene)
		return;

	scoped_ptr<Iterator<IStorm3D_Model *> > modelIterator(scene->ITModel->Begin());
	for(; !modelIterator->IsEnd(); modelIterator->Next())
	{
		IStorm3D_Model *model = modelIterator->GetCurrent();
		addModel(model);
	}
}

void SelfIlluminationChanger::clearModels()
{
	data->materials.clear();
}

void SelfIlluminationChanger::setFactor(const COL &color)
{
	data->setFactor(color);
}

} // util
