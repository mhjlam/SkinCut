#include "Cutter.hpp"

#include <d3d11.h>

#include <imgui.h>

#include "Util.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "Constants.hpp"
#include "StopWatch.hpp"
#include "RenderTarget.hpp"
#include "VertexBuffer.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


namespace SkinCut {
	extern Configuration g_Config;
}


Cutter::Cutter(std::shared_ptr<Renderer> renderer, std::shared_ptr<Camera> camera, 
			   std::vector<std::shared_ptr<Model>>& models) 
: m_Renderer(renderer), m_Camera(camera), m_Models(models) {
	std::wstring shaderPath = Util::wstr(g_Config.ResourcePath) + L"Shaders\\";

	m_Device = renderer->Device;
	m_Context = renderer->DeviceContext;

	m_ShaderStretch = std::make_shared<Shader>(m_Device, m_Context, 
											  shaderPath + L"Stretch.vs.cso", 
											  shaderPath + L"Stretch.ps.cso");
	m_ShaderPatch = std::make_shared<Shader>(m_Device, m_Context, 
											shaderPath + L"Pass.vs.cso", 
											shaderPath + L"Patch.ps.cso");
}

void Cutter::Pick(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view) {
	ImGuiIO& io = ImGui::GetIO();

	// Acquire cursor position in window dimensions
	Math::Vector2 cursor(io.MousePos.x, io.MousePos.y);

	// Find closest ray-mesh intersection
	Intersection ix = Intersect(cursor, resolution, window, projection, view);
	if (!ix.Hit) {
		return;
	}

	// Keep track of number of points selected
	if (!m_Intersection0) {
		m_Intersection0 = std::make_unique<Intersection>(ix);
	}
	else if (!m_Intersection1) {
		m_Intersection1 = std::make_unique<Intersection>(ix);
	}

	// Create new cut when two points were selected
	if (m_Intersection0 && m_Intersection1) {
		Cut(*m_Intersection0.get(), *m_Intersection1.get());
		Reset();
	}
}

void Cutter::Split(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view) {
	ImGuiIO& io = ImGui::GetIO();
	Math::Vector2 cursor(io.MousePos.x, io.MousePos.y);

	Intersection ix = Intersect(cursor, resolution, window, projection, view);
	if (ix.Hit) {
		ix.Model->Subdivide(ix.Face, g_Config.SplitMode, ix.PositionObject);
	}
}

void Cutter::Cut(Intersection& a, Intersection& b) {
	if (!a.Model || !b.Model || a.Model.get() != b.Model.get()) {
		Util::DialogMessage("Invalid selection");
		return;
	}

	StopWatch sw(ClockType::QPC_MS);
	auto& model = a.Model;
	Math::Quadrilateral cutQuad;
	LinkList cutLine;
	std::vector<Edge*> cutEdges;
	std::shared_ptr<RenderTarget> patch;

	// Find all triangles intersected by the cutting quad, and order them into a chain of segments
	sw.Start("1] Form cutting line");
	model->FormCutline(a, b, cutLine, cutQuad);
	sw.Stop("1] Form cutting line");

	// Generate wound patch texture (must be done first because mesh elements will be deleted later)
	sw.Start("2] Generate wound patch");
	GenPatch(cutLine, model, patch);
	sw.Stop("2] Generate wound patch");

	// Paint wound patch onto mesh color texture
	sw.Start("3] Paint wound patch");
	DrawPatch(cutLine, model, patch);
	sw.Stop("3] Paint wound patch");

	// Only draw wound texture
	if (g_Config.PickMode == PickType::PAINT) {
		return;
	}

	// Fuse cutting line into mesh
	if (g_Config.PickMode >= PickType::MERGE) {
		sw.Start("4] Fuse cutting line");
		model->FuseCutline(cutLine, cutEdges);
		sw.Stop("4] Fuse cutting line");
	}

	// Open carve cutting line into mesh
	if (g_Config.PickMode == PickType::CARVE) {
		sw.Start("5] Carve incision");
		model->OpenCutLine(cutEdges, cutQuad);
		sw.Stop("5] Carve incision");
	}

#ifdef _DEBUG
	sw.Report();
#endif
}

void Cutter::GenPatch(std::list<Link>& cutline, std::shared_ptr<Model>& model, std::shared_ptr<RenderTarget>& patch) {
	// Determine height/width of color map
	D3D11_TEXTURE2D_DESC colorDesc;
	ComPtr<ID3D11Texture2D> colorTex;
	Util::GetTexture2D(model->ColorMap, colorTex, colorDesc);
	float texWidth = (float)colorDesc.Width;
	float texHeight = (float)colorDesc.Height;

	// Convert y range from [1,0] to [0,1]
	Math::Vector2 p0(cutline.front().TexCoord0.x, 1.0f - cutline.front().TexCoord0.y);
	Math::Vector2 p1(cutline.back().TexCoord1.x, 1.0f - cutline.back().TexCoord1.y);

	// Target texture width/height in pixels
	uint32_t pixelWidth = uint32_t(Math::Vector2::Distance(p0, p1) * texWidth);
	uint32_t pixelHeight = uint32_t(2.0f * std::log10f((float)pixelWidth) * std::sqrtf((float)pixelWidth));

	// Generate wound patch
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	HREXCEPT(m_Context->Map(m_ShaderPatch->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource));
	CB_PATCH_PS* patchBuffer = (CB_PATCH_PS*)mappedSubResource.pData;
	patchBuffer->Discolor = Math::Color(0.58f, 0.26f, 0.29f, 1.00f); // float4(0.58, 0.27, 0.28, 1.00);
	patchBuffer->LightColor = Math::Color(0.89f, 0.71f, 0.65f, 1.00f); // float4(0.65, 0.36, 0.37, 1.00);
	patchBuffer->InnerColor = Math::Color(0.54f, 0.00f, 0.01f, 1.00f);
	patchBuffer->OffsetX = Util::Random(0.0f, 100.0f);
	patchBuffer->OffsetY = Util::Random(0.0f, 100.0f);
	m_Context->Unmap(m_ShaderPatch->m_PixelBuffers[0].Get(), 0);

	auto buffer = std::make_shared<VertexBuffer>(m_Device);
	auto target = std::make_shared<RenderTarget>(m_Device, m_Context, pixelWidth, pixelHeight, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, false);

	m_Context->IASetInputLayout(m_ShaderPatch->m_InputLayout.Get());
	m_Context->IASetPrimitiveTopology(buffer->m_Topology);
	m_Context->IASetVertexBuffers(0, 1, buffer->m_Buffer.GetAddressOf(), &buffer->m_Strides, &buffer->m_Offsets);

	m_Context->VSSetShader(m_ShaderPatch->m_VertexShader.Get(), nullptr, 0);

	m_Context->PSSetConstantBuffers(0, static_cast<uint32_t>(m_ShaderPatch->m_PixelBuffers.size()), m_ShaderPatch->m_PixelBuffers[0].GetAddressOf());
	m_Context->PSSetShader(m_ShaderPatch->m_PixelShader.Get(), nullptr, 0);

	m_Context->RSSetState(nullptr); // default rasterizer
	m_Context->RSSetViewports(1, &target->m_Viewport);

	m_Context->OMSetRenderTargets(1, target->m_RenderTarget.GetAddressOf(), nullptr);
	m_Context->OMSetBlendState(target->m_BlendState.Get(), target->m_BlendFactor, target->m_SampleMask);
	m_Context->OMSetDepthStencilState(m_ShaderPatch->m_DepthState.Get(), 0);

	m_Context->Draw(buffer->m_NumVertices, 0);

	m_Context->OMSetRenderTargets(0, nullptr, nullptr);

	patch = target;
}

void Cutter::DrawPatch(std::list<Link>& cutLine, std::shared_ptr<Model>& model, std::shared_ptr<RenderTarget>& patch) {
	// Compute cut length and height of wound patch in texture-space
	float cutLength = 0;
	for (auto& link : cutLine) {
		cutLength += Math::Vector2::Distance(link.TexCoord0, link.TexCoord1);
	}

	// height of cut is based on ratio of pixel height to pixel width of the wound patch texture
	float cutHeight = cutLength * patch->m_Viewport.Height / patch->m_Viewport.Width;

	// Find faces closest to each line segment
	LinkFaceMap faces;
	model->ChainFaces(cutLine, faces, cutHeight);

	// Paint to color and discolor maps
	m_Renderer->ApplyPatch(model, patch, faces, cutLength, cutHeight);
	m_Renderer->ApplyDiscolor(model, faces, cutHeight);
}

Intersection Cutter::Intersect(Math::Vector2 cursor, Math::Vector2 resolution, 
							   Math::Vector2 window, Math::Matrix proj, Math::Matrix view) {
	// convert screen-space position into viewport space
	Math::Vector2 screenPos((cursor.x * resolution.x) / window.x, (cursor.y * resolution.y) / window.y);

	// Create an object-space ray from the given screen-space position.
	Math::Ray ray = CreateRay(screenPos, resolution, proj, view);

	Intersection intersect;
	intersect.Hit = false;
	intersect.Ray = ray;
	intersect.Model = nullptr;
	intersect.PositionScreen = screenPos;
	intersect.NearZ = Constants::NEAR_PLANE;
	intersect.FarZ = Constants::FAR_PLANE;

	// find closest model that intersects with ray
	for (auto& model : m_Models) {
		if (model->RayIntersection(ray, intersect)) {
			if (intersect.Distance < Constants::FLOAT_MAX) {
				intersect.Model = model;
				intersect.Hit = true;
			}
		}
	}

	if (intersect.Hit) {
		intersect.PositionWorld = Math::Vector3::Transform(intersect.PositionObject, intersect.Model->World);
	}

	return intersect;
}
