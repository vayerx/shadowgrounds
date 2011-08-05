
#include "precompiled.h"

#include "ModelTextureUncompress.h"
#include "..\util\Debug_MemoryManager.h"

void ChangeToUncompressed(IStorm3D *storm3d, IStorm3D_Model *model)
{
	Iterator<IStorm3D_Model_Object *> *object_iterator;
	for(object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
	{
		IStorm3D_Model_Object *object = object_iterator->GetCurrent();
		IStorm3D_Material *material = object->GetMesh()->GetMaterial();
		if(material == NULL)
			continue;

		IStorm3D_Texture *t = material->GetBaseTexture();
		if(t == NULL)
			continue;

		const char *fname = t->GetFilename();
		IStorm3D_Texture *nt = storm3d->CreateNewTexture(fname, TEXLOADFLAGS_NOCOMPRESS);

		t->Release();
		material->SetBaseTexture(nt);
		material->SetSelfIllumination(Color(1,1,1));
	}
	
	delete object_iterator;
}

