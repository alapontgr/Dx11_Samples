#include "framework/Framework.h"

namespace framework
{

	Camera::Camera()
		: m_view(1.0f)
		, m_projection(1.0f)
		, m_viewProjection(1.0f)
		, m_invViewProjection(1.0f)
	{
	}

	void Camera::setTarget(const v3& cameraPos, const v3& target)
	{
		m_view = glm::lookAt(cameraPos, target, v3(0.0f, 1.0f, 0.0f));
		updateMatrices();
	}

	void Camera::setPerspective(f32 fovDeg, f32 aspect, f32 nearPlane, f32 farPlane)
	{
		m_projection = glm::perspective(glm::radians(fovDeg), aspect, nearPlane, farPlane);
		updateMatrices();
	}

	void Camera::updateMatrices()
	{
		m_viewProjection = m_projection * m_view;
		m_invViewProjection = glm::inverse(m_viewProjection);
	}

	FirstPersonCamera::FirstPersonCamera() {}

	void FirstPersonCamera::init(const v3& defaultPos, const v3& initialTarget, f32 moveSpeed, f32 rotSpeed)
	{
		m_pos = defaultPos;
		v3 dir = glm::normalize(initialTarget - defaultPos);
		m_currPitchYawRoll.x = glm::degrees(glm::atan2<f32, glm::highp>(dir.y, -dir.z));
		m_currPitchYawRoll.y = glm::degrees(glm::atan2<f32, glm::highp>(dir.x, -dir.z));
		m_currPitchYawRoll.z = 0.0f;
		Camera::setTarget(m_pos, initialTarget);

		m_forward = dir;
		m_rotationSpeed = rotSpeed;
		m_translationSpeed = moveSpeed;
	}

	void FirstPersonCamera::update(f32 elapsedTime)
	{
		v2 mouseDelta = Window::getMouseDelta();
		f32 mouseWheel = Window::getMouseWheel();
		bool isRightMouseDown = Window::isMouseDown(MouseButton::Right);

		bool WDown = Window::isKeyDown(Keys::W);
		bool SDown = Window::isKeyDown(Keys::S);
		bool ADown = Window::isKeyDown(Keys::A);
		bool DDown = Window::isKeyDown(Keys::D);
		bool QDown = Window::isKeyDown(Keys::Q);
		bool EDown = Window::isKeyDown(Keys::E);

		bool LDown = Window::isKeyDown(Keys::Left);
		bool RDown = Window::isKeyDown(Keys::Right);
		bool UpDown = Window::isKeyDown(Keys::Up);
		bool DownDown = Window::isKeyDown(Keys::Down);

		m_translationSpeed *= (LDown ? 0.9f : RDown ? 1.1f : 1.0f);
		m_rotationSpeed *= (DownDown ? 0.9f : UpDown ? 1.1f : 1.0f);

		if (isRightMouseDown)
		{
			m_currPitchYawRoll.x -= mouseDelta.y * m_rotationSpeed * elapsedTime;
			m_currPitchYawRoll.y -= mouseDelta.x * m_rotationSpeed * elapsedTime;
		}

		v3 speed(0.0);
		if (WDown || SDown || ADown || DDown || QDown || EDown)
		{
			f32 upScale = (QDown ? 1.0f : 0.0f) + (EDown ? -1.0f : 0.0f);
			f32 rightScale = (ADown ? -1.0f : 0.0f) + (DDown ? 1.0f : 0.0f);
			f32 frontScale = (WDown ? 1.0f : 0.0f) + (SDown ? -1.0f : 0.0f);
			speed = v3(rightScale, upScale, -frontScale) * elapsedTime * m_translationSpeed;
		}

		m4 rotation = glm::yawPitchRoll(glm::radians(m_currPitchYawRoll.y), glm::radians(m_currPitchYawRoll.x), 0.0f);
		v3 right = rotation[0];
		v3 up = rotation[1];
		v3 forw = rotation[2];
		m_pos += (speed.x * right + speed.y * up + speed.z * forw);
		v4 forward = rotation * v4(0.0f, 0.0f, -1.0f, 1.0f);
		m_forward = glm::normalize(forward);
		v3 target = m_pos + v3(forward.x, forward.y, forward.z);
		Camera::setTarget(m_pos, target);
	}

}