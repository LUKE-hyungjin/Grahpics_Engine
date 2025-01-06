#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace luke {

    using DirectX::SimpleMath::Vector2;
    using DirectX::SimpleMath::Vector3;

    struct Vertex {
        Vector3 position;
        Vector3 color;
    };

    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
    };

    class MeshGenerator {
    public:
        static MeshData MakeTriangle();
        static MeshData MakeSquare();
        static MeshData MakeCube();
    };
}
