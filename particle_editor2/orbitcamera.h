// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef ORBIT_CAMERA_H
#define ORBIT_CAMERA_H

namespace frozenbyte {
namespace particle {

class OrbitCamera 
{
	Vector m_position;
	Vector m_target;
	float m_fov;

public:
	void setPosition(const Vector& v) 
	{
		m_position = v;
	}
	void setTarget(const Vector& v) 
	{
		m_target = v;
	}

	void pan(const Vector& v) 
	{
		m_position += v;
		m_target += v;
	}

	void truck(float amount) 
	{
		if(amount == 0.0f)
			return;
		Vector dir = m_target - m_position;
		if(dir.GetLength() <= amount) {
			amount = dir.GetLength() - 0.0001f;
		}
		dir.Normalize();
		m_position += dir * amount;
	}

	void orbit(float dx, float dy) 
	{
		Vector up(0.0f, 1.0f, 0.0f);
		Vector dir = m_target - m_position;
		dir.Normalize();
		Vector left = dir.GetCrossWith(up);
		left.Normalize();
		QUAT q1(up, dx);
		QUAT q2(left, dy);
		Matrix rx, ry;
		rx.CreateRotationMatrix(q1);
		ry.CreateRotationMatrix(q2);
		Vector temp = m_position - m_target;
		rx.RotateVector(temp);
		ry.RotateVector(temp);
		m_position = m_target + temp;
	}

	void setFov(float fov) 
	{
		m_fov = fov; // fov in degrees!!!!
	}

	void apply(IStorm3D_Camera* camera) 
	{
		camera->SetPosition(m_position);
		camera->SetTarget(m_target);
		camera->SetFieldOfView(m_fov);
		camera->SetVisibilityRange(10000.0f);	
	}
};

} // particle
} // frozenbyte

#endif
