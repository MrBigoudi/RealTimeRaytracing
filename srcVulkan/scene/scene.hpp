#pragma once

#include <vector>
#include <array>

#include "material.hpp"
#include "mesh.hpp"
#include "bvh.hpp"

#include <memory>

namespace vkr{

class Scene;
using ScenePtr = std::shared_ptr<Scene>;

class Scene{
    private:
        std::vector<cr::Material> _Materials = {cr::Material()}; // always one default material
        std::vector<cr::MeshPtr> _Meshes = {};

        uint32_t _NbTriangles = 0;
        uint32_t _NbMaterials = 1; // the default one
        uint32_t _NbMeshes = 0;

        cr::BVH_Ptr _BVH = nullptr;

    public:
        Scene();

    public:
        void addMesh(cr::MeshPtr mesh);
        void addMaterial(const glm::vec4& color);
        void addRandomMaterial();

    private:
        void recursiveTopDownTraversalBVH(std::vector<cr::BVH_NodeGPU>& bvhNodesGPU, cr::BVH_Ptr bvh, uint32_t nodeId) const;
};

}