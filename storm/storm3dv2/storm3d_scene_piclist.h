// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "storm3d_scene.h"

// Common datatype includes
#include "DatatypeDef.h"


//------------------------------------------------------------------
// Storm3D_Scene_PicList
//------------------------------------------------------------------
class Storm3D_Scene_PicList
{

protected:

	// Pointer to scene
	Storm3D_Scene *scene;

	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	VC3 position;	// 2d/3d
	VC2 size;

public:

	virtual void Render();

	Storm3D_Scene_PicList(Storm3D *Storm3D2,Storm3D_Scene *scene,VC2 position,VC2 size);	//2d
	Storm3D_Scene_PicList(Storm3D *Storm3D2,Storm3D_Scene *scene,VC3 position,VC2 size);		//3d
	virtual ~Storm3D_Scene_PicList() {}
};

//------------------------------------------------------------------
// Storm3D_Scene_PicList_Picture
//------------------------------------------------------------------
class Storm3D_Scene_PicList_Picture : public Storm3D_Scene_PicList
{
	Storm3D_Material *material;
	float alpha;
	float rotation;
	float x1;
	float y1;
	float x2;
	float y2;
	bool wrap;

public:

	struct CustomShape
	{
		// triangle list
		struct VXFORMAT_2D *vertices;
		int numVertices;
	};

	CustomShape *customShape;

public:

	// Virtual
	void Render();

	Storm3D_Scene_PicList_Picture(Storm3D *Storm3D2,Storm3D_Scene *scene,Storm3D_Material *mat,VC2 position,VC2 size,float alpha,float rotation,float x1,float y1,float x2,float y2,bool wrap);
	~Storm3D_Scene_PicList_Picture();

	void createCustomShape(struct VXFORMAT_2D *vertices, int numVertices);
};

//------------------------------------------------------------------
// Storm3D_Scene_PicList_Picture3D
//------------------------------------------------------------------
class Storm3D_Scene_PicList_Picture3D : public Storm3D_Scene_PicList
{
	Storm3D_Material *material;

public:

	// Virtual
	void Render();

	Storm3D_Scene_PicList_Picture3D(Storm3D *Storm3D2,Storm3D_Scene *scene,Storm3D_Material *mat,VC3 position,VC2 size);
};

//------------------------------------------------------------------
// Storm3D_Scene_PicList_Font
//------------------------------------------------------------------
class Storm3D_Scene_PicList_Font : public Storm3D_Scene_PicList
{
	Storm3D_Font *font;
	char *text;
	wchar_t *uniText;
	float alpha;
	COL colorFactor;
	unsigned int width, height;
	float wd, hd;

	boost::shared_ptr<glTexWrapper> tex;

public:

	// Virtual
	void Render();

	Storm3D_Scene_PicList_Font(Storm3D *Storm3D2,Storm3D_Scene *scene,Storm3D_Font *font,VC2 position,VC2 size,const char *text,float alpha,const COL &colorFactor);
	Storm3D_Scene_PicList_Font(Storm3D *Storm3D2,Storm3D_Scene *scene,Storm3D_Font *font,VC2 position,VC2 size,const wchar_t *text,float alpha,const COL &colorFactor);
	~Storm3D_Scene_PicList_Font();
};
