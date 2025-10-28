#pragma once

#include "Math.hpp"


namespace SkinCut {
	class Camera {
	public:
		Camera(uint32_t width, uint32_t height, float yaw, float pitch, float distance);

		void Update();
		void Resize(uint32_t width, uint32_t height);
		void Reset();
		
	public:
		Math::Vector3 Eye;
		Math::Vector3 Target;

		Math::Matrix View;
		Math::Matrix Projection;

	private:
		float m_Yaw;
		float m_Pitch;
		float m_Distance;
		Math::Vector2 m_Pan;
		Math::Vector2 m_Dim;

		// backup parameters for resetting
		float m_YawBackup;
		float m_PitchBackup;
		float m_DistanceBackup;
	};
}
