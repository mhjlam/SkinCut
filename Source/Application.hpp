#pragma once

#include <array>
#include <tuple>
#include <memory>
#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <windows.h>
#include <wrl/client.h>

#include <SpriteFont.h>
#include <SpriteBatch.h>

#include "Types.hpp"
#include "Constants.hpp"


namespace SkinCut {
	class Light;
	class Model;
	class Camera;
	class Cutter;
	class Renderer;
	class Interface;

	class Application {
	public:
		Application();
		virtual ~Application();

		virtual bool Init(HWND hWnd, const std::string& resourcePath);
		virtual bool Update();
		virtual bool Render();
		virtual bool Reload();

		LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		bool LoadConfig();
		bool LoadScene();
		bool InitRenderer();
		bool InitCutter();
		bool InitInterface();

	protected:
		HWND m_WindowHandle;

		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	m_Context;

		std::unique_ptr<DirectX::SpriteFont> m_SpriteFont;
		std::unique_ptr<DirectX::SpriteBatch> m_SpriteBatch;

		std::shared_ptr<Camera>	m_Camera;
		std::shared_ptr<Cutter>	m_Cutter;
		std::shared_ptr<Renderer> m_Renderer;

		std::unique_ptr<Interface> m_Interface;

		std::vector<std::shared_ptr<Light>> m_Lights;
		std::vector<std::shared_ptr<Model>> m_Models;
	};
}
