#pragma once

#include "triangle.hpp"
#include <vector>
#include <memory>

namespace glr{

class Mesh;
using MeshPtr = std::shared_ptr<Mesh>;

#define MAX_NB_MESHES 2<<5

struct MeshModelGPU {
    glm::mat4 _ModelMatrix = glm::mat4(1);
    uint32_t _MaterialId = 0;
};

class Mesh{
    private:
        static uint32_t _IdGenerator;
        uint32_t _Id = 0;

    public:
        std::vector<Triangle> _Triangles{};
        MeshModelGPU _InternalStruct;
        static const std::string MODELS_DIRECTORY;

    public:
        Mesh();

    public:
        void setModel(const glm::mat4& model);
        void setPosition(const glm::vec3& position);
        void setScale(float scale);
        void setRotation(float thetaX, float thetaY, float thetaZ);
        void setMaterial(uint32_t materialId);

    public:
        static MeshPtr primitiveTriangle();
        static MeshPtr primitiveSquare();
        static MeshPtr primitiveCube();
        static MeshPtr primitiveSphere();
        static MeshPtr load(const std::string& path);

};

}