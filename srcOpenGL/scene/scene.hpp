#pragma once

#include <vector>
#include <array>

#include "program.hpp"

#include "triangle.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "bvh.hpp"

#include <glad/gl.h>

#include <memory>

namespace glr{

class Scene;
using ScenePtr = std::shared_ptr<Scene>;

class Scene{
    private:
        std::vector<cr::Material> _Materials = {cr::Material()}; // always one default material
        std::vector<cr::MeshPtr> _Meshes = {};

        uint32_t _MaterialIdGenerator = 0;
        
        GLuint _TrianglesSSBO = 0;
        GLuint _MaterialsSSBO = 0;
        GLuint _MeshModelsSSBO = 0;
        GLuint _BVH_SSBO = 0;

        uint32_t _NbTriangles = 0;
        uint32_t _NbMaterials = 1; // the default one
        uint32_t _NbMeshes = 0;

        // tmp
        cr::BVH_Ptr _BVH = nullptr;

    public:
        Scene();

    public:
        std::vector<cr::TriangleGPU> getTriangleToGPUData() const;
        std::vector<cr::MaterialGPU> getMaterialToGPUData() const;
        std::vector<cr::MeshModelGPU> getMeshModelToGPUData() const;
        std::vector<cr::BVH_NodeGPU> getBVH_NodesToGPUData(cr::BVH_Ptr bvh) const;

        void addMesh(cr::MeshPtr mesh);
        void addMaterial(const glm::vec4& color);
        void addRandomMaterial();

        void sendDataToGpu(ProgramPtr program);

    private:
        void createSSBO();
        void bindSSBO();

        void recursiveTopDownTraversalBVH(std::vector<cr::BVH_NodeGPU>& bvhNodesGPU, cr::BVH_Ptr bvh, uint32_t nodeId) const;
        void recursiveBottomUpTraversalBVH(std::vector<cr::BVH_NodeGPU>& bvhNodesGPU, cr::BVH_Ptr bvh, uint32_t nodeId) const;
};

}