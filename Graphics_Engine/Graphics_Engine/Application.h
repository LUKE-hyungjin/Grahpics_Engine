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

    //struct ModelViewProjectionConstantBuffer
    //{
    //    Matrix model;
    //    Matrix view;
    //    Matrix projection;
    //};

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

        //ModelViewProjectionConstantBuffer m_constantBufferData;

        bool m_usePerspectiveProjection = true;
    };
} // namespace hlab