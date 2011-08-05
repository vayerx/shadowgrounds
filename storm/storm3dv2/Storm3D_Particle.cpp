/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Particle

	- Particle "type"
	- Single particle in particlesystem

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Particle::Storm3D_Particle
//------------------------------------------------------------------
Storm3D_Particle::Storm3D_Particle() :
	lifetime(0),
	has_last_position(false)
{
}



//------------------------------------------------------------------
// Storm3D_Particle::Storm3D_Particle
//------------------------------------------------------------------
Storm3D_Particle::Storm3D_Particle(IStorm3D_Material *_material,
	int _lifetime,int _fadestart,float _size,float _stretch,
	int _rows,int _columns,int _framechangetime) :
	has_last_position(false),
	material((Storm3D_Material*)_material),
	lifetime(_lifetime),
	fadestart(_fadestart),
	size(_size),
	stretch(_stretch),
	rows(_rows),
	columns(_columns),
	framechangetime(_framechangetime),
	framechangecounter(0),
	frame(0)
{
}



//------------------------------------------------------------------
// Storm3D_Particle::SetMaterial
//------------------------------------------------------------------
void Storm3D_Particle::SetMaterial(IStorm3D_Material *mat)
{
	material=(Storm3D_Material*)mat;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetLifeTime
//------------------------------------------------------------------
void Storm3D_Particle::SetLifeTime(int milliseconds)
{
	lifetime=milliseconds;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetFadeStartTime
//------------------------------------------------------------------
void Storm3D_Particle::SetFadeStartTime(int milliseconds)
{
	fadestart=milliseconds;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetSize
//------------------------------------------------------------------
void Storm3D_Particle::SetSize(float _size)
{
	size=_size;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetStretching
//------------------------------------------------------------------
void Storm3D_Particle::SetStretching(float _stretch)
{
	stretch=_stretch;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetAnimationRowsAndColumns
//------------------------------------------------------------------
void Storm3D_Particle::SetAnimationRowsAndColumns(int _rows,int _columns)
{
	rows=_rows;
	columns=_columns;
}



//------------------------------------------------------------------
// Storm3D_Particle::SetAnimationFrameChangeTime
//------------------------------------------------------------------
void Storm3D_Particle::SetAnimationFrameChangeTime(int milliseconds)
{
	framechangetime=milliseconds;
}


