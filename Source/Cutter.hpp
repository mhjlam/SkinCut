#pragma once

#include <list>
#include <memory>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"
#include "Types.hpp"


namespace SkinCut {
    class Camera;
    class Model;
    class Shader;
    class Tester;
    class Renderer;
    class RenderTarget;

    class Cutter {
    public:
        Cutter(std::shared_ptr<Renderer> renderer, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>& models);

        void Reset() {
            m_Intersection0.reset();
            m_Intersection1.reset();
        };

        bool HasSelection() const {
            return m_Intersection0 != nullptr || m_Intersection1 != nullptr;
        }

        void Pick(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view);
        void Split(Math::Vector2 resolution, Math::Vector2 window, Math::Matrix projection, Math::Matrix view);

    private:
        void Cut(Intersection& ia, Intersection& ib);
        void GenPatch(std::list<Link>& cutLine, std::shared_ptr<Model>& model, std::shared_ptr<RenderTarget>& patch);
        void DrawPatch(std::list<Link>& cutLine, std::shared_ptr<Model>& model, std::shared_ptr<RenderTarget>& patch);

        Intersection Intersect(Math::Vector2 cursor, Math::Vector2 resolution, Math::Vector2 window, Math::Matrix proj, Math::Matrix view);

    public:
        friend class Tester; // Can access private members for testing purposes

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D11Context> m_Context;

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Renderer> m_Renderer;
        std::vector<std::shared_ptr<Model>> m_Models;

        std::shared_ptr<Shader> m_ShaderPatch;
        std::shared_ptr<Shader> m_ShaderStretch;

        std::unique_ptr<Intersection> m_Intersection0;
        std::unique_ptr<Intersection> m_Intersection1;
    };
}
