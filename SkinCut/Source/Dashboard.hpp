#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl/client.h>


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Light;
	struct Configuration;
	extern Configuration gConfig;

	class Dashboard
	{
	public:
		Dashboard(HWND hwnd, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
		~Dashboard();

		void Update();
		void Render(std::vector<Light*>& lights);
	};

}

