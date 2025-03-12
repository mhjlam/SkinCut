#pragma once

#include <array>
#include <tuple>
#include <memory>
#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <windows.h>
#include <wrl/client.h>

#include <DirectXTK/Inc/SpriteFont.h>
#include <DirectXTK/Inc/SpriteBatch.h>

#include "Structs.hpp"


using DirectX::SpriteFont;
using DirectX::SpriteBatch;
using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Light;
	class Entity;
	class Camera;
	class Cutter;
	class Renderer;
	class Interface;

	class Application
	{
	protected:
		HWND									mHwnd;

		ComPtr<ID3D11Device>					mDevice;
		ComPtr<IDXGISwapChain>					mSwapChain;
		ComPtr<ID3D11DeviceContext>				mContext;

		std::shared_ptr<Camera>					mCamera;
		std::vector<std::shared_ptr<Light>>		mLights;
		std::vector<std::shared_ptr<Entity>>	mModels;

		std::shared_ptr<Renderer>				mRenderer;
		std::unique_ptr<SpriteFont>				mSpriteFont;
		std::unique_ptr<SpriteBatch>			mSpriteBatch;

		std::shared_ptr<Cutter>					mCutter;
		std::unique_ptr<Interface>				mInterface;


	public:
		Application();
		virtual ~Application();

		virtual bool Init(HWND hWnd, const std::string& resourcePath);
		virtual bool Update();
		virtual bool Render();
		virtual bool Reload();

		LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected: // subroutines
		bool LoadConfig();
		bool LoadScene();
		bool InitRenderer();
		bool InitCutter();
		bool InitInterface();
	};
}

