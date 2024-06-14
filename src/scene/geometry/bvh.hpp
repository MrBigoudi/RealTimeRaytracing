#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <array>
#include <optional>
#include <vector>

#include "triangle.hpp"
#include "mesh.hpp"

enum Axis {
    X,Y,Z
};

struct AABB_GPU {
    glm::vec4 _Min = INFINITY*glm::vec4(1.f,1.f,1.f,1.f);
    glm::vec4 _Max = -INFINITY*glm::vec4(1.f,1.f,1.f,1.f);
};

class AABB{
    public:
        static float getDiagonal(const AABB_GPU& aabb);
        static float getSurfaceArea(const AABB_GPU& aabb);
        static AABB_GPU buildFromTriangle(const TriangleGPU& triangle);
        static AABB_GPU merge(const AABB_GPU& aabb1, const AABB_GPU& aabb2);
};

struct BVH_NodeGPU {
    uint32_t _NbTriangles;
    uint32_t _FirstTriangleIndex;
    AABB_GPU _BoundingBox;
};

struct BVH_Params {
    size_t _NbTriangles;
    std::array<TriangleGPU, MAX_NB_TRIANGLES> _UnsortedTriangles;
    std::array<MeshModelGPU, MAX_NB_MESHES> _MeshesInTheScene;
    
    // bvh structure
    std::array<std::optional<BVH_NodeGPU>, (2*MAX_NB_TRIANGLES)-1> _Clusters{};
    std::array<bool, (2*MAX_NB_TRIANGLES)-1> _IsLeaf{};
    std::array<uint32_t, (2*MAX_NB_TRIANGLES)-1> _Parent{};
    std::array<uint32_t, (2*MAX_NB_TRIANGLES)-1> _LeftChild{};
    std::array<uint32_t, (2*MAX_NB_TRIANGLES)-1> _RightChild{};
    std::array<uint32_t, MAX_NB_TRIANGLES> _TriangleIndices{};

    void printParent() const;
    void printLeftChild() const;
    void printRightChild() const;
    void printIsLeaf() const;
};

struct PlocParams {
    const uint32_t _SEARCH_RADIUS = 16;
    uint32_t _NbTotalClusters = 0;

    uint32_t _Iteration = 0;
    std::array<uint32_t, MAX_NB_TRIANGLES> _MortonCodes{};

    std::array<std::optional<uint32_t>, MAX_NB_TRIANGLES> _C_In{};
    std::array<std::optional<uint32_t>, MAX_NB_TRIANGLES> _C_Out{};
    std::array<uint32_t, MAX_NB_TRIANGLES> _NearestNeighborIndices{};
    std::array<uint32_t, MAX_NB_TRIANGLES> _PrefixScan{};
};

class BVH {
    public:
        BVH_Params _InternalStruct = {};

    public:
        BVH(uint32_t nbTriangles,
            const std::array<TriangleGPU, MAX_NB_TRIANGLES>& unsortedTriangles,
            const std::array<MeshModelGPU, MAX_NB_MESHES>& meshesInTheScene);

    private:
        static BVH_NodeGPU mergeBVH_Nodes(const BVH_NodeGPU& node1, const BVH_NodeGPU& node2);

    private:
        std::array<uint32_t, MAX_NB_TRIANGLES> getMortonCodes() const;

        AABB_GPU getSceneAABB() const;

        AABB_GPU getCircumscribedCube(const AABB_GPU& sceneAABB) const;

        std::array<glm::vec3, MAX_NB_TRIANGLES> getTrianglesCentroids() const;

        std::array<glm::vec3, MAX_NB_TRIANGLES> getNormalizedCentroids(
            const std::array<glm::vec3, MAX_NB_TRIANGLES>& centroids,
            const AABB_GPU& circumscribedCube) const; 

        uint32_t expandBits(uint32_t value) const;
        uint32_t morton3D(const glm::vec3& point) const;

        void sortMortonCodesAndTriangleIndices(
            std::array<uint32_t, MAX_NB_TRIANGLES>& triangleIndices, // empty
            std::array<uint32_t, MAX_NB_TRIANGLES>& mortonCodes // empty
        ) const;


        void ploc();
        PlocParams plocPreprocessing();
        void plocNearestNeighborSearch(PlocParams& plocParams, uint32_t index);
        void plocMerging(PlocParams& plocParams, uint32_t index);
        void plocCompaction(PlocParams& plocParams, uint32_t index);
        void plocPrefixScan(PlocParams& plocParams, uint32_t index);
};