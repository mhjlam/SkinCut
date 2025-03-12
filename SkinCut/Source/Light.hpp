#pragma once

#include <map>
#include <array>
#include <memory>
#include <string>
#include <cstdint>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class FrameBuffer;

	struct LightLoadInfo
	{
		float yaw = 0;
		float fovy = 0;
		float pitch = 0;
		float distance = 0;
		Math::Color color = Math::Color();
	};


	class Light
	{
	private:
		LightLoadInfo mLoadInfo;
		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mContext;
		
	public:
		float mYaw;
		float mFovy;
		float mPitch;
		float mDistance;
		float mFarPlane;
		float mNearPlane;
		float mAttenuation;
		float mFieldOfView;
		float mFalloffStart;
		float mFalloffWidth;

		float mBrightness;
		float mBrightnessPrev;

		std::string mName;
		Math::Color mColor;
		Math::Vector3 mPosition;
		Math::Vector3 mDirection;

		Math::Matrix mView;
		Math::Matrix mProjection;
		Math::Matrix mViewProjection;
		Math::Matrix mViewProjectionLinear;

		std::shared_ptr<FrameBuffer> mShadowMap;


	public:
		Light(ComPtr<ID3D11Device>& device, 
			  ComPtr<ID3D11DeviceContext>& context, 
			  float yaw, float pitch, float distance, 
			  Math::Color color = Math::Color(0.5f, 0.5f, 0.5f), 
			  std::string name = "Light", float fovY = 45.0f, uint32_t shadowSize = 2048);

		void Update();
		void Reset();


	private:
		void SetViewProjection();
	};
}

