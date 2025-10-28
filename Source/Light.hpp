#pragma once

#include <map>
#include <array>
#include <memory>
#include <string>
#include <cstdint>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


namespace SkinCut {
	class FrameBuffer;

	struct LightLoadInfo {
		float Yaw = 0;
		float FovY = 0;
		float Pitch = 0;
		float Distance = 0;
		Math::Color Color = Math::Color();
	};

	class Light {
	public:
		Light(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			  Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context, 
			  float yaw, float pitch, float distance, 
			  Math::Color color = Math::Color(0.5f, 0.5f, 0.5f), 
			  std::string name = "Light", float fovY = 45.0f, uint32_t shadowSize = 2048);

		void Update();
		void Reset();

	private:
		void SetViewProjection();
		
	public:
		float Yaw;
		float Pitch;
		float Distance;
		float FarPlane;
		float NearPlane;
		float Attenuation;
		float FalloffStart;
		float FalloffWidth;
		float FieldOfViewDegrees;
		float FieldOfViewRadians;

		float Brightness;
		float BrightnessPrev;

		std::string Name;
		Math::Color Color;
		Math::Vector3 Position;
		Math::Vector3 Direction;

		Math::Matrix View;
		Math::Matrix Projection;
		Math::Matrix ViewProjection;
		Math::Matrix ViewProjectionLinear;

		std::shared_ptr<FrameBuffer> ShadowMap;
	
	private:
		LightLoadInfo m_LoadInfo;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	};
}
