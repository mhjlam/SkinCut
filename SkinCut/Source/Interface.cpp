#include "Interface.hpp"

#include <sstream>

#include <wrl/client.h>

#include <d3d11.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>

#include "Math.hpp"
#include "Util.hpp"
#include "Light.hpp"
#include "Structs.hpp"


using Microsoft::WRL::ComPtr;
using namespace SkinCut;


namespace SkinCut {
	extern Configuration gConfig;
}



Interface::Interface(HWND hwnd, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
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

Interface::~Interface()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void Interface::Update()
{
	// Start the frame
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}


void Interface::Render(std::vector<std::shared_ptr<Light>>& lights)
{
	ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200.f, 0.f));

	if (gConfig.EnableInterface) {
		ImGui::Begin("Settings");
		{
			ImGui::SetNextItemWidth(-100.f);

			if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
				int renderMode = ToInt(gConfig.RenderMode);

				ImGui::PushItemWidth(185.f);
				ImGui::Combo("##RenderMode", &renderMode, "Kelemen/Szirmay-Kalos\0Blinn-Phong\0Lambertian");
				ImGui::PopItemWidth();

				gConfig.RenderMode = (RenderType)renderMode;

				ImGui::Checkbox("Wireframe", &gConfig.EnableWireframe);
			}

			if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Color", &gConfig.EnableColor);
				ImGui::Checkbox("Normals", &gConfig.EnableBumps);
				ImGui::Checkbox("Shadows", &gConfig.EnableShadows);
				ImGui::Checkbox("Speculars", &gConfig.EnableSpeculars);
				ImGui::Checkbox("Occlusion", &gConfig.EnableOcclusion);
				ImGui::Checkbox("Irradiance", &gConfig.EnableIrradiance);
				ImGui::Checkbox("Subsurface", &gConfig.EnableScattering);
			}

			if (ImGui::CollapsingHeader("Shading")) {
				ImGui::PushItemWidth(100.f);
				ImGui::SliderFloat("Ambient", &gConfig.Ambient, 0.0f, 1.0f);
				ImGui::SliderFloat("Fresnel", &gConfig.Fresnel, 0.0f, 1.0f);
				ImGui::SliderFloat("Bumpiness", &gConfig.Bumpiness, 0.0f, 1.0f);
				ImGui::SliderFloat("Roughness", &gConfig.Roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Specularity", &gConfig.Specularity, 0.0f, 2.0f);
				ImGui::SliderFloat("Convolution", &gConfig.Convolution, 0.0f, 0.1f);
				ImGui::SliderFloat("Translucency", &gConfig.Translucency, 0.0f, 1.0f);
				ImGui::PopItemWidth();
			}

			if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (auto& light : lights) {
					ImGui::SliderFloat(light->mName.c_str(), &light->mBrightness, 0.0f, 1.0f);
				}
			}

			if (ImGui::CollapsingHeader("Cutter"), ImGuiTreeNodeFlags_DefaultOpen) {
				{
					int pickMode = ToInt(gConfig.PickMode);
					ImGui::PushItemWidth(185.f);
					ImGui::Combo("##PickMode", &pickMode, "Paint\0Merge\0Carve");
					ImGui::PopItemWidth();
					gConfig.PickMode = (PickType)pickMode;

					// TODO: reset the earlier chosen intersections
					//mCutter->Reset();
				}

				{
					int splitMode = ToInt(gConfig.SplitMode);
					ImGui::PushItemWidth(185.f);
					ImGui::Combo("##SplitMode", &splitMode, "Split3\0Split4\0Split6");
					ImGui::PopItemWidth();
					gConfig.SplitMode = (SplitType)splitMode;
				}
			}

			ImGui::Separator();
			ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

