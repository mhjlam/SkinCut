#include "Application.hpp"

#include <io.h>
#include <math.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "SimpleJSON/JSON.h"

#include "Light.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Shader.hpp"
#include "Target.hpp"
#include "Utility.hpp"
#include "Renderer.hpp"
#include "Dashboard.hpp"
#include "Generator.hpp"
#include "Stopwatch.hpp"
#include "FrameBuffer.hpp"


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
	static ID3D11RenderTargetView* gMainRenderTargetView = nullptr;
}


// number of runs for performance test
constexpr auto cNumTestRuns = 100;


Application::Application()
{
	mHwnd = nullptr;
	std::ignore = _setmode(_fileno(stdout), _O_U16TEXT);
}

Application::~Application() { }

bool Application::Initialize(HWND hwnd, const std::string& resourcePath)
{
	mHwnd = hwnd;
	if (!mHwnd) { return false; }
	gConfig.ResourcePath = resourcePath;

	Stopwatch sw("init", CLOCK_QPC_MS);
	
	if (!LoadConfig() || !SetupRenderer() || !LoadScene() || !SetupDashboard()) {
		return false;
	}

	sw.Stop("init");
	std::stringstream ss;
	ss << "Initialization done (took " << sw.ElapsedTime("init") << " ms)" << std::endl;
	Utility::ConsoleMessage(ss.str());

	return true;
}


bool Application::LoadConfig()
{
// 	Utility::ConsoleMessage("Loading settings...");

	std::string configfile = gConfig.ResourcePath + std::string("Config.json");

	std::string contents;
	std::ifstream in(configfile, std::ios::in);
	if (!in) return false;

	in.seekg(0, std::ios::end);
	contents.resize(static_cast<unsigned int>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	auto data = std::unique_ptr<JSONValue>(JSON::Parse(contents.c_str()));
	if (!data || !data->IsObject()) return false;
	JSONObject root = data->AsObject();

	gConfig.EnableWireframe = root.at(L"bWireframe")->AsBool();
	gConfig.EnableDashboard = root.at(L"bDashboard")->AsBool();

	gConfig.EnableColor = root.at(L"bColor")->AsBool();
	gConfig.EnableBumps = root.at(L"bBumps")->AsBool();
	gConfig.EnableShadows = root.at(L"bShadows")->AsBool();
	gConfig.EnableSpeculars = root.at(L"bSpeculars")->AsBool();
	gConfig.EnableOcclusion = root.at(L"bOcclusion")->AsBool();
	gConfig.EnableIrradiance = root.at(L"bIrradiance")->AsBool();
	gConfig.EnableScattering = root.at(L"bScattering")->AsBool();

	gConfig.Ambient = (float)root.at(L"fAmbient")->AsNumber();
	gConfig.Fresnel = (float)root.at(L"fFresnel")->AsNumber();
	gConfig.Roughness = (float)root.at(L"fRoughness")->AsNumber();
	gConfig.Bumpiness = (float)root.at(L"fBumpiness")->AsNumber();
	gConfig.Specularity = (float)root.at(L"fSpecularity")->AsNumber();
	gConfig.Scattering = (float)root.at(L"fScattering")->AsNumber();
	gConfig.Translucency = (float)root.at(L"fTranslucency")->AsNumber();

	std::wstring pickMode = root.at(L"sPick")->AsString();
	if (Utility::CompareString(pickMode, L"draw")) {
		gConfig.PickMode = PickType::PAINT;
	}
	else if (Utility::CompareString(pickMode, L"fuse")) {
		gConfig.PickMode = PickType::MERGE;
	}
	else {
		gConfig.PickMode = PickType::CARVE;
	}

	std::wstring splitMode = root.at(L"sSplit")->AsString();
	if (Utility::CompareString(splitMode, L"4split")) {
		gConfig.SplitMode = SplitType::SPLIT4;
	}
	else if (Utility::CompareString(splitMode, L"6split")) {
		gConfig.SplitMode = SplitType::SPLIT6;
	}
	else {
		gConfig.SplitMode = SplitType::SPLIT3;
	}
	
	std::wstring renderMode = root.at(L"sRenderer")->AsString();
	if (Utility::CompareString(renderMode, L"phong")) {
		gConfig.RenderMode = RenderType::PHONG;
	}
	else if (Utility::CompareString(renderMode, L"lambert")) {
		gConfig.RenderMode = RenderType::LAMBERT;
	}
	else {
		gConfig.RenderMode = RenderType::KELEMEN;
	}

	return true;
}


bool Application::LoadScene()
{
	std::string scenefile = gConfig.ResourcePath + std::string("Scene.json");

	RECT rect; GetClientRect(mHwnd, &rect);
	uint32_t width = uint32_t(rect.right - rect.left);
	uint32_t height = uint32_t(rect.bottom - rect.top);

	std::string contents;
	std::ifstream in(scenefile, std::ios::in);
	if (!in) return false;

	in.seekg(0, std::ios::end);
	contents.resize(static_cast<unsigned int>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	auto jdata = std::unique_ptr<JSONValue>(JSON::Parse(contents.c_str()));
	if (!jdata || !jdata->IsObject()) return false;
	auto& jcamera = jdata->AsObject().at(L"camera")->AsObject();
	auto& jlights = jdata->AsObject().at(L"lights")->AsArray();
	auto& jmodels = jdata->AsObject().at(L"models")->AsArray();


	// camera
	float cy = (float)jcamera.at(L"position")->AsArray().at(0)->AsNumber();
	float cp = (float)jcamera.at(L"position")->AsArray().at(1)->AsNumber();
	float cd = (float)jcamera.at(L"position")->AsArray().at(2)->AsNumber();
	mCamera = std::make_unique<Camera>(width, height, cy, cp, cd);

	// lights
	for (auto jlight : jlights) {
		std::string name = Utility::wstr2str(jlight->AsObject().at(L"name")->AsString());

		float y = (float)jlight->AsObject().at(L"position")->AsArray().at(0)->AsNumber();
		float p = (float)jlight->AsObject().at(L"position")->AsArray().at(1)->AsNumber();
		float d = (float)jlight->AsObject().at(L"position")->AsArray().at(2)->AsNumber();

		float r = (float)jlight->AsObject().at(L"color")->AsArray().at(0)->AsNumber();
		float g = (float)jlight->AsObject().at(L"color")->AsArray().at(1)->AsNumber();
		float b = (float)jlight->AsObject().at(L"color")->AsArray().at(2)->AsNumber();

		mLights.push_back(std::make_shared<Light>(mDevice, mContext, y, p, d, Color(r,g,b), name));
	}

	// models
	for (auto jmodel : jmodels) {
		float x = (float)jmodel->AsObject().at(L"position")->AsArray().at(0)->AsNumber();
		float y = (float)jmodel->AsObject().at(L"position")->AsArray().at(1)->AsNumber();
		float z = (float)jmodel->AsObject().at(L"position")->AsArray().at(2)->AsNumber();

		float rx = (float)jmodel->AsObject().at(L"rotation")->AsArray().at(0)->AsNumber();
		float ry = (float)jmodel->AsObject().at(L"rotation")->AsArray().at(1)->AsNumber();

		std::string name = Utility::wstr2str(jmodel->AsObject().at(L"name")->AsString());

		std::wstring resourcePath = Utility::str2wstr(gConfig.ResourcePath);
		std::wstring meshPath = resourcePath + jmodel->AsObject().at(L"mesh")->AsString();
		std::wstring colorPath = resourcePath + jmodel->AsObject().at(L"color")->AsString();
		std::wstring normalPath = resourcePath + jmodel->AsObject().at(L"normal")->AsString();
		std::wstring specularPath = resourcePath + jmodel->AsObject().at(L"specular")->AsString();
		std::wstring discolorPath = resourcePath + jmodel->AsObject().at(L"discolor")->AsString();
		std::wstring occlusionPath = resourcePath + jmodel->AsObject().at(L"occlusion")->AsString();

		mModels.push_back(std::make_shared<Entity>(mDevice, Vector3(x,y,z), Vector2(rx, ry), 
			meshPath, colorPath, normalPath, specularPath, discolorPath, occlusionPath));
	}

	return true;
}


bool Application::SetupRenderer()
{
	RECT rect; GetClientRect(mHwnd, &rect);
	uint32_t width = uint32_t(rect.right - rect.left);
	uint32_t height = uint32_t(rect.bottom - rect.top);

	mRenderer = std::make_unique<Renderer>(mHwnd, width, height);

	mDevice = mRenderer->mDevice;
	mContext = mRenderer->mContext;
	mSwapChain = mRenderer->mSwapChain;

	mGenerator = std::make_unique<Generator>(mDevice, mContext);

	return true;
}


bool Application::SetupDashboard()
{
	std::wstring resourcePath = Utility::str2wstr(gConfig.ResourcePath);
	std::wstring spriteFontFile = resourcePath + L"Fonts\\Arial12.spritefont";
	
	mDashboard = std::make_unique<Dashboard>(mHwnd, mDevice, mContext);

	// Sprite batch and sprite font enable simple text rendering
	mSpriteBatch = std::make_unique<SpriteBatch>(mContext.Get());
	mSpriteFont = std::make_unique<SpriteFont>(mDevice.Get(), spriteFontFile.c_str());

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

	// Reload scene
	if (ImGui::IsKeyPressed(ImGuiKey_R)) {
		Reload();
		return true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
		gConfig.EnableDashboard = !gConfig.EnableDashboard;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_P)) { // cycle through pick modes
		gConfig.PickMode = (gConfig.PickMode == PickType::CARVE) ? gConfig.PickMode = PickType::PAINT : PickType((int)gConfig.PickMode + 1);
		mPointA.release();
		mPointB.release();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_S)) {
		gConfig.SplitMode = (gConfig.SplitMode == SplitType::SPLIT6) ? SplitType::SPLIT3 : SplitType((int)gConfig.SplitMode + 1);
		mPointA.release();
		mPointB.release();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_W)) {
		gConfig.EnableWireframe = !gConfig.EnableWireframe;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_T)) {
		PerformanceTest();
	}

	mDashboard->Update();

	if (!io.KeyCtrl && !io.KeyShift && !mPointA && !mPointB && !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
		mCamera->Update();
	}

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
        Pick();
    }

	for (auto& light : mLights) {
		light->Update();
	}

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
	if (gConfig.EnableDashboard)
	{
		std::vector<Light*> lights;
		for (auto& light : mLights) {
			lights.push_back(light.get());
		}

		// render dashboard (user interface panels)
		mDashboard->Render(lights);

		// render text
		RECT rect; GetClientRect(mHwnd, &rect);
		uint32_t width = rect.right - rect.left;
		uint32_t height = rect.bottom - rect.top;

		std::wstring pickText = std::wstring(L"ick mode: ") + Utility::str2wstr(ToString(gConfig.PickMode));
		std::wstring splitText = std::wstring(L"plit mode: ") + Utility::str2wstr(ToString(gConfig.SplitMode));

		DirectX::XMFLOAT2 ptv, stv;
		DirectX::XMStoreFloat2(&ptv, mSpriteFont->MeasureString(pickText.c_str()));
		DirectX::XMStoreFloat2(&stv, mSpriteFont->MeasureString(splitText.c_str()));

		mSpriteBatch->Begin();
		{
			mSpriteFont->DrawString(mSpriteBatch.get(), L"P", Vector2(float(width - ptv.x - 22), float(height - 44)), DirectX::Colors::Orange);
			mSpriteFont->DrawString(mSpriteBatch.get(), pickText.c_str(), Vector2(float(width - ptv.x - 11), float(height - 44)), DirectX::Colors::LightGray);

			mSpriteFont->DrawString(mSpriteBatch.get(), L"S", Vector2(float(width - stv.x - 22), float(height - 22)), DirectX::Colors::Orange);
			mSpriteFont->DrawString(mSpriteBatch.get(), splitText.c_str(), Vector2(float(width - stv.x - 11), float(height - 22)), DirectX::Colors::LightGray);
		}
		mSpriteBatch->End();
	}

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



void Application::Pick()
{
	RECT rect;
	GetClientRect(mHwnd, &rect);
	ImGuiIO& io = ImGui::GetIO();

	// Acquire viewport dimensions, cursor position, window dimensions
	Vector2 cursor(io.MousePos.x, io.MousePos.y); // in window coordinates
	Vector2 resolution((float)mRenderer->mWidth, (float)mRenderer->mHeight);
	Vector2 window((float)rect.right - (float)rect.left - 1, (float)rect.bottom - (float)rect.top - 1);

	// Find closest ray-mesh intersection
	Intersection ix = FindIntersection(cursor, resolution, window, mCamera->mProjection, mCamera->mView);
	if (!ix.hit) {
		throw std::exception("No intersection");
	}

	// Keep track of number of points selected
	if (!mPointA) {
		mPointA = std::make_unique<Intersection>(ix);
	}
	else if (!mPointB) {
		mPointB = std::make_unique<Intersection>(ix);
	}

	// Create new cut when two points were selected
	if (mPointA && mPointB) {
		CreateCut(*mPointA.get(), *mPointB.get());
		mPointA.reset();
		mPointB.reset();
	}
}


void Application::CreateCut(Intersection& a, Intersection& b)
{
	if (!a.model || !b.model || a.model.get() != b.model.get()) {
		Utility::DialogMessage("Invalid selection");
		return;
	}

	Stopwatch sw(CLOCK_QPC_MS);
	auto& model = a.model;
	Quadrilateral cutQuad;
	std::list<Link> cutLine;
	std::vector<Edge*> cutEdges;
	std::shared_ptr<Target> patch;

	// Find all triangles intersected by the cutting quad, and order them into a chain of segments
	sw.Start("1] Form cutting line");
	model->FormCutline(a, b, cutLine, cutQuad);
	sw.Stop("1] Form cutting line");

	// Generate wound patch texture (must be done first because mesh elements will be deleted later)
	sw.Start("2] Generate wound patch");
	CreateWound(cutLine, model, patch);
	sw.Stop("2] Generate wound patch");

	// Paint wound patch onto mesh color texture
	sw.Start("3] Paint wound patch");
	PaintWound(cutLine, model, patch);
	sw.Stop("3] Paint wound patch");

	// Only draw wound texture
	if (gConfig.PickMode == PickType::PAINT) {
		return;
	}

	// Fuse cutting line into mesh
	if (gConfig.PickMode >= PickType::MERGE) {
		sw.Start("4] Fuse cutting line");
		model->FuseCutline(cutLine, cutEdges);
		sw.Stop("4] Fuse cutting line");
	}

	// Open carve cutting line into mesh
	if (gConfig.PickMode == PickType::CARVE) {
		sw.Start("5] Carve incision");
		model->OpenCutLine(cutEdges, cutQuad);
		sw.Stop("5] Carve incision");
	}

#ifdef _DEBUG
	sw.Report();
#endif
}


void Application::Split()
{
	RECT rect;
	GetClientRect(mHwnd, &rect);
	ImGuiIO& io = ImGui::GetIO();

	Vector2 cursor = Vector2(io.MousePos.x, io.MousePos.y);
	Vector2 resolution((float)mRenderer->mWidth, (float)mRenderer->mHeight);
	Vector2 window = Vector2((float)rect.right - (float)rect.left - 1, (float)rect.bottom - (float)rect.top - 1);

	Intersection ix = FindIntersection(cursor, resolution, window, mCamera->mProjection, mCamera->mView);
	if (!ix.hit) {
		throw std::exception("No intersection");
	}
	ix.model->Subdivide(ix.face, gConfig.SplitMode, ix.pos_os);
}


void Application::DrawDecal()
{
	RECT rect;
	GetClientRect(mHwnd, &rect);
	ImGuiIO& io = ImGui::GetIO();

	Vector2 cursor(io.MousePos.x, io.MousePos.y);
	Vector2 resolution((float)mRenderer->mWidth, (float)mRenderer->mHeight);
	Vector2 window((float)rect.right - (float)rect.left - 1, (float)rect.bottom - (float)rect.top - 1);

	Intersection ix = FindIntersection(cursor, resolution, window, mCamera->mProjection, mCamera->mView);
	if (!ix.hit) {
		throw std::exception("No intersection");
	}
	mRenderer->CreateWoundDecal(ix);
}


Intersection Application::FindIntersection(Vector2 cursor, Vector2 resolution, Vector2 window, Matrix proj, Matrix view)
{
	// convert screen-space position into viewport space
	Vector2 screenPos = Vector2((cursor.x * resolution.x) / window.x, (cursor.y * resolution.y) / window.y);

	// Create an object-space ray from the given screen-space position.
	Ray ray = CreateRay(screenPos, resolution, proj, view);


	Intersection ix;
	ix.hit = false;
	ix.ray = ray;
	ix.model = nullptr;
	ix.pos_ss = screenPos;
	ix.nearz = Camera::cNearPlane;
	ix.farz = Camera::cFarPlane;

	// find closest model that intersects with ray
	constexpr float tmin = std::numeric_limits<float>::max();

	for (auto& model : mModels) {
		if (model->RayIntersection(ray, ix)) {
			ix.hit = true;
			if (ix.dist < tmin) {
				ix.model = model;
			}
		}
	}

	if (ix.hit) {
		ix.pos_ws = Vector3::Transform(ix.pos_os, ix.model->mMatrixWorld);
	}

	return ix;
}


void Application::CreateWound(std::list<Link>& cutline, std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch)
{
	// determine height/width of color map
	D3D11_TEXTURE2D_DESC colorDesc;
	ComPtr<ID3D11Texture2D> colorTex;
	Utility::GetTexture2D(model->mColorMap, colorTex, colorDesc);
	float texWidth = (float)colorDesc.Width;
	float texHeight = (float)colorDesc.Height;

	// convert y range from [1,0] to [0,1]
	Vector2 p0 = Vector2(cutline.front().x0.x, 1.0f - cutline.front().x0.y);
	Vector2 p1 = Vector2(cutline.back().x1.x, 1.0f - cutline.back().x1.y);

	// target texture width/height in pixels
	uint32_t pixelWidth = uint32_t(Vector2::Distance(p0, p1) * texWidth);
	uint32_t pixelHeight = uint32_t(2.0f * std::log10f((float)pixelWidth) * std::sqrtf((float)pixelWidth));

	// generate wound patch
	patch = mGenerator->GenerateWoundPatch(pixelWidth, pixelHeight);
}


void Application::PaintWound(std::list<Link>& cutLine, std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch)
{
	// Compute cut length and height of wound patch in texture-space
	float cutLength = 0;
	for (auto& link : cutLine) {
		cutLength += Vector2::Distance(link.x0, link.x1);
	}

	// height of cut is based on ratio of pixel height to pixel width of the wound patch texture
	float cutHeight = cutLength * patch->mViewport.Height / patch->mViewport.Width;

	// Find faces closest to each line segment
	LinkFaceMap cf;
	model->ChainFaces(cutLine, cf, cutHeight);

	// Paint to color and discolor maps
	mRenderer->PaintWoundPatch(model, patch, cf, cutLength, cutHeight);
	mRenderer->PaintDiscoloration(model, cf, cutHeight);
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
			gResizeWidth = (UINT)LOWORD(lParam); // Queue resize
			gResizeHeight = (UINT)HIWORD(lParam);
			return 0;
		}
		case WM_SYSCOMMAND: {
			if ((wParam & 0xfff0) == SC_KEYMENU) { // Disable ALT application menu
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


std::vector<std::tuple<std::wstring, Vector2, Vector2>> Application::CreateSamples(std::vector<std::pair<Vector2, Vector2>>& locations, std::vector<float>& lengths, std::wstring setName)
{
	std::vector<std::tuple<std::wstring, Vector2, Vector2>> samples;
	if (lengths.size() != locations.size()) {
		return samples;
	}

	for (uint32_t i = 0; i < locations.size(); ++i) {
		float hlength = 0.5f * lengths[i];

		Vector2 p = locations[i].first;		// position
		Vector2 d = locations[i].second;	// direction

		Vector2 p0 = p - d * hlength;
		Vector2 p1 = p + d * hlength;

		std::wstringstream wss;
		wss << setName << " " << i + 1;

		samples.push_back(std::make_tuple(wss.str(), p0, p1));
	}

	return samples;
}

void Application::RunTest(std::vector<std::tuple<std::wstring, Vector2, Vector2>>& samples, Vector2& resolution, Vector2& window, Matrix& projection, Matrix& view)
{
	for (auto& sample : samples) {
		std::array<long long, 5> stageTime = {}; // init values to zero

		for (uint32_t run = 0; run < (uint32_t)cNumTestRuns; ++run) {
			Stopwatch sw;

			Intersection ix0 = FindIntersection(std::get<1>(sample), resolution, window, projection, view);
			Intersection ix1 = FindIntersection(std::get<2>(sample), resolution, window, projection, view);

			Quadrilateral cutQuad;
			std::list<Link> cutLine;
			std::vector<Edge*> cutEdges;
			std::shared_ptr<Target> patch;

			sw.Start("1");
			ix0.model->FormCutline(ix0, ix1, cutLine, cutQuad);
			sw.Stop("1");

			sw.Start("2");
			CreateWound(cutLine, ix0.model, patch);
			sw.Stop("2");

			sw.Start("3");
			PaintWound(cutLine, ix0.model, patch);
			sw.Stop("3");

			sw.Start("4");
			ix0.model->FuseCutline(cutLine, cutEdges);
			sw.Stop("4");

			sw.Start("5");
			ix0.model->OpenCutLine(cutEdges, cutQuad);
			sw.Stop("5");

			stageTime[0] += sw.ElapsedTime("1");
			stageTime[1] += sw.ElapsedTime("2");
			stageTime[2] += sw.ElapsedTime("3");
			stageTime[3] += sw.ElapsedTime("4");
			stageTime[4] += sw.ElapsedTime("5");

			ix0.model->Reload();
		}


		std::wstringstream wss;
		wss << std::get<0>(sample) << std::endl;

		double total_time = 0.0;
		for (uint32_t ti = 0; ti < stageTime.size(); ti++)
		{
			double stageTimeAvg = stageTime[ti] / (double)cNumTestRuns / 1000.0;
			wss << stageTimeAvg << std::endl;
			total_time += stageTimeAvg;
		}

		wss << total_time << std::endl;
		Utility::ConsoleMessageW(wss.str());
	}
}


void Application::PerformanceTest()
{
	RECT rect; GetClientRect(mHwnd, &rect);
	Vector2 resolution(float(mRenderer->mWidth), float(mRenderer->mHeight));
	Vector2 window(float(rect.right) - float(rect.left - 1), float(rect.bottom) - float(rect.top - 1));

	Matrix proj(3.047189f, 0.00000000f, 0.000000000f, 0.0f, 
	            0.000000f, 5.67128229f, 0.000000000f, 0.0f, 
	            0.000000f, 0.00000000f, 1.005025150f, 1.0f, 
	            0.000000f, 0.00000000f,-0.100502513f, 0.0f);

	Matrix view(1.0f, 0.0f, 0.0f, 0.0f, 
	            0.0f, 1.0f, 0.0f, 0.0f, 
	            0.0f, 0.0f, 1.0f, 0.0f, 
	            0.0f, 0.0f, 5.0f, 1.0f);

	// screen-space locations and normalized direction vectors based on manual samples
	std::vector<std::pair<Vector2, Vector2>> sampleLocations = {
		std::make_pair(Vector2(638.0f, 175.0f), Vector2( 0.999650240f, 0.026445773f)),
		std::make_pair(Vector2(754.0f, 342.0f), Vector2(-0.138322249f, 0.990387261f)),
		std::make_pair(Vector2(618.0f, 618.0f), Vector2( 0.899437010f, 0.437050372f)),
		std::make_pair(Vector2(692.0f, 375.0f), Vector2( 0.474099845f, 0.880471110f)),
		std::make_pair(Vector2(582.0f, 346.0f), Vector2(-0.651344180f, 0.758782387f)),
		std::make_pair(Vector2(631.0f, 467.0f), Vector2( 0.978677809f, 0.205401510f))
	};

	std::vector<float> cutSizeLrg(sampleLocations.size(), 160.0f);
	std::vector<float> cutSizeMed(sampleLocations.size(), 80.0f);
	std::vector<float> cutSizeSml(sampleLocations.size(), 40.0f);

	auto cutSamplesLrg = CreateSamples(sampleLocations, cutSizeLrg, L"large");	// large-sized cuts
	auto cutSamplesMed = CreateSamples(sampleLocations, cutSizeMed, L"large");	// medium-sized cuts (50% of large)
	auto cutSamplesSml = CreateSamples(sampleLocations, cutSizeSml, L"small");	// small-sized cuts (25% of large)

	RunTest(cutSamplesLrg, resolution, window, proj, view);
	RunTest(cutSamplesMed, resolution, window, proj, view);
	RunTest(cutSamplesSml, resolution, window, proj, view);
}
