#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>
#include <vector>

#include "Graphics.h"

namespace luke
{


    using DirectX::SimpleMath::Matrix;
    using DirectX::SimpleMath::Vector3;

    struct Vertex
    {
        Vector3 position;
        Vector3 color;
    };

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

        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
        ComPtr<ID3D11Buffer> m_constantBuffer;
        UINT m_indexCount;

        //ModelViewProjectionConstantBuffer m_constantBufferData;

        bool m_usePerspectiveProjection = true;
    };
} // namespace hlab