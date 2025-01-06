#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <vector>
#include <memory>

#include "Graphics.h"
#include "MeshGenerator.h"
#include "Mesh.h"

namespace luke
{
    using DirectX::SimpleMath::Matrix;
    using DirectX::SimpleMath::Vector3;

    struct ModelViewProjectionConstantBuffer
    {
        Matrix model;
        Matrix view;
        Matrix projection;
    };

    class Application : public Graphics
    {
    public:
        Application();

        virtual bool Initialize(HWND hwnd, UINT width, UINT height) override;
        virtual void UpdateGUI() override;
        virtual void Update(float dt) override;
        virtual void Render() override;

    protected:
        ComPtr<ID3D11VertexShader> m_colorVertexShader;
        ComPtr<ID3D11PixelShader> m_colorPixelShader;
        ComPtr<ID3D11InputLayout> m_colorInputLayout;
        std::shared_ptr<Mesh> m_mesh;
        UINT m_indexCount;

        ModelViewProjectionConstantBuffer m_constantBufferData;

        bool m_usePerspectiveProjection = true;
        Vector3 m_modelTranslation = Vector3(0.0f);
        Vector3 m_modelRotation = Vector3(0.0f);
        Vector3 m_modelScaling = Vector3(0.5f);
        Vector3 m_viewEyePos = { 0.0f, 0.0f, -2.0f };
        Vector3 m_viewEyeDir = { 0.0f, 0.0f, 1.0f };
        Vector3 m_viewUp = { 0.0f, 1.0f, 0.0f };
        float m_projFovAngleY = 70.0f;
        float m_nearZ = 0.01f;
        float m_farZ = 100.0f;
        float m_aspect = 0.0f;
    };
} // namespace hlab