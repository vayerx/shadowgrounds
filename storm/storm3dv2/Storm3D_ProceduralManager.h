// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_PROCEDURAL_MANAGER_H
#define INCLUDED_STORM3D_PROCEDURAL_MANAGER_H

#pragma once

#include <IStorm3D_ProceduralManager.h>
#include <boost/scoped_ptr.hpp>
#include <atlbase.h>

struct IDirect3DTexture9;
class Storm3D;
class IStorm3D_Logger;

class Storm3D_ProceduralManager: public IStorm3D_ProceduralManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Storm3D_ProceduralManager(Storm3D &storm);
	~Storm3D_ProceduralManager();

	void setLogger(IStorm3D_Logger *logger);
	void setTarget(CComPtr<IDirect3DTexture9> &target, CComPtr<IDirect3DTexture9> &offsetTarget);
	void addEffect(const std::string &name, const Effect &effect);
	void enableDistortionMode(bool enable);
	void useFallback(bool fallback);

	void setActiveEffect(const std::string &name);
	void update(int ms);
	void apply(int stage);
	void applyOffset(int stage);
	bool hasDistortion() const;

	void releaseTarget();
	void reset();
};

#endif
