#include "bvh.hpp"
#include <algorithm>
#include <numeric>

BVH::BVH(uint32_t nbTriangles,
    const std::array<TriangleGPU, MAX_NB_TRIANGLES>& unsortedTriangles,
    const std::array<MeshModelGPU, MAX_NB_MESHES>& meshesInTheScene){
    // init parameters
    _InternalStruct._NbTriangles = nbTriangles;
    _InternalStruct._UnsortedTriangles = unsortedTriangles;
    _InternalStruct._MeshesInTheScene = meshesInTheScene;

    // ploc algorithm
    ploc();
}

PlocParams BVH::plocPreprocessing(){
    PlocParams plocParams{};
    sortMortonCodesAndTriangleIndices(_InternalStruct._TriangleIndices, plocParams._MortonCodes);
    _InternalStruct._IsLeaf.fill(false);
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        uint32_t triangleIndex = _InternalStruct._TriangleIndices[i];
        BVH_NodeGPU leafCluster{};
        leafCluster._FirstTriangleIndex = i;
        leafCluster._NbTriangles = 1;
        leafCluster._BoundingBox = AABB::buildFromTriangle(_InternalStruct._UnsortedTriangles[triangleIndex]);
        _InternalStruct._Clusters[i] = leafCluster;
        plocParams._C_In[i] = i;
        plocParams._C_Out[i] = i;
        _InternalStruct._IsLeaf[i] = true;
    }
    plocParams._Iteration = _InternalStruct._NbTriangles;
    plocParams._NbTotalClusters = _InternalStruct._NbTriangles;
    return plocParams;
}

void BVH::ploc(){
    /// PLOC algorithm
    /// cf papers/ploc.pdf
    // preprocessing
    PlocParams plocParams = plocPreprocessing();
    // main loop
    while(plocParams._Iteration > 1) {
        // nearest neighbor search
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocNearestNeighborSearch(plocParams, i);
        }
        
        // merging
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocMerging(plocParams, i);
        }
        
        // compaction
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocCompaction(plocParams, i);
        }

        #pragma omp single
        {
            plocParams._Iteration = plocParams._PrefixScan[plocParams._Iteration-1];
            if(plocParams._C_In[plocParams._Iteration-1].has_value()){
                plocParams._Iteration++;
            }
            std::swap(plocParams._C_In, plocParams._C_Out);
        }
    }
}

void BVH::plocPrefixScan(PlocParams& plocParams, uint32_t index){
    uint32_t n = plocParams._Iteration;

    // Initialize local storage for this thread's contributions
    std::vector<uint32_t> localContributions(n, 0);

    // Each thread contributes to its local value
    localContributions[index] = plocParams._C_In[index].has_value() ? 1 : 0;

    // Prefix scan using OpenMP (up-sweep phase)
    for (uint32_t d = 1; d < n; d <<= 1) {
        uint32_t temp = localContributions[index];
        if (index >= d) {
            temp += localContributions[index - d];
        }
        #pragma omp barrier
        localContributions[index] = temp;
        #pragma omp barrier
    }

    // Ensure prefix sum array is updated correctly in a single pass
    #pragma omp single
    {
        plocParams._PrefixScan[0] = 0;
        for (uint32_t i = 1; i < n; ++i) {
            plocParams._PrefixScan[i] = localContributions[i - 1];
        }
    }
}

void BVH::plocCompaction(PlocParams& plocParams, uint32_t index){
    plocPrefixScan(plocParams, index);
    // Compaction phase: write valid clusters to their new positions
    if (plocParams._C_In[index].has_value()) {
        uint32_t newIndex = plocParams._PrefixScan[index];
        plocParams._C_Out[newIndex] = plocParams._C_In[index];
    }
}


void BVH::plocMerging(PlocParams& plocParams, uint32_t index){
    // if nearest neighbors of two clusters mutually corresond
    if(plocParams._NearestNeighborIndices[plocParams._NearestNeighborIndices[index]] == index){
        // to avoid conflicts, only merging on the lower index
        if(index < plocParams._NearestNeighborIndices[index]){
            uint32_t ci = plocParams._C_In[index].value();
            uint32_t ciNeighbor = plocParams._C_In[plocParams._NearestNeighborIndices[index]].value();

            // update new clusters
            BVH_NodeGPU mergedNode = BVH::mergeBVH_Nodes(_InternalStruct._Clusters[ci].value(), _InternalStruct._Clusters[ciNeighbor].value());
            uint32_t newClusterIndex = 0;
            #pragma omp critical
            {
                newClusterIndex = plocParams._NbTotalClusters;
                plocParams._NbTotalClusters++;
            }
            _InternalStruct._Clusters[newClusterIndex] = mergedNode;
            _InternalStruct._LeftChild[newClusterIndex] = ci;
            _InternalStruct._RightChild[newClusterIndex] = ciNeighbor;
            _InternalStruct._Parent[ci] = newClusterIndex;
            _InternalStruct._Parent[ciNeighbor] = newClusterIndex;

            // mark merged cluster as invalid
            plocParams._C_In[ciNeighbor].reset();
            plocParams._C_In[ci] = newClusterIndex;
        }
    }
}

void BVH::plocNearestNeighborSearch(PlocParams& plocParams, uint32_t index){
    float minDist = INFINITY;
    BVH_NodeGPU currentIndexCluster = _InternalStruct._Clusters[plocParams._C_In[index].value()].value();
    uint32_t startIndex = static_cast<uint32_t>(std::max(static_cast<uint32_t>(0), index-plocParams._SEARCH_RADIUS));
    uint32_t endIndex = static_cast<uint32_t>(std::min(index+plocParams._SEARCH_RADIUS+1, plocParams._Iteration));
    for(uint32_t j=startIndex; j<endIndex; j++){
        if(j == index){continue;}
        BVH_NodeGPU jIndexCluster = _InternalStruct._Clusters[plocParams._C_In[j].value()].value();
        AABB_GPU newAABB = AABB::merge(currentIndexCluster._BoundingBox, jIndexCluster._BoundingBox);
        float curDist = AABB::getSurfaceArea(newAABB);
        if(curDist < minDist){
            minDist = curDist;
            plocParams._NearestNeighborIndices[index] = j;
        }
    }
}



void BVH::sortMortonCodesAndTriangleIndices(
            std::array<uint32_t, MAX_NB_TRIANGLES>& triangleIndices,
            std::array<uint32_t, MAX_NB_TRIANGLES>& mortonCodes
        ) const {
    // generate triangle indices
    std::iota(triangleIndices.begin(), triangleIndices.end(), 0);
    // generate morton codes
    mortonCodes = getMortonCodes();
    // sort morton codes and the array of indices and put them in an array of pair
    std::vector<std::pair<uint32_t, uint32_t>> mortonIndexPairs(_InternalStruct._NbTriangles);
    for (size_t i = 0; i < _InternalStruct._NbTriangles; i++) {
        mortonIndexPairs[i] = {mortonCodes[i], triangleIndices[i]};
    }
    std::sort(mortonIndexPairs.begin(), mortonIndexPairs.end());
    for (size_t i = 0; i < _InternalStruct._NbTriangles; i++) {
        mortonCodes[i] = mortonIndexPairs[i].first;
        triangleIndices[i] = mortonIndexPairs[i].second;
    }
}


AABB_GPU BVH::getSceneAABB() const {
    AABB_GPU sceneBoundingBox{};
    for(const TriangleGPU& triangle : _InternalStruct._UnsortedTriangles){
        glm::vec4 p0 = _InternalStruct._MeshesInTheScene[triangle._ModelId]._ModelMatrix * triangle._P0;
        glm::vec4 p1 = _InternalStruct._MeshesInTheScene[triangle._ModelId]._ModelMatrix * triangle._P1;
        glm::vec4 p2 = _InternalStruct._MeshesInTheScene[triangle._ModelId]._ModelMatrix * triangle._P2;

        sceneBoundingBox._Max.x = std::max(sceneBoundingBox._Max.x, std::max(p0.x, std::max(p1.x, p2.x)));
        sceneBoundingBox._Max.y = std::max(sceneBoundingBox._Max.y, std::max(p0.y, std::max(p1.y, p2.y)));
        sceneBoundingBox._Max.z = std::max(sceneBoundingBox._Max.z, std::max(p0.z, std::max(p1.z, p2.z)));

        sceneBoundingBox._Min.x = std::min(sceneBoundingBox._Min.x, std::min(p0.x, std::min(p1.x, p2.x)));
        sceneBoundingBox._Min.y = std::min(sceneBoundingBox._Min.y, std::min(p0.y, std::min(p1.y, p2.y)));
        sceneBoundingBox._Min.z = std::min(sceneBoundingBox._Min.z, std::min(p0.z, std::min(p1.z, p2.z)));
    }
    return sceneBoundingBox;
}

AABB_GPU BVH::getCircumscribedCube(const AABB_GPU& sceneAABB) const {
    AABB_GPU circumscribedCube = sceneAABB;

    float distX = sceneAABB._Max.x - sceneAABB._Min.x;
    float distY = sceneAABB._Max.y - sceneAABB._Min.y;
    float distZ = sceneAABB._Max.z - sceneAABB._Min.z;

    float maxDist = 0.f;
    Axis maxDistAxis = X;

    if(distX > maxDist){
        maxDist = distX;
        maxDistAxis = X;
    } else if(distY > maxDist){
        maxDist = distY;
        maxDistAxis = Y;
    } else {
        maxDist = distZ;
        maxDistAxis = Z;
    }

    float delta = 0.f;
    switch(maxDistAxis){
        case X:
            delta = (maxDist - distY) / 2.f;
            circumscribedCube._Max.y += delta;
            circumscribedCube._Min.y -= delta;
            delta = (maxDist - distZ) / 2.f;
            circumscribedCube._Max.z += delta;
            circumscribedCube._Min.z -= delta;
            break;
        case Y:
            delta = (maxDist - distX) / 2.f;
            circumscribedCube._Max.x += delta;
            circumscribedCube._Min.x -= delta;
            delta = (maxDist - distZ) / 2.f;
            circumscribedCube._Max.z += delta;
            circumscribedCube._Min.z -= delta;
            break;
        case Z:
            delta = (maxDist - distX) / 2.f;
            circumscribedCube._Max.x += delta;
            circumscribedCube._Min.x -= delta;
            delta = (maxDist - distY) / 2.f;
            circumscribedCube._Max.y += delta;
            circumscribedCube._Min.y -= delta;
            break;
    }

    return circumscribedCube;
}

std::array<glm::vec3, MAX_NB_TRIANGLES> BVH::getTrianglesCentroids() const{
    std::array<glm::vec3, MAX_NB_TRIANGLES> centroids = {};
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        TriangleGPU triangle = _InternalStruct._UnsortedTriangles[i];
        centroids[i] = Triangle::getCentroid(triangle, _InternalStruct._MeshesInTheScene[triangle._ModelId]._ModelMatrix);
    }
    return centroids;
}

std::array<glm::vec3, MAX_NB_TRIANGLES> BVH::getNormalizedCentroids(
            const std::array<glm::vec3, MAX_NB_TRIANGLES>& centroids,
            const AABB_GPU& circumscribedCube) const {
    std::array<glm::vec3, MAX_NB_TRIANGLES> normalizedCentroids = {};
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        float normalizedX = (centroids[i].x - circumscribedCube._Min.x) / (circumscribedCube._Max.x - circumscribedCube._Min.x);
        float normalizedY = (centroids[i].y - circumscribedCube._Min.y) / (circumscribedCube._Max.y - circumscribedCube._Min.y);
        float normalizedZ = (centroids[i].z - circumscribedCube._Min.z) / (circumscribedCube._Max.z - circumscribedCube._Min.z);
        normalizedCentroids[i] = glm::vec3(normalizedX, normalizedY, normalizedZ);
    }
    return normalizedCentroids;
}

std::array<uint32_t, MAX_NB_TRIANGLES> BVH::getMortonCodes() const {
    // get scene AABB
    AABB_GPU sceneBoundingBox = getSceneAABB();
    // build circumscribed cube
    AABB_GPU circumscribedCube = getCircumscribedCube(sceneBoundingBox);
    // get the triangle centroids
    std::array<glm::vec3, MAX_NB_TRIANGLES> trianglesCentroids = getTrianglesCentroids();
    // normalize the centroids
    std::array<glm::vec3, MAX_NB_TRIANGLES> trianglesNormalizedCentroids = getNormalizedCentroids(trianglesCentroids, circumscribedCube);
    // compute the morton codes
    std::array<uint32_t, MAX_NB_TRIANGLES> mortonCodes{};
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        glm::vec3 centroid = trianglesNormalizedCentroids[i];
        uint32_t code = morton3D(centroid);
        mortonCodes[i] = code;
    }
    return mortonCodes;
}

uint32_t BVH::expandBits(uint32_t value) const {
    value = (value * 0x00010001u) & 0xFF0000FFu;
    value = (value * 0x00000101u) & 0x0F00F00Fu;
    value = (value * 0x00000011u) & 0xC30C30C3u;
    value = (value * 0x00000005u) & 0x49249249u;
    return value;
}

uint32_t BVH::morton3D(const glm::vec3& point) const {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
    y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
    z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
    
    uint32_t xx = expandBits((uint32_t)x);
    uint32_t yy = expandBits((uint32_t)y);
    uint32_t zz = expandBits((uint32_t)z);
    
    return (xx << 2) | (yy << 1) | zz;
}

float AABB::getDiagonal(const AABB_GPU& aabb){
    return glm::distance(aabb._Max, aabb._Min);
}

float AABB::getSurfaceArea(const AABB_GPU& aabb){
    glm::vec3 diff = aabb._Max - aabb._Min;
    return 2 * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
}

AABB_GPU AABB::buildFromTriangle(const TriangleGPU& triangle){
    AABB_GPU aabb{};

    aabb._Min.x = std::min(triangle._P0.x, std::min(triangle._P1.x, triangle._P2.x));
    aabb._Min.y = std::min(triangle._P0.y, std::min(triangle._P1.y, triangle._P2.y));
    aabb._Min.z = std::min(triangle._P0.z, std::min(triangle._P1.z, triangle._P2.z));

    aabb._Max.x = std::max(triangle._P0.x, std::max(triangle._P1.x, triangle._P2.x));
    aabb._Max.y = std::max(triangle._P0.y, std::max(triangle._P1.y, triangle._P2.y));
    aabb._Max.z = std::max(triangle._P0.z, std::max(triangle._P1.z, triangle._P2.z));

    return aabb;
}

AABB_GPU AABB::merge(const AABB_GPU& aabb1, const AABB_GPU& aabb2){
    AABB_GPU aabb{};

    aabb._Min.x = std::min(aabb1._Min.x, aabb2._Min.x);
    aabb._Min.y = std::min(aabb1._Min.y, aabb2._Min.y);
    aabb._Min.z = std::min(aabb1._Min.z, aabb2._Min.z);

    aabb._Max.x = std::max(aabb1._Max.x, aabb2._Max.x);
    aabb._Max.y = std::max(aabb1._Max.y, aabb2._Max.y);
    aabb._Max.z = std::max(aabb1._Max.z, aabb2._Max.z);

    return aabb;
}

BVH_NodeGPU BVH::mergeBVH_Nodes(const BVH_NodeGPU& node1, const BVH_NodeGPU& node2){
    // TODO: rethink this
    BVH_NodeGPU mergedNode{};
    mergedNode._NbTriangles = node1._NbTriangles + node2._NbTriangles;
    mergedNode._FirstTriangleIndex = node1._FirstTriangleIndex;
    mergedNode._BoundingBox = AABB::merge(node1._BoundingBox, node2._BoundingBox);
    return mergedNode;
}


void BVH_Params::printParent() const {
    fprintf(stdout, "Parent Array:\n[ ");
    for(size_t i = 0; i < (2 * MAX_NB_TRIANGLES) - 1; ++i) {
        fprintf(stdout, "%u", _Parent[i]);
        if (i < (2 * MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printLeftChild() const {
    fprintf(stdout, "LeftChild Array:\n[ ");
    for(size_t i = 0; i < (2 * MAX_NB_TRIANGLES) - 1; ++i) {
        fprintf(stdout, "%u", _LeftChild[i]);
        if (i < (2 * MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printRightChild() const {
    fprintf(stdout, "RightChild Array:\n[ ");
    for(size_t i = 0; i < (2 * MAX_NB_TRIANGLES) - 1; ++i) {
        fprintf(stdout, "%u", _RightChild[i]);
        if (i < (2 * MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printIsLeaf() const {
    fprintf(stdout, "IsLeaf Array:\n[ ");
    for(size_t i = 0; i < (2 * MAX_NB_TRIANGLES) - 1; ++i) {
        fprintf(stdout, "%s", _IsLeaf[i] ? "true" : "false");
        if (i < (2 * MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}