#include "MeshGenerator.h"
#include <iostream>

namespace luke {
    using namespace std;
    using namespace DirectX;
    using namespace DirectX::SimpleMath;
    MeshData MeshGenerator::MakeTriangle()
    {

        vector<Vector3> positions;
        vector<Vector3> colors;
        vector<Vector3> normals;

        const float scale = 1.0f;

        // 윗면
        positions.push_back(Vector3(0.0f, 0.5f, 0.0f) * scale);
        positions.push_back(Vector3(0.5f, -0.5f, 0.0f) * scale);
        positions.push_back(Vector3(-0.5f, -0.5f, 0.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

        MeshData meshData;
        for (size_t i = 0; i < positions.size(); i++)
        {
            Vertex v;
            v.position = positions[i];
            v.color = colors[i];
            //v.normal = normals[i];
            meshData.vertices.push_back(v);
        }

        meshData.indices = {
            0, 1, 2 
        };

        return meshData;
    }

    MeshData MeshGenerator::MakeSquare() {
        vector<Vector3> positions;
        vector<Vector3> colors;
        vector<Vector3> normals;

        const float scale = 0.5f;

        positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

        MeshData meshData;

        for (size_t i = 0; i < positions.size(); i++) {
            Vertex v;
            v.position = positions[i];
            v.color = colors[i];
            // v.normals = normals[i];

            meshData.vertices.push_back(v);
        }
        meshData.indices = {
            0, 1, 2, 0, 2, 3,
        };

        return meshData;
    }
}
