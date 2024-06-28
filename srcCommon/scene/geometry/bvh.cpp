#include "bvh.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <numeric>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace cr{

BVH::BVH(uint32_t nbTriangles,
    const std::vector<TriangleGPU>& unsortedTriangles,
    const std::vector<MeshModelGPU>& meshesInTheScene){
    // init parameters
    _InternalStruct._NbTriangles = nbTriangles;
    _InternalStruct._UnsortedTriangles = unsortedTriangles;
    _InternalStruct._MeshesInTheScene = meshesInTheScene;
    // fprintf(stdout, "test\n");

    // ploc algorithm
    // auto start = glfwGetTime();
    ploc();
    // fprintf(stdout, "\nploc: %f ms\n", 1000*(glfwGetTime()-start));
}

PlocParams BVH::plocPreprocessing(){
    PlocParams plocParams{};
    sortMortonCodesAndTriangleIndices(_InternalStruct._TriangleIndices, plocParams._MortonCodes);
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        uint32_t triangleIndex = _InternalStruct._TriangleIndices[i];
        BVH_NodeGPU leafCluster{};
        leafCluster._TriangleId = triangleIndex;
        TriangleGPU curTriangle = _InternalStruct._UnsortedTriangles[triangleIndex];
        leafCluster._BoundingBox = AABB::buildFromTriangle(
            curTriangle,
            _InternalStruct._MeshesInTheScene[curTriangle._ModelId]
        );
        _InternalStruct._Clusters[i] = leafCluster;
        plocParams._C_In[i] = i;
        plocParams._C_Out[i].reset();
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
    // fprintf(stdout, "preprocessing done\n");
    // plocParams.printMortonCodes();
    // _InternalStruct.printTriangleIndices();
    // _InternalStruct.printClusters();

    // debug
    // int cpt = 0;

    // main loop
    while(plocParams._Iteration > 1) {
        // fprintf(stdout, "\niteration: %d\n", plocParams._Iteration);
        // plocParams.printC_In();
        // auto start = glfwGetTime();
        // nearest neighbor search
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocNearestNeighborSearch(plocParams, i);
        }
        // fprintf(stdout, "nearest neighbor search: %f ms\n", 1000*(glfwGetTime()-start));
        // fprintf(stdout, "\nnearest neighbors done\n");
        // plocParams.printNearestNeighborIndices();
        
        // start = glfwGetTime();
        // merging
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocMerging(plocParams, i);
        }
        // fprintf(stdout, "merging: %f ms\n", 1000*(glfwGetTime()-start));
        // fprintf(stdout, "\nmerging done\n");
        // plocParams.printC_In();
        // _InternalStruct.printIsLeaf();
        // _InternalStruct.printLeftChild();
        // _InternalStruct.printRightChild();
        // _InternalStruct.printParent();
        // _InternalStruct.printClusters();
        
        // start = glfwGetTime();
        // compaction
        plocPrefixScan(plocParams);
        // fprintf(stdout, "\nprefix scan done\n");
        // plocParams.printPrefixScan();
        // fprintf(stdout, "prefix scan: %f ms\n", 1000*(glfwGetTime()-start));
        // start = glfwGetTime();
        #pragma omp parallel for
        for(uint32_t i=0; i<plocParams._Iteration; i++){
            plocCompaction(plocParams, i);
        }
        // fprintf(stdout, "\ncompaction done\n");
        // plocParams.printC_Out();
        // fprintf(stdout, "compaction: %f ms\n", 1000*(glfwGetTime()-start));

        #pragma omp single
        {
            if(plocParams._C_In[plocParams._Iteration-1].has_value()){
                plocParams._Iteration = plocParams._PrefixScan[plocParams._Iteration-1] + 1;
            } else {
                plocParams._Iteration = plocParams._PrefixScan[plocParams._Iteration-1];
            }
            std::swap(plocParams._C_In, plocParams._C_Out);
        }
        // fprintf(stdout, "\nreinit loop done\n");
        // plocParams.printC_In();
        // plocParams.printC_Out();
        // fprintf(stdout, "new iteration: %u\n", plocParams._Iteration);
        // if(cpt>=6){exit(EXIT_SUCCESS);}

        // debug
        // cpt++;
    }
}

void BVH::plocPrefixScan(PlocParams& plocParams){
    // Hillis Steele Scan    
    uint32_t n = plocParams._Iteration;

    // init the output array
    #pragma omp parallel for
    for(uint32_t i=1; i<n; i++){
        plocParams._PrefixScan[i] = (plocParams._C_In[i-1].has_value()?1:0);
    }

    // up phase
    std::vector<uint32_t> temp = std::vector<uint32_t>(n, 0);
    for(int step=1; step<static_cast<int>(n); step*=2){
        #pragma omp parallel for
        for(int i=step; i<static_cast<int>(n); i++){
            temp[i] = plocParams._PrefixScan[i] + plocParams._PrefixScan[i-step];
        }
        #pragma omp parallel for
        for(int i=step; i<static_cast<int>(n); i++){
            plocParams._PrefixScan[i] = temp[i];
            temp[i] = 0;
        }
    }
}

void BVH::plocCompaction(PlocParams& plocParams, uint32_t index){
    // Compaction phase: write valid clusters to their new positions
    if (plocParams._C_In[index].has_value()) {
        uint32_t newIndex = plocParams._PrefixScan[index];
        plocParams._C_Out[newIndex] = plocParams._C_In[index].value();
    }
}


void BVH::plocMerging(PlocParams& plocParams, uint32_t index){
    uint32_t neighborIndex = plocParams._NearestNeighborIndices[index];
    // if nearest neighbors of two clusters mutually corresond
    if(plocParams._NearestNeighborIndices[neighborIndex] == index){
        // to avoid conflicts, only merging on the lower index
        if(index < neighborIndex){
            // for global clusters arrays
            uint32_t ci = plocParams._C_In[index].value();
            uint32_t ciNeighbor = plocParams._C_In[neighborIndex].value();

            // update new clusters
            BVH_NodeGPU mergedNode = BVH::mergeBVH_Nodes(
                _InternalStruct._Clusters[ci].value(), 
                _InternalStruct._Clusters[ciNeighbor].value()
            );
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
            plocParams._C_In[neighborIndex].reset();
            plocParams._C_In[index] = newClusterIndex;
        }
    }
}

void BVH::plocNearestNeighborSearch(PlocParams& plocParams, uint32_t index){
    float minDist = INFINITY;
    BVH_NodeGPU currentIndexCluster = _InternalStruct._Clusters[plocParams._C_In[index].value()].value();
    uint32_t startIndex = static_cast<uint32_t>(std::max(0, static_cast<int>(index)-static_cast<int>(plocParams._SEARCH_RADIUS)));
    uint32_t endIndex = static_cast<uint32_t>(std::min(index+plocParams._SEARCH_RADIUS+1, plocParams._Iteration));
    // fprintf(stdout, "start index: %u, end index: %u\n", startIndex, endIndex);
    for(uint32_t j=startIndex; j<endIndex; j++){
        if(j == index){continue;}
        BVH_NodeGPU jIndexCluster = _InternalStruct._Clusters[plocParams._C_In[j].value()].value();
        AABB_GPU newAABB = AABB::merge(currentIndexCluster._BoundingBox, jIndexCluster._BoundingBox);
        float curDist = AABB::getSurfaceArea(newAABB);
        // fprintf(stdout, "i: %u, j: %u, dist: %f, minDist: %f\n", index, j, curDist, minDist);
        if(curDist < minDist){
            minDist = curDist;
            plocParams._NearestNeighborIndices[index] = j;
        }
    }
}



void BVH::sortMortonCodesAndTriangleIndices(
            std::vector<uint32_t>& triangleIndices,
            std::vector<uint32_t>& mortonCodes
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

std::vector<glm::vec3> BVH::getTrianglesCentroids() const{
    std::vector<glm::vec3> centroids = std::vector<glm::vec3>(Triangle::MAX_NB_TRIANGLES, glm::vec3(0.f));
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        TriangleGPU triangle = _InternalStruct._UnsortedTriangles[i];
        centroids[i] = Triangle::getCentroid(triangle, _InternalStruct._MeshesInTheScene[triangle._ModelId]._ModelMatrix);
    }
    return centroids;
}

std::vector<glm::vec3> BVH::getNormalizedCentroids(
            const std::vector<glm::vec3>& centroids,
            const AABB_GPU& circumscribedCube) const {
    std::vector<glm::vec3> normalizedCentroids = std::vector<glm::vec3>(Triangle::MAX_NB_TRIANGLES, glm::vec3(0.f));
    for(size_t i=0; i<_InternalStruct._NbTriangles; i++){
        float lengthX = (circumscribedCube._Max.x - circumscribedCube._Min.x);
        float lengthY = (circumscribedCube._Max.y - circumscribedCube._Min.y);
        float lengthZ = (circumscribedCube._Max.z - circumscribedCube._Min.z);
        float normalizedX = (centroids[i].x - circumscribedCube._Min.x) / lengthX;
        float normalizedY = (centroids[i].y - circumscribedCube._Min.y) / lengthY;
        float normalizedZ = (centroids[i].z - circumscribedCube._Min.z) / lengthZ;
        normalizedCentroids[i] = glm::vec3(normalizedX, normalizedY, normalizedZ);
    }
    return normalizedCentroids;
}

std::vector<uint32_t> BVH::getMortonCodes() const {
    // get scene AABB
    AABB_GPU sceneBoundingBox = getSceneAABB();
    // build circumscribed cube
    AABB_GPU circumscribedCube = getCircumscribedCube(sceneBoundingBox);
    // get the triangle centroids
    std::vector<glm::vec3> trianglesCentroids = getTrianglesCentroids();

    // normalize the centroids
    std::vector<glm::vec3> trianglesNormalizedCentroids = getNormalizedCentroids(trianglesCentroids, circumscribedCube);
    // compute the morton codes
    std::vector<uint32_t> mortonCodes = std::vector<uint32_t>(Triangle::MAX_NB_TRIANGLES, 0);
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

AABB_GPU AABB::buildFromTriangle(const TriangleGPU& triangle, const MeshModelGPU& model){
    AABB_GPU aabb{};

    glm::vec4 p0 = model._ModelMatrix * triangle._P0;
    glm::vec4 p1 = model._ModelMatrix * triangle._P1;
    glm::vec4 p2 = model._ModelMatrix * triangle._P2;

    aabb._Min.x = std::min(p0.x, std::min(p1.x, p2.x));
    aabb._Min.y = std::min(p0.y, std::min(p1.y, p2.y));
    aabb._Min.z = std::min(p0.z, std::min(p1.z, p2.z));

    aabb._Max.x = std::max(p0.x, std::max(p1.x, p2.x));
    aabb._Max.y = std::max(p0.y, std::max(p1.y, p2.y));
    aabb._Max.z = std::max(p0.z, std::max(p1.z, p2.z));

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
    mergedNode._BoundingBox = AABB::merge(node1._BoundingBox, node2._BoundingBox);
    return mergedNode;
}


void BVH_Params::printParent() const {
    fprintf(stdout, "Parent Array:\n[ ");
    for(size_t i = 0; i < (2 * Triangle::MAX_NB_TRIANGLES) - 1; i++) {
        if(_Parent[i].has_value()){
            fprintf(stdout, "%u", _Parent[i].value());
        } else {
            fprintf(stdout, "null");
        }
        if (i < (2 * Triangle::MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printLeftChild() const {
    fprintf(stdout, "LeftChild Array:\n[ ");
    for(size_t i = 0; i < (2 * Triangle::MAX_NB_TRIANGLES) - 1; i++) {
        if(_LeftChild[i].has_value()){
            fprintf(stdout, "%u", _LeftChild[i].value());
        } else {
            fprintf(stdout, "null");
        }
        if (i < (2 * Triangle::MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printRightChild() const {
    fprintf(stdout, "RightChild Array:\n[ ");
    for(size_t i = 0; i < (2 * Triangle::MAX_NB_TRIANGLES) - 1; i++) {
        if(_RightChild[i].has_value()){
            fprintf(stdout, "%u", _RightChild[i].value());
        } else {
            fprintf(stdout, "null");
        }
        if (i < (2 * Triangle::MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printIsLeaf() const {
    fprintf(stdout, "IsLeaf Array:\n[ ");
    for(size_t i = 0; i < (2 * Triangle::MAX_NB_TRIANGLES) - 1; i++) {
        if(_IsLeaf[i].has_value()){
            fprintf(stdout, "%s", _IsLeaf[i].value() ? "true" : "false");
        } else {
            fprintf(stdout, "null");
        }
        if (i < (2 * Triangle::MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void BVH_Params::printClusters() const {
    fprintf(stdout, "Clusters Array:\n[\n ");
    for(size_t i = 0; i < (2 * Triangle::MAX_NB_TRIANGLES) - 1; i++) {
        if(!_Clusters[i].has_value()){
            fprintf(stdout, "null");
        } else {
            fprintf(
                stdout,
                "{leftChild: %u, rightChild: %u, triId: %u, aabb: (%s, %s)}",
                _Clusters[i]->_LeftChild,
                _Clusters[i]->_RightChild,
                _Clusters[i]->_TriangleId,
                glm::to_string(_Clusters[i]->_BoundingBox._Min).c_str(),
                glm::to_string(_Clusters[i]->_BoundingBox._Max).c_str()
            );
        }
        if (i < (2 * Triangle::MAX_NB_TRIANGLES) - 2) {
            fprintf(stdout, ",\n ");
        }
    }
    fprintf(stdout, "\n]\n");
}

void BVH_Params::printTriangleIndices() const {
    fprintf(stdout, "Triangle indices Array:\n[ ");
    for(size_t i = 0; i < Triangle::MAX_NB_TRIANGLES; i++) {
        fprintf(stdout, "%u", _TriangleIndices[i]);
        if (i < (Triangle::MAX_NB_TRIANGLES) - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void PlocParams::printMortonCodes() const {
    fprintf(stdout, "MortonCodes Array:\n[ ");
    for(size_t i = 0; i < _MortonCodes.size(); i++) {
        fprintf(stdout, "%u", _MortonCodes[i]);
        if (i < _MortonCodes.size() - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void PlocParams::printC_In() const {
    fprintf(stdout, "C_In Array:\n[ ");
    for(size_t i = 0; i < _C_In.size(); i++) {
        if (_C_In[i]) {
            fprintf(stdout, "%u", _C_In[i].value());
        } else {
            fprintf(stdout, "null");
        }
        if (i < _C_In.size() - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void PlocParams::printC_Out() const {
    fprintf(stdout, "C_Out Array:\n[ ");
    for(size_t i = 0; i < _C_Out.size(); i++) {
        if (_C_Out[i]) {
            fprintf(stdout, "%u", _C_Out[i].value());
        } else {
            fprintf(stdout, "null");
        }
        if (i < _C_Out.size() - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void PlocParams::printNearestNeighborIndices() const {
    fprintf(stdout, "NearestNeighborIndices Array:\n[ ");
    for(size_t i = 0; i < _NearestNeighborIndices.size(); i++) {
        fprintf(stdout, "%u", _NearestNeighborIndices[i]);
        if (i < _NearestNeighborIndices.size() - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

void PlocParams::printPrefixScan() const {
    fprintf(stdout, "PrefixScan Array:\n[ ");
    for(size_t i = 0; i < _PrefixScan.size(); i++) {
        fprintf(stdout, "%u", _PrefixScan[i]);
        if (i < _PrefixScan.size() - 1) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, " ]\n");
}

}