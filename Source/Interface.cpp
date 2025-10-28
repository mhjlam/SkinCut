#include "Interface.hpp"

#include <sstream>

#include <d3d11.h>
#include <wrl/client.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "Math.hpp"
#include "Util.hpp"
#include "Light.hpp"
#include "Types.hpp"


using Microsoft::WRL::ComPtr;
using namespace SkinCut;


namespace SkinCut {
	extern Configuration g_Config;
}


Interface::Interface(HWND hwnd, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
}

Interface::~Interface() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Interface::Update() {
	// Start the frame
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}

void Interface::Render(std::vector<std::shared_ptr<Light>>& lights) {
	ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200.f, 0.f));

	if (!g_Config.HideInterface) {
		ImGui::Begin("SkinCut", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
		
		ImGui::SetNextItemWidth(-100.f);
		if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
			//int renderMode = ToInt(g_Config.RenderMode);
			//ImGui::PushItemWidth(185.f);
			//ImGui::Combo("##RenderMode", &renderMode, "Kelemen/Szirmay-Kalos\0Blinn-Phong\0Lambertian");
			//ImGui::PopItemWidth();
			//g_Config.RenderMode = (RenderType)renderMode;

			ImGui::Checkbox("Wireframe", &g_Config.WireframeMode);

			ImGui::Separator();

			ImGui::Checkbox("Color", &g_Config.EnableColor);
			ImGui::Checkbox("Normals", &g_Config.EnableBumps);
			ImGui::Checkbox("Shadows", &g_Config.EnableShadows);
			ImGui::Checkbox("Speculars", &g_Config.EnableSpeculars);
			ImGui::Checkbox("Occlusion", &g_Config.EnableOcclusion);
			ImGui::Checkbox("Irradiance", &g_Config.EnableIrradiance);
			ImGui::Checkbox("Subsurface", &g_Config.EnableScattering);
		}

		if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto& light : lights) {
				ImGui::SliderFloat(light->Name.c_str(), &light->Brightness, 0.0f, 1.0f);
			}
		}

		if (ImGui::CollapsingHeader("Shading", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushItemWidth(100.f);
			ImGui::SliderFloat("Ambient", &g_Config.Ambient, 0.0f, 1.0f);
			ImGui::SliderFloat("Fresnel", &g_Config.Fresnel, 0.0f, 1.0f);
			ImGui::SliderFloat("Bumpiness", &g_Config.Bumpiness, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &g_Config.Roughness, 0.0f, 1.0f);
			ImGui::SliderFloat("Specularity", &g_Config.Specularity, 0.0f, 2.0f);

			ImGui::Separator();

			ImGui::SliderFloat("Convolution", &g_Config.Convolution, 0.0f, 0.1f);
			ImGui::SliderFloat("Translucency", &g_Config.Translucency, 0.0f, 0.999f);
			ImGui::PopItemWidth();
		}

		if (ImGui::CollapsingHeader("Cutter", ImGuiTreeNodeFlags_DefaultOpen)) {
				int pickMode = ToInt(g_Config.PickMode);
				ImGui::PushItemWidth(185.f);
				ImGui::Combo("##PickMode", &pickMode, "Paint\0Merge\0Carve");
				ImGui::PopItemWidth();
				g_Config.PickMode = (PickType)pickMode;

				// TODO: reset the earlier chosen intersections
				//m_Cutter->Reset();

				int splitMode = ToInt(g_Config.SplitMode);
				ImGui::PushItemWidth(185.f);
				ImGui::Combo("##SplitMode", &splitMode, "Split3\0Split4\0Split6");
				ImGui::PopItemWidth();
				g_Config.SplitMode = (SplitType)splitMode;
		}

		ImGui::Separator();
		ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
