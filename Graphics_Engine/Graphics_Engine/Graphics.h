﻿#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <wrl.h> // ComPtr
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace luke
{

  using Microsoft::WRL::ComPtr;
  using std::vector;
  using std::wstring;

  // 모든 예제들이 공통적으로 사용할 기능들을 가지고 있는
  // 부모 클래스
  class Graphics
  {
  public:
    Graphics();
    virtual ~Graphics();

    bool CreateSwapchain(DXGI_SWAP_CHAIN_DESC desc);
    bool CreateRenderTargetView(ID3D11Resource *pResource,
                                const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
                                ID3D11RenderTargetView **ppRTView);
    float GetAspectRatio() const;

    int Run();

    virtual bool Initialize(HWND hwnd, UINT width, UINT height);
    virtual void UpdateGUI() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

    //virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Convenience overrides for handling mouse input.
    virtual void OnMouseDown(WPARAM btnState, int x, int y) {};
    virtual void OnMouseUp(WPARAM btnState, int x, int y) {};
    virtual void OnMouseMove(WPARAM btnState, int x, int y) {};

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitDirect3D(HWND hwnd);
    bool InitGUI();
    void CreateVertexShaderAndInputLayout(const wstring &filename,
                                          const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
                                          ComPtr<ID3D11VertexShader> &vertexShader,
                                          ComPtr<ID3D11InputLayout> &inputLayout);
    void CreatePixelShader(const wstring &filename, ComPtr<ID3D11PixelShader> &pixelShader);
    void CreateIndexBuffer(const vector<uint16_t> &indices, ComPtr<ID3D11Buffer> &m_indexBuffer);

    template <typename T_VERTEX>
    void CreateVertexBuffer(const vector<T_VERTEX> &vertices, ComPtr<ID3D11Buffer> &vertexBuffer)
    {

      // D3D11_USAGE enumeration (d3d11.h)
      // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

      D3D11_BUFFER_DESC bufferDesc;
      ZeroMemory(&bufferDesc, sizeof(bufferDesc));
      bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
      bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
      bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
      bufferDesc.StructureByteStride = sizeof(T_VERTEX);

      D3D11_SUBRESOURCE_DATA vertexBufferData = {0}; // MS 예제에서 초기화하는 방식
      vertexBufferData.pSysMem = vertices.data();
      vertexBufferData.SysMemPitch = 0;
      vertexBufferData.SysMemSlicePitch = 0;

      const HRESULT hr =
          m_device->CreateBuffer(&bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
      if (FAILED(hr))
      {
        std::cout << "CreateBuffer() failed. " << std::hex << hr << std::endl;
      };
    }

    template <typename T_CONSTANT>
    void CreateConstantBuffer(const T_CONSTANT &constantBufferData,
                              ComPtr<ID3D11Buffer> &constantBuffer)
    {
      D3D11_BUFFER_DESC cbDesc;
      cbDesc.ByteWidth = sizeof(constantBufferData);
      cbDesc.Usage = D3D11_USAGE_DYNAMIC;
      cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      cbDesc.MiscFlags = 0;
      cbDesc.StructureByteStride = 0;

      // Fill in the subresource data.
      D3D11_SUBRESOURCE_DATA InitData;
      InitData.pSysMem = &constantBufferData;
      InitData.SysMemPitch = 0;
      InitData.SysMemSlicePitch = 0;

      m_device->CreateBuffer(&cbDesc, &InitData, constantBuffer.GetAddressOf());
    }

    template <typename T_DATA>
    void UpdateBuffer(const T_DATA &bufferData, ComPtr<ID3D11Buffer> &buffer)
    {
      D3D11_MAPPED_SUBRESOURCE ms;
      m_context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
      memcpy(ms.pData, &bufferData, sizeof(bufferData));
      m_context->Unmap(buffer.Get(), NULL);
    }

  public:
    int m_screenWidth; // 렌더링할 최종 화면의 해상도
    int m_screenHeight;
    HWND m_mainWindow;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<ID3D11Texture2D> mFrameBuffer;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RasterizerState> m_rasterizerSate;

    // Depth buffer 관련
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    D3D11_VIEWPORT m_screenViewport;
  };
} 