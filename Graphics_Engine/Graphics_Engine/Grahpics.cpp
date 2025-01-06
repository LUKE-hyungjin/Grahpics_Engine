// 참고: 헤더 include 순서
// https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes

#include "Graphics.h"

#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// VCPKG를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace luke
{

    using namespace std;

    // RegisterClassEx()에서 멤버 함수를 직접 등록할 수가 없기 때문에
    // 클래스의 멤버 함수에서 간접적으로 메시지를 처리할 수 있도록 도와줍니다.
    Graphics *g_graphics = nullptr;

    // 생성자
    Graphics::Graphics()
        : m_mainWindow(0),
          m_screenViewport(D3D11_VIEWPORT())
    {

        g_graphics = this;
    }

    Graphics::~Graphics()
    {
        g_graphics = nullptr;

        // Cleanup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        DestroyWindow(m_mainWindow);
    }

    bool Graphics::CreateSwapchain(DXGI_SWAP_CHAIN_DESC desc)
    {
        // IDXGIFactory를 이용한 CreateSwapChain()
        ComPtr<IDXGIDevice3> dxgiDevice;

        m_device.As(&dxgiDevice);

        ComPtr<IDXGIAdapter> dxgiAdapter;
        dxgiDevice->GetAdapter(&dxgiAdapter);

        ComPtr<IDXGIFactory> dxgiFactory;
        dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

        ComPtr<IDXGISwapChain> swapChain;
        dxgiFactory->CreateSwapChain(m_device.Get(), &desc, &swapChain);

        swapChain.As(&m_swapChain);
        return true;
        // 참고: IDXGIFactory4를 이용한 CreateSwapChainForHwnd()
        /*
        ComPtr<IDXGIFactory4> dxgiFactory;
        dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
        swapChainDesc.Width = lround(m_screenWidth); // Match the size of the window.
        swapChainDesc.Height = lround(m_screenHeight);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
        swapChainDesc.SwapEffect =
            DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Microsoft Store apps must use this SwapEffect.
        swapChainDesc.Flags = 0;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        ComPtr<IDXGISwapChain1> swapChain;
        dxgiFactory->CreateSwapChainForHwnd(m_device.Get(), m_mainWindow, &swapChainDesc, nullptr,
        nullptr, swapChain.GetAddressOf());
        */
    }
    bool Graphics::CreateRenderTargetView(ID3D11Resource *pResource,
                                         const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
                                         ID3D11RenderTargetView **ppRTView)
    {
        if (FAILED(m_device->CreateRenderTargetView(pResource, pDesc, ppRTView)))
            return false;

        return true;
    }
    float Graphics::GetAspectRatio() const { return float(m_screenWidth) / m_screenHeight; }

    int Graphics::Run()
    {
        ImGui_ImplDX11_NewFrame(); // GUI 프레임 시작
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame(); // 어떤 것들을 렌더링 할지 기록 시작
        ImGui::Begin("Scene Control");

        // ImGui가 측정해주는 Framerate 출력
        ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);

        UpdateGUI(); // 추가적으로 사용할 GUI

        ImGui::End();
        ImGui::Render(); // 렌더링할 것들 기록 끝

        Update(ImGui::GetIO().DeltaTime); // 애니메이션 같은 변화

        Render(); // 우리가 구현한 렌더링

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // GUI 렌더링

        // Switch the back buffer and the front buffer
        // 주의: ImGui RenderDrawData() 다음에 Present() 호출
        m_swapChain->Present(1, 0);

        return 0;
    }

    bool Graphics::Initialize(HWND hwnd, UINT width, UINT height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
        if (!InitDirect3D(hwnd))
            return false;

        if (!InitGUI())
            return false;

        return true;
    }


    bool Graphics::InitDirect3D(HWND hwnd)
    {
        m_mainWindow = hwnd;
        // 이 예제는 Intel 내장 그래픽스 칩으로 실행을 확인하였습니다.
        // (LG 그램, 17Z90n, Intel Iris Plus Graphics)
        // 만약 그래픽스 카드 호환성 문제로 D3D11CreateDevice()가 실패하는 경우에는
        // D3D_DRIVER_TYPE_HARDWARE 대신 D3D_DRIVER_TYPE_WARP 사용해보세요
        // const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;
        const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

        // 여기서 생성하는 것들
        // m_device, m_context, m_swapChain,
        // m_renderTargetView, m_screenViewport, m_rasterizerSate

        // m_device와 m_context 생성

        UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        const D3D_FEATURE_LEVEL featureLevels[2] = {
            D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
            D3D_FEATURE_LEVEL_9_3};
        D3D_FEATURE_LEVEL featureLevel;

        // BGRA 텍스처 포맷 지원을 위한 기본 플래그
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        // 디버그 모드일 때만 디버그 레이어 활성화
#if defined(DEBUG) || defined(_DEBUG)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        if (FAILED(D3D11CreateDevice(
                nullptr,                  // Specify nullptr to use the default adapter.
                driverType,               // Create a device using the hardware graphics driver.
                0,                        // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
                createDeviceFlags,        // Set debug and Direct2D compatibility flags.
                featureLevels,            // List of feature levels this app can support.
                ARRAYSIZE(featureLevels), // Size of the list above.
                D3D11_SDK_VERSION,        // Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
                &device,                  // Returns the Direct3D device created.
                &featureLevel,            // Returns feature level of device created.
                &context                  // Returns the device immediate context.
                )))
        {
            cout << "D3D11CreateDevice() failed." << endl;
            return false;
        }

        if (featureLevel != D3D_FEATURE_LEVEL_11_0)
        {
            cout << "D3D Feature Level 11 unsupported." << endl;
            return false;
        }

        // 참고: Immediate vs deferred context
        // A deferred context is primarily used for multithreading and is not necessary for a
        // single-threaded application.
        // https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-intro#deferred-context

#pragma region swapchain desc

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = 2;                                // Double-buffering
        swapChainDesc.Windowed = true;                                // windowed/full-screen mode
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        //
        swapChainDesc.BufferDesc.Width = m_screenWidth;               // set the back buffer width
        swapChainDesc.BufferDesc.Height = m_screenHeight;             // set the back buffer height
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
        swapChainDesc.OutputWindow = m_mainWindow;                        // the window to be used
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // 화면 스케일링 모드 설정
        //// DXGI_MODE_SCALING_UNSPECIFIED: 시스템이 자동으로 적절한 스케일링 모드 선택
        //// DXGI_MODE_SCALING_CENTERED: 중앙 정렬
        //// DXGI_MODE_SCALING_STRETCHED: 화면에 맞게 늘림
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 스캔라인 순서 설정
                                                                                          //// DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED: 시스템이 자동으로 적절한 순서 선택
                                                                                          //// DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE: 순차 스캔 (위에서 아래로)
                                                                                          //// DXGI_MODE_SCANLINE_ORDER_INTERLACED: 인터레이스 스캔 (홀수/짝수 라인 번갈아가며)
#pragma region 4X MSAA surported check
        // 4X MSAA 지원하는지 확인
        UINT numQualityLevels = 0;
        device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels);
#pragma endregion
        if (numQualityLevels > 0)
        {
            swapChainDesc.SampleDesc.Count = 4; // how many multisamples
            swapChainDesc.SampleDesc.Quality = numQualityLevels - 1;
        }
        else // MSAA not supported.(numQualityLevels == 0)
        {
            swapChainDesc.SampleDesc.Count = 1; // how many multisamples
            swapChainDesc.SampleDesc.Quality = 0;
        }
#pragma endregion
        if (FAILED(device.As(&m_device)))
        {
            cout << "device.AS() failed." << endl;
            return false;
        }

        if (FAILED(context.As(&m_context)))
        {
            cout << "context.As() failed." << endl;
            return false;
        }

        if (!(CreateSwapchain(swapChainDesc)))
            assert(NULL && "Create Swapchain Failed!");
#pragma region CreateDeviceAndSwapChain
        if (FAILED(D3D11CreateDeviceAndSwapChain(0, // Default adapter
                                                 driverType,
                                                 0, // No software device
                                                 createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION,
                                                 &swapChainDesc, &m_swapChain, &m_device, &featureLevel,
                                                 &m_context)))
        {
            cout << "D3D11CreateDeviceAndSwapChain() failed." << endl;
            return false;
        }
#pragma endregion

#pragma region GetBuffer
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&mFrameBuffer));
#pragma endregion

#pragma region CreateRenderTargetView
        if (!(CreateRenderTargetView(mFrameBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf())))
            assert(NULL && "Create RenderTargetView Failed!");
#pragma endregion

#pragma region CreateRasterizerState
        // Create a rasterizer state
        D3D11_RASTERIZER_DESC rastDesc;
        ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC)); // Need this
        rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
        // rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
        rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
        rastDesc.FrontCounterClockwise = false;
        rastDesc.DepthClipEnable = true; // <- zNear, zFar 확인에 필요

        m_device->CreateRasterizerState(&rastDesc, &m_rasterizerSate);
#pragma endregion

#pragma region depthstencil desc
        D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
        depthStencilBufferDesc.Width = m_screenWidth;
        depthStencilBufferDesc.Height = m_screenHeight;
        depthStencilBufferDesc.MipLevels = 1;
        depthStencilBufferDesc.ArraySize = 1;
        depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        if (numQualityLevels > 0)
        {
            depthStencilBufferDesc.SampleDesc.Count = 4; // how many multisamples
            depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
        }
        else
        {
            depthStencilBufferDesc.SampleDesc.Count = 1; // how many multisamples
            depthStencilBufferDesc.SampleDesc.Quality = 0;
        }
        depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilBufferDesc.CPUAccessFlags = 0;
        depthStencilBufferDesc.MiscFlags = 0;
#pragma endregion
        if (FAILED(m_device->CreateTexture2D(&depthStencilBufferDesc, 0,
                                             m_depthStencilBuffer.GetAddressOf())))
        {
            cout << "CreateTexture2D() failed." << endl;
        }
        if (FAILED(
                m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), 0, &m_depthStencilView)))
        {
            cout << "CreateDepthStencilView() failed." << endl;
        }

#pragma region CreateDepthStencilState desc
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        depthStencilDesc.DepthEnable = true; // false
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc,
                                                     m_depthStencilState.GetAddressOf())))
        {
            cout << "CreateDepthStencilState() failed." << endl;
        }
#pragma endregion

        return true;
    }

    bool Graphics::InitGUI()
    {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
        ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get()))
        {
            return false;
        }

        if (!ImGui_ImplWin32_Init(m_mainWindow))
        {
            return false;
        }

        return true;
    }

    // 참고: How To: Compile a Shader
    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/how-to--compile-a-shader

    // 참고: 앞에 L이 붙은 문자열은 wide character로 이루어진 문자열을
    // 의미합니다.
    // String and character literals (C++)
    // https://learn.microsoft.com/en-us/cpp/cpp/string-and-character-literals-cpp?view=msvc-170

    // 참고: 쉐이더를 미리 컴파일해둔 .cso 파일로부터 만들 수도 있습니다.
    // 확장자 cso는 Compiled Shader Object를 의미합니다.
    // 여기서는 쉐이더 파일을 읽어들여서 컴파일합니다.

    void CheckResult(HRESULT hr, ID3DBlob *errorBlob)
    {
        if (FAILED(hr))
        {
            // 파일이 없을 경우
            if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0)
            {
                cout << "File not found." << endl;
            }

            // 에러 메시지가 있으면 출력
            if (errorBlob)
            {
                cout << "Shader compile error\n"
                     << (char *)errorBlob->GetBufferPointer() << endl;
            }
        }
    }

    void Graphics::CreateVertexShaderAndInputLayout(
        const wstring &filename, const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
        ComPtr<ID3D11VertexShader> &vertexShader, ComPtr<ID3D11InputLayout> &inputLayout)
    {

        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        // 주의: 쉐이더의 시작점의 이름이 "main"인 함수로 지정
        HRESULT hr =
            D3DCompileFromFile(filename.c_str(), 0, 0, "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        m_device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
                                     &vertexShader);

        m_device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
                                    shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                                    &inputLayout);
    }

    void Graphics::CreatePixelShader(const wstring &filename, ComPtr<ID3D11PixelShader> &pixelShader)
    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        // 주의: 쉐이더의 시작점의 이름이 "main"인 함수로 지정
        HRESULT hr =
            D3DCompileFromFile(filename.c_str(), 0, 0, "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        m_device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
                                    &pixelShader);
    }

    void Graphics::CreateIndexBuffer(const std::vector<uint16_t> &indices,
                                    ComPtr<ID3D11Buffer> &m_indexBuffer)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
        bufferDesc.ByteWidth = UINT(sizeof(uint16_t) * indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(uint16_t);

        D3D11_SUBRESOURCE_DATA indexBufferData = {0};
        indexBufferData.pSysMem = indices.data();
        indexBufferData.SysMemPitch = 0;
        indexBufferData.SysMemSlicePitch = 0;

        m_device->CreateBuffer(&bufferDesc, &indexBufferData, m_indexBuffer.GetAddressOf());
    }

} // namespace hlab