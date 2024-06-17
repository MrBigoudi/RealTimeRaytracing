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

class Scene;
using ScenePtr = std::shared_ptr<Scene>;

class Scene{
    private:
        std::vector<Material> _Materials = {Material()}; // always one default material
        std::vector<MeshPtr> _Meshes = {};

        uint32_t _MaterialIdGenerator = 0;
        
        GLuint _TrianglesSSBO = 0;
        GLuint _MaterialsSSBO = 0;
        GLuint _MeshModelsSSBO = 0;
        GLuint _BVH_SSBO = 0;

        uint32_t _NbTriangles = 0;
        uint32_t _NbMaterials = 1; // the default one
        uint32_t _NbMeshes = 0;

    public:
        Scene();

    public:
        std::vector<TriangleGPU> getTriangleToGPUData() const;
        std::vector<MaterialGPU> getMaterialToGPUData() const;
        std::vector<MeshModelGPU> getMeshModelToGPUData() const;
        std::vector<BVH_NodeGPU> getBVH_NodesToGPUData(BVH_Ptr bvh) const;

        void addMesh(MeshPtr mesh);
        void addMaterial(const glm::vec4& color);
        void addRandomMaterial();

        void sendDataToGpu(ProgramPtr program);

    private:
        void createSSBO();
        void bindSSBO();

        void recursiveTopDownTraversalBVH(std::vector<BVH_NodeGPU>& bvhNodesGPU, BVH_Ptr bvh, uint32_t nodeId) const;
        void recursiveBottomUpTraversalBVH(std::vector<BVH_NodeGPU>& bvhNodesGPU, BVH_Ptr bvh, uint32_t nodeId) const;
};