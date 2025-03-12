#include "Light.hpp"

#include "Util.hpp"
#include "FrameBuffer.hpp"


using namespace SkinCut;
using namespace SkinCut::Math;
using DirectX::XMConvertToRadians;

Light::Light(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float yaw, float pitch, float distance, Color color, std::string name, float fovY, uint32_t shadowSize)
: mDevice(device), mContext(context), mYaw(yaw), mPitch(pitch), mDistance(distance), mColor(color), mFovy(fovY)
{
	mName = name;
	mNearPlane = 0.1f;									// z-value of near plane of the projection matrix
	mFarPlane = 10.0f;									// z-value of far plane of the projection matrix
	mAttenuation = 1.f / 128.f;							// quadratic attenuation factor
	mFieldOfView = XMConvertToRadians(mFovy);			// field of view of 45 degrees (in radians)
	mFalloffStart = float(cos(mFieldOfView / 2.0f));	// cosine of the umbra angle
	mFalloffWidth = 0.05f;								// cosine of angle between umbra and penumbra

	mBrightness = mBrightnessPrev = Color::RGBtoHSV(mColor).z;

	mShadowMap = std::make_shared<FrameBuffer>(mDevice, mContext, shadowSize, shadowSize);
	SetViewProjection();

	mLoadInfo.yaw = mYaw;
	mLoadInfo.fovy = mFovy;
	mLoadInfo.pitch = mPitch;
	mLoadInfo.color = mColor;
	mLoadInfo.distance = mDistance;
}


void Light::Update()
{
	if (mBrightness != mBrightnessPrev) {
		Color hsv = Color::RGBtoHSV(mColor);
		hsv.z = mBrightness; // adjust brightness
		mColor = Color::HSVtoRGB(hsv);

		mBrightnessPrev = mBrightness;
	}
}


void Light::Reset()
{
	mYaw = mLoadInfo.yaw;
	mFovy = mLoadInfo.fovy;
	mPitch = mLoadInfo.pitch;
	mColor = mLoadInfo.color;
	mDistance = mLoadInfo.distance;

	mNearPlane = 0.1f;									// z-value of near plane of the projection matrix
	mFarPlane = 10.0f;									// z-value of far plane of the projection matrix
	mAttenuation = 1.f / 128.f;							// quadratic attenuation factor
	mFieldOfView = XMConvertToRadians(mFovy);			// field of view of 45 degrees (in radians)
	mFalloffStart = float(cos(mFieldOfView / 2.0f));	// cosine of the umbra angle
	mFalloffWidth = 0.05f;								// cosine of angle between umbra and penumbra

	mBrightness = mBrightnessPrev = Color::RGBtoHSV(mColor).z;
	mShadowMap = std::make_shared<FrameBuffer>(mDevice, mContext, (uint32_t)mShadowMap->mViewport.Width, (uint32_t)mShadowMap->mViewport.Height);

	SetViewProjection();
}


void Light::SetViewProjection()
{
	mView = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(mYaw)) * 
			DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(mPitch)) *
			DirectX::XMMatrixTranslation(0, 0, mDistance);

	mProjection = DirectX::XMMatrixPerspectiveFovLH(mFieldOfView, 1.0f, mNearPlane, mFarPlane);

	Matrix scalebias = Matrix(0.5, 0.0, 0.0, 0.0, // remaps normalized device coordinates [-1,1] to [0,1]
							  0.0,-0.5, 0.0, 0.0, 
							  0.0, 0.0, 1.0, 0.0, 
							  0.5, 0.5, 0.0, 1.0);

	mViewProjection = mView * mProjection * scalebias;

	// Linearize depth buffer (See: http://www.mvps.org/directx/articles/linear_z/linearz.htm)
	Matrix linearProjection = mProjection;
	linearProjection._33 /= mFarPlane;
	linearProjection._43 /= mFarPlane;

	mViewProjectionLinear = mView * linearProjection;

	Matrix viewinv = mView.Invert();
	Vector4 vs_target = Vector4::Transform(Vector4(0.0f, 0.0f, mDistance, 1.0f), viewinv);
	Vector4 vs_position = Vector4::Transform(Vector4(0.0f, 0.0f, 0.0f, 1.0f), viewinv);

	Vector3 target = Vector3(vs_target.x, vs_target.y, vs_target.z);
	mPosition = Vector3(vs_position.x, vs_position.y, vs_position.z);
	mDirection = Vector3::Normalize(target - mPosition);
}

