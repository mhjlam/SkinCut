#pragma once

#include <array>
#include <tuple>
#include <memory>
#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <windows.h>
#include <wrl/client.h>

#include "DirectXTK/Inc/SpriteFont.h"
#include "DirectXTK/Inc/SpriteBatch.h"

#include "Structures.hpp"


using DirectX::SpriteFont;
using DirectX::SpriteBatch;
using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Light;
	class Entity;
	class Decal;
	class Camera;
	class Renderer;
	class Generator;
	class Dashboard;
	class FrameBuffer;
	class Target;


	class Application
	{
	protected:
		HWND									mHwnd;

		ComPtr<ID3D11Device>					mDevice;
		ComPtr<IDXGISwapChain>					mSwapChain;
		ComPtr<ID3D11DeviceContext>				mContext;

		std::unique_ptr<SpriteFont>				mSpriteFont;
		std::unique_ptr<SpriteBatch>			mSpriteBatch;

		std::unique_ptr<Camera>					mCamera;
		std::vector<std::shared_ptr<Light>>		mLights;
		std::vector<std::shared_ptr<Entity>>	mModels;

		std::unique_ptr<Renderer>				mRenderer;
		std::unique_ptr<Dashboard>				mDashboard;
		std::unique_ptr<Generator>				mGenerator;

		std::unique_ptr<Intersection>			mPointA;
		std::unique_ptr<Intersection>			mPointB;


	public:
		Application();
		virtual ~Application();

		virtual bool Initialize(HWND hWnd, const std::string& respath);
		virtual bool Update();
		virtual bool Render();
		virtual bool Reload();
		LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected: // subroutines
		bool LoadScene();
		bool LoadConfig();
		bool SetupRenderer();
		bool SetupDashboard();

		void Pick();
		void CreateCut(Intersection& ia, Intersection& ib);

		void Split();
		void DrawDecal();

		Intersection FindIntersection(Math::Vector2 cursor, Math::Vector2 resolution, Math::Vector2 window, Math::Matrix proj, Math::Matrix view);

		void CreateWound(std::list<Link>& cutline, std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch);
		void PaintWound(std::list<Link>& cutline, std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch);


		std::vector<std::tuple<std::wstring, Math::Vector2, Math::Vector2>> CreateSamples(std::vector<std::pair<Math::Vector2, Math::Vector2>>& locations, std::vector<float>& lengths, std::wstring setName);
		void RunTest(std::vector<std::tuple<std::wstring, Math::Vector2, Math::Vector2>>& samples, Math::Vector2& resolution, Math::Vector2& window, Math::Matrix& projection, Math::Matrix& view);
		void PerformanceTest();
	};
}

