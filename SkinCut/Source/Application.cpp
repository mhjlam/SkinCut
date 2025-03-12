#include "Application.hpp"

#include <io.h>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <iomanip>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include <nlohmann/json.hpp>

#include "Util.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Cutter.hpp"
#include "Entity.hpp"
#include "Tester.hpp"
#include "Renderer.hpp"
#include "Interface.hpp"
#include "StopWatch.hpp"
#include "FrameBuffer.hpp"
#include "RenderTarget.hpp"


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")


using namespace SkinCut;
using namespace SkinCut::Math;



namespace SkinCut {
	Configuration gConfig;

	static UINT gResizeWidth = 0;
	static UINT gResizeHeight = 0;
	static bool gSwapChainOccluded = false;
}


Application::Application()
{
	mHwnd = nullptr;
	std::ignore = _setmode(_fileno(stdout), _O_U16TEXT);
}


Application::~Application() { }


bool Application::Init(HWND hwnd, const std::string& resourcePath)
{
	mHwnd = hwnd;
	if (!mHwnd) { return false; }
	gConfig.ResourcePath = resourcePath;

	StopWatch sw("init", CLOCK_QPC_MS);
	
	if (!LoadConfig() || !InitRenderer() || !LoadScene() || !InitCutter() || !InitInterface()) {
		return false;
	}

	sw.Stop("init");
	std::stringstream ss;
	ss << "Initialization done (took " << sw.ElapsedTime("init") << " ms)" << std::endl;
	Util::ConsoleMessage(ss.str());

	return true;
}


bool Application::Update()
{
	if (!mRenderer) {
		throw std::exception("Renderer was not initialized properly");
	}

	// Handle window being minimized or screen locked
	if (mRenderer->mSwapChainOccluded && mRenderer->mSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
		::Sleep(10);
		return true;
	}
	mRenderer->mSwapChainOccluded = false;

	// Handle window resize (don't resize directly in the WM_SIZE handler)
	if (gResizeWidth != 0 && gResizeHeight != 0) {
		mCamera->Resize(gResizeWidth, gResizeHeight);
		mRenderer->Resize(gResizeWidth, gResizeHeight);
	}

	ImGuiIO& io = ImGui::GetIO();

	// Handle keyboard input
	if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
		DestroyWindow(mHwnd);
		return true;
	}

	mInterface->Update();

	// Reload scene
	if (ImGui::IsKeyPressed(ImGuiKey_R)) {
		Reload();
		return true;
	}

	// Toggle user interface
	if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
		gConfig.EnableInterface = !gConfig.EnableInterface;
	}

	// Toggle wireframe
	if (ImGui::IsKeyPressed(ImGuiKey_W)) {
		gConfig.EnableWireframe = !gConfig.EnableWireframe;
	}

	// Run tests
	if (ImGui::IsKeyPressed(ImGuiKey_T)) {
		RECT rect; GetClientRect(mHwnd, &rect);
		Vector2 resolution(float(mRenderer->mWidth), float(mRenderer->mHeight));
		Vector2 window(float(rect.right) - float(rect.left - 1), float(rect.bottom) - float(rect.top - 1));
		Tester::Test(mCutter, resolution, window);
	}

	// Update camera
	if (!mCutter->HasSelection() && !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
		mCamera->Update();
	}

	// Pick cut point
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
		RECT rect; GetClientRect(mHwnd, &rect);
		Vector2 resolution((float)mRenderer->mWidth, (float)mRenderer->mHeight);
		Vector2 window((float)rect.right - (float)rect.left - 1, (float)rect.bottom - (float)rect.top - 1);

		mCutter->Pick(resolution, window, mCamera->mProjection, mCamera->mView);
	}

	// Update lights
	for (auto& light : mLights) {
		light->Update();
	}

	// Update models
	for (auto& model : mModels) {
		model->Update(mCamera->mView, mCamera->mProjection);
	}

	return true;
}


bool Application::Render()
{
	if (!mRenderer) {
		throw std::exception("Renderer was not initialized properly");
	}

	// Resize renderer if window size changed
	if (gResizeWidth != 0 && gResizeHeight != 0) {
		if (mRenderer->mBackBuffer->mColorBuffer.Get()) {
			mRenderer->mBackBuffer->mColorBuffer.Get()->Release();
		}
		mRenderer->mSwapChain->ResizeBuffers(0, gResizeWidth, gResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		gResizeWidth = gResizeHeight = 0;

		ID3D11Texture2D* pBackBuffer;
		mRenderer->mSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		mRenderer->mDevice->CreateRenderTargetView(pBackBuffer, nullptr, mRenderer->mBackBuffer->mColorBuffer.GetAddressOf());
		pBackBuffer->Release();
	}

	// Render scene
	mRenderer->Render(mModels, mLights, mCamera);

	// Render user interface
	mInterface->Render(mLights);

	// Present rendered image on screen.
	HREXCEPT(mRenderer->mSwapChain->Present(1, 0));
	//gSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

	return true;
}


bool Application::Reload()
{
	mCamera->Reset();

	for (auto& light : mLights) {
		light->Reset();
	}

	for (auto& model : mModels) {
		model->Reload();
	}

	return true;
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	switch (msg) {
		case WM_SIZE: {
			if (wParam == SIZE_MINIMIZED) {
				return 0;
			}
			// Queue resize
			gResizeWidth = (UINT)LOWORD(lParam);
			gResizeHeight = (UINT)HIWORD(lParam);
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




bool Application::LoadConfig()
{
	Util::ConsoleMessage("Loading settings...");

	std::string configfile = gConfig.ResourcePath + std::string("Config.json");

	std::string contents;
	std::ifstream in(configfile, std::ios::in);
	if (!in) { return false; }

	in.seekg(0, std::ios::end);
	contents.resize(static_cast<unsigned int>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	
    auto root = nlohmann::json::parse(contents);
	if (root.is_null()) { return false; }

	gConfig.EnableWireframe = root.at("wireframe");
	gConfig.EnableInterface = root.at("interface");

	gConfig.EnableColor = root.at("color");
	gConfig.EnableBumps = root.at("bumps");
	gConfig.EnableShadows = root.at("shadows");
	gConfig.EnableSpeculars = root.at("speculars");
	gConfig.EnableOcclusion = root.at("occlusion");
	gConfig.EnableIrradiance = root.at("irradiance");
	gConfig.EnableScattering = root.at("scattering");

	gConfig.Ambient = (float)root.at("ambient");
	gConfig.Fresnel = (float)root.at("fresnel");
	gConfig.Roughness = (float)root.at("roughness");
	gConfig.Bumpiness = (float)root.at("bumpiness");
	gConfig.Specularity = (float)root.at("specularity");
	gConfig.Convolution = (float)root.at("convolution");
	gConfig.Translucency = (float)root.at("translucency");

	std::string picker = root.at("picker");
	std::string splitter = root.at("splitter");
	std::string renderer = root.at("renderer");

	// Picker
	if (Util::CompareString(picker, "paint")) {
		gConfig.PickMode = PickType::PAINT;
	}
	else if (Util::CompareString(picker, "merge")) {
		gConfig.PickMode = PickType::MERGE;
	}
	else if (Util::CompareString(picker, "carve")) {
		gConfig.PickMode = PickType::CARVE;
	}
	else {
		return false;
	}

	// Splitter
	if (Util::CompareString(splitter, "split3")) {
		gConfig.SplitMode = SplitType::SPLIT3;
	}
	else if (Util::CompareString(splitter, "split4")) {
		gConfig.SplitMode = SplitType::SPLIT4;
	}
	else if (Util::CompareString(splitter, "split6")) {
		gConfig.SplitMode = SplitType::SPLIT6;
	}
	else {
		return false;
	}

	// Renderer
	if (Util::CompareString(renderer, "kelemen")) {
		gConfig.RenderMode = RenderType::KELEMEN;
	}
	else if (Util::CompareString(renderer, "phong")) {
		gConfig.RenderMode = RenderType::PHONG;
	}
	else if (Util::CompareString(renderer, "lambert")) {
		gConfig.RenderMode = RenderType::LAMBERT;
	}

	return true;
}


bool Application::LoadScene()
{
	std::string sceneFile = gConfig.ResourcePath + std::string("Scene.json");

	RECT rect; GetClientRect(mHwnd, &rect);
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
    mCamera = std::make_unique<Camera>(width, height, Position[0], Position[1], Position[2]);

	// Lights
    for (auto& light : lights) {
        std::string name = light.at("name");
		auto& Position = light.at("position");
        auto& color = light.at("color");
        mLights.push_back(std::make_shared<Light>(mDevice, mContext, Position[0], Position[1], Position[2], Color(color[0], color[1], color[2]), name));
    }

	// Models
    for (auto& model : models) {
		auto& Position = model.at("position");
		auto& rotation = model.at("rotation");
        std::string name = model.at("name");
        std::wstring resourcePath = Util::wstr(gConfig.ResourcePath);
        std::wstring meshPath = resourcePath + Util::wstr(model.at("mesh"));
        std::wstring colorPath = resourcePath + Util::wstr(model.at("color"));
        std::wstring normalPath = resourcePath + Util::wstr(model.at("normal"));
        std::wstring specularPath = resourcePath + Util::wstr(model.at("specular"));
        std::wstring discolorPath = resourcePath + Util::wstr(model.at("discolor"));
        std::wstring occlusionPath = resourcePath + Util::wstr(model.at("occlusion"));
        mModels.push_back(std::make_shared<Entity>(mDevice, Vector3(Position[0], Position[1], Position[2]), Vector2(rotation[0], rotation[1]), meshPath, colorPath, normalPath, specularPath, discolorPath, occlusionPath));
    }

	return true;
}


bool Application::InitRenderer()
{
	RECT rect; GetClientRect(mHwnd, &rect);
	uint32_t width = uint32_t(rect.right - rect.left);
	uint32_t height = uint32_t(rect.bottom - rect.top);

	mRenderer = std::make_unique<Renderer>(mHwnd, width, height);

	mDevice = mRenderer->mDevice;
	mContext = mRenderer->mContext;
	mSwapChain = mRenderer->mSwapChain;

	return true;
}


bool Application::InitCutter()
{
	if (!mRenderer || !mCamera || mModels.empty()) {
		return false;
	}

	mCutter = std::make_shared<Cutter>(mRenderer, mCamera, mModels);
	return true;
}


bool Application::InitInterface()
{
	std::wstring resourcePath = Util::wstr(gConfig.ResourcePath);
	std::wstring spriteFontFile = resourcePath + L"Fonts\\Arial12.spritefont";
	
	mInterface = std::make_unique<Interface>(mHwnd, mDevice, mContext);

	// Sprite batch and sprite font enable simple text rendering
	mSpriteBatch = std::make_unique<SpriteBatch>(mContext.Get());
	mSpriteFont = std::make_unique<SpriteFont>(mDevice.Get(), spriteFontFile.c_str());

	return true;
}

