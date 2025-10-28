#include "Camera.hpp"

#include <imgui.h>

#include "Constants.hpp"


using namespace DirectX;
using namespace SkinCut;


Camera::Camera(uint32_t width, uint32_t height, float yaw, float pitch, float distance)
: m_Yaw(yaw), m_Pitch(pitch), m_Distance(distance) {
	m_YawBackup = m_Yaw;
	m_PitchBackup = m_Pitch;
	m_DistanceBackup = m_Distance;

	m_Pan = Math::Vector2(0.0f, 0.0f);
	m_Dim = Math::Vector2((float)width, (float)height);

	View = Math::Matrix::CreateRotationY(m_Yaw) * 
			Math::Matrix::CreateRotationX(m_Pitch) * 
			Math::Matrix::CreateTranslation(-m_Pan.x, -m_Pan.y, m_Distance);

	Projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(Constants::FOV_Y), m_Dim.x / m_Dim.y, Constants::NEAR_PLANE, Constants::FAR_PLANE);
	Eye = View.Invert().Translation();
	Target = Math::Vector3(0,0,0);
}

void Camera::Update() {
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) { // left: rotate
		m_Yaw -= io.MouseDelta.x * 0.004f;
		m_Pitch -= io.MouseDelta.y * 0.004f;
		m_Pitch = std::max(Constants::MIN_PITCH, std::min(m_Pitch, Constants::MAX_PITCH)); // clamp pitch
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) { // right: pan
		// Update position
		float dX = io.MouseDelta.x / m_Dim.x;
		float dY = io.MouseDelta.y / m_Dim.y;

		Math::Matrix transform = Math::Matrix::CreateTranslation(0, 0, m_Distance) * Projection;

		Math::Vector4 t = Math::Vector4::Transform(
			Math::Vector4(m_Pan.x, m_Pan.y, 0.0f, 1.0f), transform);
		Math::Vector4 s = Math::Vector4::Transform(
			Math::Vector4(t.x - dX*t.w, t.y + dY*t.w, t.z, t.w), transform.Invert());

		m_Pan.x = s.x;
		m_Pan.y = s.y;
	}

	if (io.MouseWheel > 0) {
		m_Distance = std::max(Constants::MIN_DISTANCE, std::min(m_Distance - 0.5f, Constants::MAX_DISTANCE));
	}
	else if (io.MouseWheel < 0) {
		m_Distance = std::max(Constants::MIN_DISTANCE, std::min(m_Distance + 0.5f, Constants::MAX_DISTANCE));
	}

	// this assumes target is at (0,0,0):
	View = Math::Matrix::CreateRotationY(m_Yaw) * 
			Math::Matrix::CreateRotationX(m_Pitch) * 
			Math::Matrix::CreateTranslation(-m_Pan.x, -m_Pan.y, m_Distance);
	Eye = View.Invert().Translation();
}

void Camera::Resize(uint32_t width, uint32_t height)  {
	m_Dim.x = (float)width;
	m_Dim.y = (float)height;
	Projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(Constants::FOV_Y), m_Dim.x / m_Dim.y, Constants::NEAR_PLANE, Constants::FAR_PLANE);
}

void Camera::Reset() {
	// restore stored values for yaw, pitch, distance
	m_Yaw = m_YawBackup;
	m_Pitch = m_PitchBackup;
	m_Distance = m_DistanceBackup;

	m_Pan.x = 0.0f;
	m_Pan.y = 0.0f;

	View = Math::Matrix::CreateRotationY(m_Yaw) * 
			Math::Matrix::CreateRotationX(m_Pitch) * 
			Math::Matrix::CreateTranslation(-m_Pan.x, -m_Pan.y, m_Distance);

	Projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(Constants::FOV_Y), m_Dim.x / m_Dim.y, Constants::NEAR_PLANE, Constants::FAR_PLANE);

	Eye = View.Invert().Translation();
	Target = Math::Vector3(0,0,0);
}
