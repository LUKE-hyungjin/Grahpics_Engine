﻿
#include "Application.h"
#include "MeshGenerator.h"
#include <tuple>
#include <vector>

namespace luke
{

    using namespace std;

    Application::Application() : Graphics(), m_indexCount(0){}

    bool Application::Initialize(HWND hwnd, UINT width, UINT height)
    {
        if (!Graphics::Initialize(hwnd, width, height))
            return false;
        m_aspect = Graphics::GetAspectRatio();

#pragma region Geometry 정의
        MeshData meshData = MeshGenerator::MakeCube();
        m_mesh = std::make_shared<Mesh>();
#pragma endregion

#pragma region 버텍스 버퍼 만들기
        Graphics::CreateVertexBuffer(meshData.vertices, m_mesh->m_vertexBuffer);
#pragma endregion

#pragma region 인덱스 버퍼 만들기
        m_indexCount = UINT(meshData.indices.size());
        Graphics::CreateIndexBuffer(meshData.indices, m_mesh->m_indexBuffer);
#pragma endregion

#pragma region ConstantBuffer 만들기
        m_constantBufferData.model = Matrix();
        m_constantBufferData.view = Matrix();
        m_constantBufferData.projection = Matrix();
        Graphics::CreateConstantBuffer(m_constantBufferData, m_mesh->m_constantBuffer);
#pragma endregion

#pragma region 쉐이더 만들기
        vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        Graphics::CreateVertexShaderAndInputLayout(L"../Shader_Source/ColorVertexShader.hlsl", inputElements,
                                                  m_colorVertexShader, m_colorInputLayout);

        Graphics::CreatePixelShader(L"../Shader_Source/ColorPixelShader.hlsl", m_colorPixelShader);
#pragma endregion

        return true;
    }

    void Application::Update(float dt)
    {
        using namespace DirectX;

        // 모델의 변환
        m_constantBufferData.model =
            Matrix::CreateScale(m_modelScaling) * Matrix::CreateRotationY(m_modelRotation.y) *
            Matrix::CreateRotationX(m_modelRotation.x) * Matrix::CreateRotationZ(m_modelRotation.z) *
            Matrix::CreateTranslation(m_modelTranslation);
        m_constantBufferData.model = m_constantBufferData.model.Transpose();

        // 시점 변환
        // m_constantBufferData.view = XMMatrixLookAtLH(m_viewEye, m_viewFocus, m_viewUp);
        m_constantBufferData.view = XMMatrixLookToLH(m_viewEyePos, m_viewEyeDir, m_viewUp);
        m_constantBufferData.view = m_constantBufferData.view.Transpose();

        // 프로젝션
        // m_aspect = AppBase::GetAspectRatio(); // <- GUI에서 조절
        if (m_usePerspectiveProjection) {
            m_constantBufferData.projection = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(m_projFovAngleY), m_aspect, m_nearZ, m_farZ);
        }
        else {
            m_constantBufferData.projection =
                XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
        }
        m_constantBufferData.projection = m_constantBufferData.projection.Transpose();

        // Constant를 CPU에서 GPU로 복사
        Graphics::UpdateBuffer(m_constantBufferData, m_mesh->m_constantBuffer);
    }

    void Application::Render()
    {

        // IA: Input-Assembler stage
        // VS: Vertex Shader
        // PS: Pixel Shader
        // RS: Rasterizer stage
        // OM: Output-Merger stage
        // Set the viewport
        ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
        D3D11_VIEWPORT m_screenViewport =
            {
                0,                     // TopLeftX
                0,                     // TopLeftY
                float(m_screenWidth),  // Width
                float(m_screenHeight), // Height
                0.0f,                  // MinDepth
                1.0f                   // MaxDepth, important for depth buffering
            };
        m_context->RSSetViewports(1, &m_screenViewport);

        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
        m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                         D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // 비교: Depth Buffer를 사용하지 않는 경우
        // m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

        // 어떤 쉐이더를 사용할지 설정
        m_context->VSSetShader(m_colorVertexShader.Get(), 0, 0);

        /* 경우에 따라서는 포인터의 배열을 넣어줄 수도 있습니다.
        ID3D11Buffer *pptr[1] = {
            m_constantBuffer.Get(),
        };
        m_context->VSSetConstantBuffers(0, 1, pptr); */

        m_context->VSSetConstantBuffers(0, 1, m_mesh->m_constantBuffer.GetAddressOf());
        m_context->PSSetShader(m_colorPixelShader.Get(), 0, 0);

        m_context->RSSetState(m_rasterizerSate.Get());

        // 버텍스/인덱스 버퍼 설정
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_context->IASetInputLayout(m_colorInputLayout.Get());
        m_context->IASetVertexBuffers(0, 1, m_mesh->m_vertexBuffer.GetAddressOf(), &stride, &offset);
        m_context->IASetIndexBuffer(m_mesh->m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->DrawIndexed(m_indexCount, 0, 0);
    }

    void Application::UpdateGUI()
    {
        ImGui::Checkbox("usePerspectiveProjection", &m_usePerspectiveProjection);

        ImGui::SliderFloat3("m_modelTranslation", &m_modelTranslation.x, -2.0f, 2.0f);
        ImGui::SliderFloat3("m_modelRotation(Rad)", &m_modelRotation.x, -3.14f, 3.14f);
        ImGui::SliderFloat3("m_modelScaling", &m_modelScaling.x, 0.1f, 2.0f);

        ImGui::SliderFloat3("m_viewEyePos", &m_viewEyePos.x, -4.0f, 4.0f);
        ImGui::SliderFloat3("m_viewEyeDir", &m_viewEyeDir.x, -4.0f, 4.0f);
        ImGui::SliderFloat3("m_viewUp", &m_viewUp.x, -2.0f, 2.0f);

        ImGui::SliderFloat("m_projFovAngleY(Deg)", &m_projFovAngleY, 10.0f, 180.0f);
        ImGui::SliderFloat("m_nearZ", &m_nearZ, 0.01f, 10.0f);
        ImGui::SliderFloat("m_farZ", &m_farZ, 0.01f, 10.0f);
        ImGui::SliderFloat("m_aspect", &m_aspect, 1.0f, 3.0f);
    }

}
