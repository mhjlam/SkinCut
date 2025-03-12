#pragma once

#include <list>
#include <memory>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"
#include "Structs.hpp"


namespace SkinCut
{
    class Camera;
    class Entity;
    class Shader;
    class Tester;
    class Renderer;
    class RenderTarget;


    class Cutter
    {
    public:
        friend class Tester;

    private:
        std::unique_ptr<Intersection> mIntersect0;
        std::unique_ptr<Intersection> mIntersect1;

        std::shared_ptr<Camera> mCamera;
        std::shared_ptr<Renderer> mRenderer;
        std::vector<std::shared_ptr<Entity>> mModels;

        std::shared_ptr<Shader> mShaderStretch;
        std::shared_ptr<Shader> mShaderPatch;

        Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;


    public:
        Cutter(std::shared_ptr<Renderer> renderer, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Entity>>& models);
        ~Cutter() = default;

        void Reset() {
            mIntersect0.reset();
            mIntersect1.reset();
        };

        bool HasSelection() const {
            return mIntersect0 != nullptr || mIntersect1 != nullptr;
        }

        void Pick(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view);
        void Split(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view);

    private:
        void Cut(Intersection& ia, Intersection& ib);
        void GenPatch(std::list<Link>& cutLine, std::shared_ptr<Entity>& model, std::shared_ptr<RenderTarget>& patch);
        void DrawPatch(std::list<Link>& cutLine, std::shared_ptr<Entity>& model, std::shared_ptr<RenderTarget>& patch);

        Intersection Intersect(Math::Vector2 cursor, Math::Vector2 resolution, Math::Vector2 window, Math::Matrix proj, Math::Matrix view);
    };
}