#include "Light.hpp"

#include "Util.hpp"
#include "FrameBuffer.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;

using DirectX::XMConvertToRadians;

Light::Light(ComPtr<ID3D11Device>& device, 
			 ComPtr<ID3D11DeviceContext>& context, 
			 float yaw, float pitch, float distance, 
			 Math::Color color, 
			 std::string name, 
			 float fovY, uint32_t shadowSize) : m_Device(device), m_Context(context), Yaw(yaw), Pitch(pitch), 
			                                    Distance(distance), Color(color), FieldOfViewDegrees(fovY) {
	Name = name;
	NearPlane = 0.1f;									// z-value of near plane of the projection matrix
	FarPlane = 10.0f;									// z-value of far plane of the projection matrix
	Attenuation = 1.f / 128.f;							// quadratic attenuation factor
	FieldOfViewRadians = XMConvertToRadians(FieldOfViewDegrees);			// field of view of 45 degrees (in radians)
	FalloffStart = float(cos(FieldOfViewRadians / 2.0f));	// cosine of the umbra angle
	FalloffWidth = 0.05f;								// cosine of angle between umbra and penumbra

	Brightness = BrightnessPrev = Math::Color::RGBtoHSV(Color).z;

	ShadowMap = std::make_shared<FrameBuffer>(m_Device, m_Context, shadowSize, shadowSize);
	SetViewProjection();

	m_LoadInfo.Yaw = Yaw;
	m_LoadInfo.FovY = FieldOfViewDegrees;
	m_LoadInfo.Pitch = Pitch;
	m_LoadInfo.Color = Color;
	m_LoadInfo.Distance = Distance;
}

void Light::Update() {
	if (Brightness != BrightnessPrev) {
		Math::Color hsv = Math::Color::RGBtoHSV(Color);
		hsv.z = Brightness; // adjust brightness
		Color = Math::Color::HSVtoRGB(hsv);

		BrightnessPrev = Brightness;
	}
}

void Light::Reset() {
	Yaw = m_LoadInfo.Yaw;
	FieldOfViewDegrees = m_LoadInfo.FovY;
	Pitch = m_LoadInfo.Pitch;
	Color = m_LoadInfo.Color;
	Distance = m_LoadInfo.Distance;

	NearPlane = 0.1f;									// z-value of near plane of the projection matrix
	FarPlane = 10.0f;									// z-value of far plane of the projection matrix
	Attenuation = 1.f / 128.f;							// quadratic attenuation factor
	FieldOfViewRadians = XMConvertToRadians(FieldOfViewDegrees);			// field of view of 45 degrees (in radians)
	FalloffStart = float(cos(FieldOfViewRadians / 2.0f));	// cosine of the umbra angle
	FalloffWidth = 0.05f;								// cosine of angle between umbra and penumbra

	Brightness = BrightnessPrev = Math::Color::RGBtoHSV(Color).z;
	ShadowMap = std::make_shared<FrameBuffer>(m_Device, m_Context, 
											  (uint32_t)ShadowMap->Viewport.Width, 
											  (uint32_t)ShadowMap->Viewport.Height);

	SetViewProjection();
}

void Light::SetViewProjection() {
	View = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Yaw)) * 
			DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Pitch)) *
			DirectX::XMMatrixTranslation(0, 0, Distance);

	Projection = DirectX::XMMatrixPerspectiveFovLH(FieldOfViewRadians, 1.0f, NearPlane, FarPlane);
	
	// remaps normalized device coordinates [-1,1] to [0,1]
	Math::Matrix scaleBias = Math::Matrix(0.5, 0.0, 0.0, 0.0,
							  			  0.0,-0.5, 0.0, 0.0, 
							  			  0.0, 0.0, 1.0, 0.0, 
							  			  0.5, 0.5, 0.0, 1.0);

	ViewProjection = View * Projection * scaleBias;

	// Linearize depth buffer (See: http://www.mvps.org/directx/articles/linear_z/linearz.htm)
	Math::Matrix linearProjection = Projection;
	linearProjection._33 /= FarPlane;
	linearProjection._43 /= FarPlane;

	ViewProjectionLinear = View * linearProjection;

	Math::Matrix viewInv = View.Invert();
	Math::Vector4 vsTarget = Math::Vector4::Transform(Math::Vector4(0.0f, 0.0f, Distance, 1.0f), viewInv);
	Math::Vector4 vsPosition = Math::Vector4::Transform(Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f), viewInv);

	Math::Vector3 target = Math::Vector3(vsTarget.x, vsTarget.y, vsTarget.z);
	Position = Math::Vector3(vsPosition.x, vsPosition.y, vsPosition.z);
	Direction = Math::Vector3::Normalize(target - Position);
}
