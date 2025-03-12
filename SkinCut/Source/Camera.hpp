#pragma once

#include "Math.hpp"


namespace SkinCut
{
	class Camera
	{
	public: // constants
		static const float cFieldOfView;
		static const float cNearPlane;
		static const float cFarPlane;
		static const float cDistanceMin;
		static const float cDistanceMax;
		static const float cPitchMin;
		static const float cPitchMax;


	public:
		Math::Vector3 mEye;
		Math::Vector3 mTarget;

		Math::Matrix mView;
		Math::Matrix mProjection;


	private:
		float mYaw;
		float mPitch;
		float mDistance;
		float mPanX, mPanY;
		float mWidth, mHeight;

		// backup parameters for resetting
		float mOrigYaw;
		float mOrigPitch;
		float mOrigDistance;


	public:
		Camera(uint32_t width, uint32_t height, float yaw, float pitch, float distance);

		void Update();
		void Resize(uint32_t width, uint32_t height);
		void Reset();
	};

}

