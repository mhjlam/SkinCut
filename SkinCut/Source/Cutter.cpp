#include "Cutter.hpp"

#include <d3d11.h>

#include <ImGui/imgui.h>

#include "Camera.hpp"
#include "Entity.hpp"
#include "Shader.hpp"
#include "Util.hpp"
#include "Renderer.hpp"
#include "StopWatch.hpp"
#include "VertexBuffer.hpp"
#include "RenderTarget.hpp"



using namespace SkinCut;
using namespace SkinCut::Math;
using Microsoft::WRL::ComPtr;


namespace SkinCut {
	extern Configuration gConfig;
}



Cutter::Cutter(std::shared_ptr<Renderer> renderer, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Entity>>& models)
: mRenderer(renderer), mCamera(camera), mModels(models)
{
	std::wstring shaderPath = Util::wstr(gConfig.ResourcePath) + L"Shaders\\";

	mDevice = renderer->mDevice;
	mContext = renderer->mContext;

	mShaderStretch = std::make_shared<Shader>(mDevice, mContext, shaderPath + L"Stretch.vs.cso", shaderPath + L"Stretch.ps.cso");
	mShaderPatch = std::make_shared<Shader>(mDevice, mContext, shaderPath + L"Pass.vs.cso", shaderPath + L"Patch.ps.cso");
}


void Cutter::Pick(Vector2 resolution, Vector2 window, Matrix projection, Matrix view)
{
	ImGuiIO& io = ImGui::GetIO();

	// Acquire cursor position in window dimensions
	Vector2 cursor(io.MousePos.x, io.MousePos.y);

	// Find closest ray-mesh intersection
	Intersection ix = Intersect(cursor, resolution, window, projection, view);
	if (!ix.Hit) {
		return;
	}

	// Keep track of number of points selected
	if (!mIntersect0) {
		mIntersect0 = std::make_unique<Intersection>(ix);
	}
	else if (!mIntersect1) {
		mIntersect1 = std::make_unique<Intersection>(ix);
	}

	// Create new cut when two points were selected
	if (mIntersect0 && mIntersect1) {
		Cut(*mIntersect0.get(), *mIntersect1.get());
		Reset();
	}
}


void Cutter::Split(Vector2 resolution, Vector2 window, Matrix projection, Matrix view)
{
	ImGuiIO& io = ImGui::GetIO();
	Vector2 cursor = Vector2(io.MousePos.x, io.MousePos.y);

	Intersection ix = Intersect(cursor, resolution, window, projection, view);
	if (!ix.Hit) {
		return;
	}
	ix.Model->Subdivide(ix.Face, gConfig.SplitMode, ix.PositionObject);
}



void Cutter::Cut(Intersection& a, Intersection& b)
{
	if (!a.Model || !b.Model || a.Model.get() != b.Model.get()) {
		Util::DialogMessage("Invalid selection");
		return;
	}

	StopWatch sw(CLOCK_QPC_MS);
	auto& model = a.Model;
	Quadrilateral cutQuad;
	std::list<Link> cutLine;
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


void Cutter::GenPatch(std::list<Link>& cutline, std::shared_ptr<Entity>& model, std::shared_ptr<RenderTarget>& patch)
{
	// Determine height/width of color map
	D3D11_TEXTURE2D_DESC colorDesc;
	ComPtr<ID3D11Texture2D> colorTex;
	Util::GetTexture2D(model->mColorMap, colorTex, colorDesc);
	float texWidth = (float)colorDesc.Width;
	float texHeight = (float)colorDesc.Height;

	// Convert y range from [1,0] to [0,1]
	Vector2 p0 = Vector2(cutline.front().TexCoord0.x, 1.0f - cutline.front().TexCoord0.y);
	Vector2 p1 = Vector2(cutline.back().TexCoord1.x, 1.0f - cutline.back().TexCoord1.y);

	// Target texture width/height in pixels
	uint32_t pixelWidth = uint32_t(Vector2::Distance(p0, p1) * texWidth);
	uint32_t pixelHeight = uint32_t(2.0f * std::log10f((float)pixelWidth) * std::sqrtf((float)pixelWidth));

	// Generate wound patch
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HREXCEPT(mContext->Map(mShaderPatch->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
	CB_PATCH_PS* patchBuffer = (CB_PATCH_PS*)mappedSubresource.pData;
	patchBuffer->Discolor = Color(0.58, 0.26, 0.29, 1.00); // float4(0.58, 0.27, 0.28, 1.00);
	patchBuffer->LightColor = Color(0.89, 0.71, 0.65, 1.00); // float4(0.65, 0.36, 0.37, 1.00);
	patchBuffer->InnerColor = Color(0.54, 0.00, 0.01, 1.00);
	patchBuffer->OffsetX = Util::Random(0.0f, 100.0f);
	patchBuffer->OffsetY = Util::Random(0.0f, 100.0f);
	mContext->Unmap(mShaderPatch->mPixelBuffers[0].Get(), 0);

	auto buffer = std::make_shared<VertexBuffer>(mDevice);
	auto target = std::make_shared<RenderTarget>(mDevice, mContext, pixelWidth, pixelHeight, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, false);

	mContext->IASetInputLayout(mShaderPatch->mInputLayout.Get());
	mContext->IASetPrimitiveTopology(buffer->mTopology);
	mContext->IASetVertexBuffers(0, 1, buffer->mBuffer.GetAddressOf(), &buffer->mStrides, &buffer->mOffsets);

	mContext->VSSetShader(mShaderPatch->mVertexShader.Get(), nullptr, 0);

	mContext->PSSetConstantBuffers(0, static_cast<uint32_t>(mShaderPatch->mPixelBuffers.size()), mShaderPatch->mPixelBuffers[0].GetAddressOf());
	mContext->PSSetShader(mShaderPatch->mPixelShader.Get(), nullptr, 0);

	mContext->RSSetState(nullptr); // default rasterizer
	mContext->RSSetViewports(1, &target->mViewport);

	mContext->OMSetRenderTargets(1, target->mRenderTarget.GetAddressOf(), nullptr);
	mContext->OMSetBlendState(target->mBlendState.Get(), target->mBlendFactor, target->mSampleMask);
	mContext->OMSetDepthStencilState(mShaderPatch->mDepthState.Get(), 0);

	mContext->Draw(buffer->mVertexCount, 0);

	mContext->OMSetRenderTargets(0, nullptr, nullptr);

	patch = target;
}


void Cutter::DrawPatch(std::list<Link>& cutLine, std::shared_ptr<Entity>& model, std::shared_ptr<RenderTarget>& patch)
{
	// Compute cut length and height of wound patch in texture-space
	float cutLength = 0;
	for (auto& link : cutLine) {
		cutLength += Vector2::Distance(link.TexCoord0, link.TexCoord1);
	}

	// height of cut is based on ratio of pixel height to pixel width of the wound patch texture
	float cutHeight = cutLength * patch->mViewport.Height / patch->mViewport.Width;

	// Find faces closest to each line segment
	LinkFaceMap faces;
	model->ChainFaces(cutLine, faces, cutHeight);

	// Paint to color and discolor maps
	mRenderer->ApplyPatch(model, patch, faces, cutLength, cutHeight);
	mRenderer->ApplyDiscolor(model, faces, cutHeight);
}


Intersection Cutter::Intersect(Vector2 cursor, Vector2 resolution, Vector2 window, Matrix proj, Matrix view)
{
	// convert screen-space position into viewport space
	Vector2 screenPos = Vector2((cursor.x * resolution.x) / window.x, (cursor.y * resolution.y) / window.y);

	// Create an object-space ray from the given screen-space position.
	Ray ray = CreateRay(screenPos, resolution, proj, view);


	Intersection ix;
	ix.Hit = false;
	ix.Ray = ray;
	ix.Model = nullptr;
	ix.PositionScreen = screenPos;
	ix.NearZ = Camera::cNearPlane;
	ix.FarZ = Camera::cFarPlane;

	// find closest model that intersects with ray
	constexpr float tmin = std::numeric_limits<float>::max();

	for (auto& model : mModels) {
		if (model->RayIntersection(ray, ix)) {
			if (ix.Distance < tmin) {
				ix.Model = model;
				ix.Hit = true;
			}
		}
	}

	if (ix.Hit) {
		ix.PositionWorld = Vector3::Transform(ix.PositionObject, ix.Model->mMatrixWorld);
	}

	return ix;
}

