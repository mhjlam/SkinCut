#include "Camera.hpp"

#include "ImGui/imgui.h"
#include "SimpleJSON/JSON.h"


using namespace DirectX;
using namespace SkinCut;
using namespace SkinCut::Math;


const float Camera::cFieldOfView = 20.f;
const float Camera::cNearPlane = 0.1f;
const float Camera::cFarPlane = 20.f;

const float Camera::cDistanceMin = Camera::cNearPlane + 1.0f;
const float Camera::cDistanceMax = Camera::cFarPlane - 1.0f;

const float Camera::cPitchMin = -DirectX::XM_PIDIV2 + 0.2f;
const float Camera::cPitchMax = DirectX::XM_PIDIV2 - 0.2f;



Camera::Camera(uint32_t width, uint32_t height, float yaw, float pitch, float distance)
	: mYaw(yaw), mPitch(pitch), mDistance(distance)
{
	mOrigYaw = mYaw;
	mOrigPitch = mPitch;
	mOrigDistance = mDistance;

	mPanX = 0.0f;
	mPanY = 0.0f;

	mWidth = (float)width;
	mHeight = (float)height;

	mView = Matrix::CreateRotationY(mYaw) * 
			Matrix::CreateRotationX(mPitch) * 
			Matrix::CreateTranslation(-mPanX, -mPanY, mDistance);

	mProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(cFieldOfView), 
		mWidth / mHeight, cNearPlane, cFarPlane);

	mEye = mView.Invert().Translation();
	mTarget = Vector3(0,0,0);
}


void Camera::Update()
{
	ImGuiIO& io = ImGui::GetIO();

	if (io.MouseDown[0]) { // left: rotate
		mYaw -= io.MouseDelta.x * 0.004f;
		mPitch -= io.MouseDelta.y * 0.004f;
		mPitch = std::max(cPitchMin, std::min(mPitch, cPitchMax)); // clamp pitch
	}

	if (io.MouseDown[1]) { // right: zoom
		mDistance += io.MouseDelta.y / 75.0f;
		mDistance = std::max(cDistanceMin, std::min(mDistance, cDistanceMax)); // clamp
	}

	if (io.MouseDown[2]) { // middle: pan
		// Update position
		float dX = io.MouseDelta.x / mWidth;
		float dY = io.MouseDelta.y / mHeight;

		Matrix transform = Matrix::CreateTranslation(0, 0, mDistance) * mProjection;

		Vector4 t = Vector4::Transform(Vector4(mPanX, mPanY, 0.0f, 1.0f), transform);
		Vector4 s = Vector4::Transform(Vector4(t.x - dX*t.w, t.y + dY*t.w, t.z, t.w), transform.Invert());

		mPanX = s.x;
		mPanY = s.y;
	}

	if (io.MouseWheel > 0) {
		mDistance -= 0.5f;
		mDistance = std::max(cDistanceMin, std::min(mDistance, cDistanceMax));
	}
	else if (io.MouseWheel < 0) {
		mDistance += 0.5f;
		mDistance = std::max(cDistanceMin, std::min(mDistance, cDistanceMax));
	}

// 	Matrix rotation = Matrix::CreateFromYawPitchRoll(mYaw, -mPitch, 0);
// 
// 	Vector3 translation(0, 0, mDistance);
// 	translation = Vector3::Transform(translation, rotation);
// 	mEye = mTarget + translation;
// 
// 	Vector3 up = Vector3::Transform(Vector3(0,1,0), rotation);
// 	mView = Matrix::CreateLookAt(mEye, mTarget, up);

	// this assumes target is at (0,0,0):
	mView = Matrix::CreateRotationY(mYaw) * 
			Matrix::CreateRotationX(mPitch) * 
			Matrix::CreateTranslation(-mPanX, -mPanY, mDistance);
	mEye = mView.Invert().Translation();
}


void Camera::Resize(uint32_t width, uint32_t height) 
{
	mWidth = (float)width;
	mHeight = (float)height;
	mProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(cFieldOfView), mWidth / mHeight, cNearPlane, cFarPlane);
}


void Camera::Reset()
{
	// restore stored values for yaw, pitch, distance
	mYaw = mOrigYaw;
	mPitch = mOrigPitch;
	mDistance = mOrigDistance;


	mPanX = 0.0f;
	mPanY = 0.0f;

	mView = Matrix::CreateRotationY(mYaw) * 
			Matrix::CreateRotationX(mPitch) * 
			Matrix::CreateTranslation(-mPanX, -mPanY, mDistance);

	mProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(cFieldOfView), mWidth / mHeight, cNearPlane, cFarPlane);

	mEye = mView.Invert().Translation();
	mTarget = Vector3(0,0,0);
}

