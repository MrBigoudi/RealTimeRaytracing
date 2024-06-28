#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <array>
#include <optional>
#include <vector>
#include <memory>

#include "triangle.hpp"
#include "mesh.hpp"

namespace cr{

class BVH;
using BVH_Ptr = std::shared_ptr<BVH>;

enum Axis {
    X,Y,Z
};

struct AABB_GPU {
    glm::vec3 _Min = INFINITY*glm::vec3(1.f,1.f,1.f);
    alignas(16) 
    glm::vec3 _Max = -INFINITY*glm::vec3(1.f,1.f,1.f);
};

class AABB{
    public:
        static float getDiagonal(const AABB_GPU& aabb);
        static float getSurfaceArea(const AABB_GPU& aabb);
        static AABB_GPU buildFromTriangle(const TriangleGPU& triangle, const MeshModelGPU& model);
        static AABB_GPU merge(const AABB_GPU& aabb1, const AABB_GPU& aabb2);
};

struct BVH_NodeGPU {
    AABB_GPU _BoundingBox;
    uint32_t _TriangleId;
    uint32_t _LeftChild;
    uint32_t _RightChild;
    // if child == 0 then leaf
};

struct BVH_Params {
    size_t _NbTriangles;
    std::vector<TriangleGPU> _UnsortedTriangles = std::vector<TriangleGPU>(Triangle::MAX_NB_TRIANGLES);
    std::vector<MeshModelGPU> _MeshesInTheScene = std::vector<MeshModelGPU>(Mesh::MAX_NB_MESHES);
    
    // bvh structure
    std::vector<std::optional<BVH_NodeGPU>> _Clusters = std::vector<std::optional<BVH_NodeGPU>>((2*Triangle::MAX_NB_TRIANGLES)-1, std::nullopt);
    std::vector<std::optional<bool>> _IsLeaf = std::vector<std::optional<bool>>((2*Triangle::MAX_NB_TRIANGLES)-1, std::nullopt);
    std::vector<std::optional<uint32_t>> _Parent = std::vector<std::optional<uint32_t>>((2*Triangle::MAX_NB_TRIANGLES)-1, std::nullopt);
    std::vector<std::optional<uint32_t>> _LeftChild = std::vector<std::optional<uint32_t>>((2*Triangle::MAX_NB_TRIANGLES)-1, std::nullopt);
    std::vector<std::optional<uint32_t>> _RightChild = std::vector<std::optional<uint32_t>>((2*Triangle::MAX_NB_TRIANGLES)-1, std::nullopt);
    std::vector<uint32_t> _TriangleIndices = std::vector<uint32_t>(Triangle::MAX_NB_TRIANGLES, 0);

    void printParent() const;
    void printLeftChild() const;
    void printRightChild() const;
    void printIsLeaf() const;
    void printTriangleIndices() const;
    void printClusters() const;
};

struct PlocParams {
    const uint32_t _SEARCH_RADIUS = 16;
    uint32_t _NbTotalClusters = 0;

    uint32_t _Iteration = 0;
    std::vector<uint32_t> _MortonCodes = std::vector<uint32_t>(Triangle::MAX_NB_TRIANGLES, 0);

    std::vector<std::optional<uint32_t>> _C_In = std::vector<std::optional<uint32_t>>(Triangle::MAX_NB_TRIANGLES, std::nullopt);
    std::vector<std::optional<uint32_t>> _C_Out = std::vector<std::optional<uint32_t>>(Triangle::MAX_NB_TRIANGLES, std::nullopt);
    std::vector<uint32_t> _NearestNeighborIndices = std::vector<uint32_t>(Triangle::MAX_NB_TRIANGLES);
    std::vector<uint32_t> _PrefixScan = std::vector<uint32_t>(Triangle::MAX_NB_TRIANGLES);

    void printMortonCodes() const;
    void printC_In() const;
    void printC_Out() const;
    void printNearestNeighborIndices() const;
    void printPrefixScan() const;
};

class BVH {
    public:
        BVH_Params _InternalStruct = {};

    public:
        BVH(uint32_t nbTriangles,
            const std::vector<TriangleGPU>& unsortedTriangles,
            const std::vector<MeshModelGPU>& meshesInTheScene);

    private:
        static BVH_NodeGPU mergeBVH_Nodes(const BVH_NodeGPU& node1, const BVH_NodeGPU& node2);

    private:
        std::vector<uint32_t> getMortonCodes() const;

        AABB_GPU getSceneAABB() const;

        AABB_GPU getCircumscribedCube(const AABB_GPU& sceneAABB) const;

        std::vector<glm::vec3> getTrianglesCentroids() const;

        std::vector<glm::vec3> getNormalizedCentroids(
            const std::vector<glm::vec3>& centroids,
            const AABB_GPU& circumscribedCube) const; 

        uint32_t expandBits(uint32_t value) const;
        uint32_t morton3D(const glm::vec3& point) const;

        void sortMortonCodesAndTriangleIndices(
            std::vector<uint32_t>& triangleIndices, // empty
            std::vector<uint32_t>& mortonCodes // empty
        ) const;


        void ploc();
        PlocParams plocPreprocessing();
        void plocNearestNeighborSearch(PlocParams& plocParams, uint32_t index);
        void plocMerging(PlocParams& plocParams, uint32_t index);
        void plocCompaction(PlocParams& plocParams, uint32_t index);
        void plocPrefixScan(PlocParams& plocParams);
};

}