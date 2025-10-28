#include "Application.hpp"

#include <io.h>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <iomanip>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <nlohmann/json.hpp>

#include "Util.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Cutter.hpp"
#include "Model.hpp"
#include "Tester.hpp"
#include "Renderer.hpp"
#include "Interface.hpp"
#include "StopWatch.hpp"
#include "FrameBuffer.hpp"
#include "RenderTarget.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


namespace SkinCut {
	Configuration g_Config;

	static UINT g_ResizeWidth = 0;
	static UINT g_ResizeHeight = 0;
}


Application::Application() {
	m_WindowHandle = nullptr;
	std::ignore = _setmode(_fileno(stdout), _O_U16TEXT);
}

Application::~Application() {
	// Required to have a destructor implementation for unique_ptr members
}


bool Application::Init(HWND hwnd, const std::string& resourcePath) {
	m_WindowHandle = hwnd;
	if (!m_WindowHandle) { return false; }
	g_Config.ResourcePath = resourcePath;

	StopWatch sw("init", ClockType::QPC_MS);
	
	if (!LoadConfig() || !InitRenderer() || !LoadScene() || !InitCutter() || !InitInterface()) {
		return false;
	}

	sw.Stop("init");
	std::stringstream ss;
	ss << "Initialization done (took " << sw.ElapsedTime("init") << " ms)" << std::endl;
	Util::ConsoleMessage(ss.str());

	return true;
}


bool Application::Update() {
	if (!m_Renderer) {
		throw std::exception("Renderer was not initialized properly");
	}

	// Handle window being minimized or screen locked
	if (m_Renderer->SwapChainOccluded && m_Renderer->SwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
		::Sleep(10);
		return true;
	}
	m_Renderer->SwapChainOccluded = false;

	// Handle window resize (don't resize directly in the WM_SIZE handler)
	if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
		m_Camera->Resize(g_ResizeWidth, g_ResizeHeight);
		m_Renderer->Resize(g_ResizeWidth, g_ResizeHeight);
	}

	ImGuiIO& io = ImGui::GetIO();

	// Handle keyboard input
	if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
		DestroyWindow(m_WindowHandle);
		return true;
	}

	m_Interface->Update();

	// Reload scene
	if (ImGui::IsKeyPressed(ImGuiKey_R)) {
		Reload();
		return true;
	}

	// Toggle user interface
	if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
		g_Config.HideInterface = !g_Config.HideInterface;
	}

	// Toggle wireframe
	if (ImGui::IsKeyPressed(ImGuiKey_W)) {
		g_Config.WireframeMode = !g_Config.WireframeMode;
	}

	// Run tests
	if (ImGui::IsKeyPressed(ImGuiKey_T)) {
		RECT rect; GetClientRect(m_WindowHandle, &rect);
		Math::Vector2 resolution(float(m_Renderer->RenderWidth), float(m_Renderer->RenderHeight));
		Math::Vector2 window(float(rect.right) - float(rect.left - 1), float(rect.bottom) - float(rect.top - 1));
		Tester::Test(m_Cutter, resolution, window);
	}

	// Update camera
	if (!m_Cutter->HasSelection() && !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
		m_Camera->Update();
	}

	// Pick cut point
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
		RECT rect; GetClientRect(m_WindowHandle, &rect);
		Math::Vector2 resolution((float)m_Renderer->RenderWidth, (float)m_Renderer->RenderHeight);
		Math::Vector2 window((float)rect.right - (float)rect.left - 1, (float)rect.bottom - (float)rect.top - 1);

		if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
			m_Cutter->Split(resolution, window, m_Camera->Projection, m_Camera->View);
		}
		else {
			m_Cutter->Pick(resolution, window, m_Camera->Projection, m_Camera->View);
		}
	}

	// Update lights
	for (auto& light : m_Lights) {
		light->Update();
	}

	// Update models
	for (auto& model : m_Models) {
		model->Update(m_Camera->View, m_Camera->Projection);
	}

	return true;
}


bool Application::Render() {
	if (!m_Renderer) {
		throw std::exception("Renderer was not initialized properly");
	}

	// Resize renderer if window size changed
	if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
		if (m_Renderer->BackBuffer->ColorBuffer.Get()) {
			m_Renderer->BackBuffer->ColorBuffer.Get()->Release();
		}
		m_Renderer->SwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		g_ResizeWidth = g_ResizeHeight = 0;

		ID3D11Texture2D* pBackBuffer;
		m_Renderer->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		m_Renderer->Device->CreateRenderTargetView(pBackBuffer, nullptr, m_Renderer->BackBuffer->ColorBuffer.GetAddressOf());
		pBackBuffer->Release();
	}

	// Render scene
	m_Renderer->Render(m_Models, m_Lights, m_Camera);

	// Render user interface
	m_Interface->Render(m_Lights);

	// Present rendered image on screen.
	HREXCEPT(m_Renderer->SwapChain->Present(1, 0));

	return true;
}


bool Application::Reload() {
	m_Camera->Reset();

	for (auto& light : m_Lights) {
		light->Reset();
	}

	for (auto& model : m_Models) {
		model->Reload();
	}

	return true;
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	switch (msg) {
		case WM_SIZE: {
			if (wParam == SIZE_MINIMIZED) {
				return 0;
			}
			// Queue resize
			g_ResizeWidth = (UINT)LOWORD(lParam);
			g_ResizeHeight = (UINT)HIWORD(lParam);
			return 0;
		}
		case WM_SYSCOMMAND: {
			// Disable ALT menu
			if ((wParam & 0xfff0) == SC_KEYMENU) {
				return 0;
			}
			break;
		}
		case WM_DESTROY: {
			::PostQuitMessage(0);
			return 0;
		}
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


bool Application::LoadConfig() {
	Util::ConsoleMessage("Loading settings...");

	std::string configfile = g_Config.ResourcePath + std::string("Config.json");

	std::string contents;
	std::ifstream in(configfile, std::ios::in);
	if (!in) { 
		return false;
	}

	in.seekg(0, std::ios::end);
	contents.resize(static_cast<unsigned int>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	
    auto root = nlohmann::json::parse(contents);
	if (root.is_null()) { 
		return false;
	}

	g_Config.WireframeMode = false;
	g_Config.HideInterface = false;

	g_Config.PickMode = PickType::CARVE;
	g_Config.SplitMode = SplitType::SPLIT3;
	g_Config.RenderMode = RenderType::KELEMEN;

	g_Config.EnableColor = root.at("color");
	g_Config.EnableBumps = root.at("bumps");
	g_Config.EnableShadows = root.at("shadows");
	g_Config.EnableSpeculars = root.at("speculars");
	g_Config.EnableOcclusion = root.at("occlusion");
	g_Config.EnableIrradiance = root.at("irradiance");
	g_Config.EnableScattering = root.at("scattering");

	g_Config.Ambient = (float)root.at("ambient");
	g_Config.Fresnel = (float)root.at("fresnel");
	g_Config.Roughness = (float)root.at("roughness");
	g_Config.Bumpiness = (float)root.at("bumpiness");
	g_Config.Specularity = (float)root.at("specularity");
	g_Config.Convolution = (float)root.at("convolution");
	g_Config.Translucency = (float)root.at("translucency");

	return true;
}


bool Application::LoadScene() {
	std::string sceneFile = g_Config.ResourcePath + std::string("Scene.json");

	RECT rect; GetClientRect(m_WindowHandle, &rect);
	uint32_t width = uint32_t(rect.right - rect.left);
	uint32_t height = uint32_t(rect.bottom - rect.top);

	std::string contents;
	std::ifstream in(sceneFile, std::ios::in);
	if (!in) return false;

	in.seekg(0, std::ios::end);
	contents.resize(static_cast<unsigned int>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

    nlohmann::json root = nlohmann::json::parse(contents);
    if (root.is_null()) { return false; }

    auto& camera = root.at("camera");
    auto& models = root.at("models");
	auto& lights = root.at("lights");

	// Camera
    auto& Position = camera.at("position");
    m_Camera = std::make_unique<Camera>(width, height, Position[0], Position[1], Position[2]);

	// Lights
    for (auto& light : lights) {
        std::string name = light.at("name");
		auto& Position = light.at("position");
        auto& color = light.at("color");
        m_Lights.push_back(std::make_shared<Light>(m_Device, m_Context, 
			Position[0], Position[1], Position[2], 
			Math::Color(color[0], color[1], color[2]), name));
    }

	// Models
    for (auto& model : models) {
		auto& Position = model.at("position");
		auto& rotation = model.at("rotation");
        std::string name = model.at("name");
        std::wstring resourcePath = Util::wstr(g_Config.ResourcePath);
        std::wstring meshPath = resourcePath + Util::wstr(model.at("mesh"));
        std::wstring colorPath = resourcePath + Util::wstr(model.at("color"));
        std::wstring normalPath = resourcePath + Util::wstr(model.at("normal"));
        std::wstring specularPath = resourcePath + Util::wstr(model.at("specular"));
        std::wstring discolorPath = resourcePath + Util::wstr(model.at("discolor"));
        std::wstring occlusionPath = resourcePath + Util::wstr(model.at("occlusion"));
        m_Models.push_back(std::make_shared<Model>(m_Device, 
			Math::Vector3(Position[0], Position[1], Position[2]), 
			Math::Vector2(rotation[0], rotation[1]), 
			meshPath, colorPath, normalPath, specularPath, discolorPath, occlusionPath));
    }

	return true;
}


bool Application::InitRenderer() {
	RECT rect; GetClientRect(m_WindowHandle, &rect);
	uint32_t width = uint32_t(rect.right - rect.left);
	uint32_t height = uint32_t(rect.bottom - rect.top);

	m_Renderer = std::make_unique<Renderer>(m_WindowHandle, width, height);

	m_Device = m_Renderer->Device;
	m_Context = m_Renderer->DeviceContext;
	m_SwapChain = m_Renderer->SwapChain;

	return true;
}


bool Application::InitCutter() {
	if (!m_Renderer || !m_Camera || m_Models.empty()) {
		return false;
	}

	m_Cutter = std::make_shared<Cutter>(m_Renderer, m_Camera, m_Models);
	return true;
}


bool Application::InitInterface() {
	std::wstring resourcePath = Util::wstr(g_Config.ResourcePath);
	std::wstring spriteFontFile = resourcePath + L"Fonts\\Arial12.spritefont";
	
	m_Interface = std::make_unique<Interface>(m_WindowHandle, m_Device, m_Context);

	// Sprite batch and sprite font enable simple text rendering
	m_SpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_Context.Get());
	m_SpriteFont = std::make_unique<DirectX::SpriteFont>(m_Device.Get(), spriteFontFile.c_str());

	return true;
}
