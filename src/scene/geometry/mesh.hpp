#pragma once

#include "triangle.hpp"
#include <vector>
#include <memory>

class Mesh;
using MeshPtr = std::shared_ptr<Mesh>;

#define MAX_NB_MESHES 2<<5

struct MeshModelGPU {
    glm::mat4 _ModelMatrix = glm::mat4(1);
    uint32_t _Id;
};

class Mesh{
    private:
        static uint32_t _IdGenerator;

    public:
        std::vector<Triangle> _Triangles{};
        MeshModelGPU _InternalStruct;

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

};