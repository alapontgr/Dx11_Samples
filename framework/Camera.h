#pragma once

#include "framework/Types.h"

namespace framework
{

	class Camera
	{
	public:

		Camera();

		void setTarget(const v3& cameraPos, const v3& target);

		void setPerspective(f32 fovDeg, f32 aspect, f32 nearPlane, f32 farPlane);

		void updateMatrices();

		m4 getView() const { return m_view; }

		m4 getProjection() const { return m_projection; }

		m4 getViewProj() const { return m_viewProjection; }

		m4 getInvViewProj() const { return m_invViewProjection; }

	protected:

		m4 m_view;
		m4 m_projection;
		m4 m_viewProjection;
		m4 m_invViewProjection;
	};

	class FirstPersonCamera : public Camera
	{
	public:
		FirstPersonCamera();

		void init(const v3& defaultPos, const v3& initialTarget, f32 moveSpeed, f32 rotSpeed);

		void update(f32 elapsedTime);

		v3 getForward() const { return m_forward; }

		v3 getPos() const { return m_pos; }

	private:

		f32 m_translationSpeed;
		f32 m_rotationSpeed;
		v3 m_pos;
		v3 m_forward;
		v3 m_currPitchYawRoll; // Angles
	};

}