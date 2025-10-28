#pragma once

#include <memory>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>


namespace SkinCut {
	class Light;

	class Interface {
	public:
		Interface(HWND hwnd, 
			      Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			      Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
		~Interface();

		void Update();
		void Render(std::vector<std::shared_ptr<Light>>& lights);
	};
}
