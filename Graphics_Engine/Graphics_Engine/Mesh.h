#pragma once

#include <d3d11.h>
#include <windows.h>
#include <wrl.h> // ComPtr

namespace luke {

    using Microsoft::WRL::ComPtr;

    struct Mesh {

        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
        ComPtr<ID3D11Buffer> m_constantBuffer;

        UINT m_indexCount = 0;
    };
}
