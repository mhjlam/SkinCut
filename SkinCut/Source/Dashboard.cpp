#include "Dashboard.hpp"

#include <sstream>

#include <wrl/client.h>

#include <d3d11.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Light.hpp"
#include "Utility.hpp"
#include "Structures.hpp"
#include "Mathematics.hpp"


using Microsoft::WRL::ComPtr;
using namespace SkinCut;



namespace SkinCut {
	extern Configuration gConfig;
}


Dashboard::Dashboard(HWND hwnd, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
}

Dashboard::~Dashboard()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void Dashboard::Update()
{
	// Start the frame
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}


void Dashboard::Render(std::vector<Light*>& lights)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

	ImGui::Begin("Settings");
	{
		if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
			int renderMode = ToInt(gConfig.RenderMode);
			ImGui::Combo("##", &renderMode, "Kelemen/Szirmay-Kalos\0Blinn-Phong\0Lambertian reflectance");
	           gConfig.RenderMode = (RenderType)renderMode;
		}

		if (ImGui::CollapsingHeader("Features", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Wireframe", &gConfig.EnableWireframe);
			ImGui::Checkbox("Color mapping", &gConfig.EnableColor);
			ImGui::Checkbox("Normal mapping", &gConfig.EnableBumps);
			ImGui::Checkbox("Shadow mapping", &gConfig.EnableShadows);
			ImGui::Checkbox("Specular mapping", &gConfig.EnableSpeculars);
			ImGui::Checkbox("Occlusion mapping", &gConfig.EnableOcclusion);
			ImGui::Checkbox("Irradiance mapping", &gConfig.EnableIrradiance);
			ImGui::Checkbox("Subsurface scattering", &gConfig.EnableScattering);
		}

		if (ImGui::CollapsingHeader("Shading", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Ambient", &gConfig.Ambient, 0.0f, 1.0f);
			ImGui::SliderFloat("Fresnel", &gConfig.Fresnel, 0.0f, 1.0f);
			ImGui::SliderFloat("Bumpiness", &gConfig.Bumpiness, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &gConfig.Roughness, 0.0f, 1.0f);
			ImGui::SliderFloat("Specularity", &gConfig.Specularity, 0.0f, 2.0f);
			ImGui::SliderFloat("Convolution", &gConfig.Convolution, 0.0f, 0.1f);
			ImGui::SliderFloat("Translucency", &gConfig.Translucency, 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto light : lights) {
				ImGui::SliderFloat(light->mName.c_str(), &light->mBrightness, 0.0f, 1.0f);
			}
		}

		ImGui::Separator();
		ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
