#include "scene.hpp"

#include <algorithm>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace vkr{

Scene::Scene(){
}

void Scene::addMesh(cr::MeshPtr mesh){
    if(_Meshes.size() == cr::Mesh::MAX_NB_MESHES) return;
    _Meshes.push_back(mesh);
    _NbMeshes++;
    _NbTriangles += std::min(mesh->_Triangles.size(), cr::Triangle::MAX_NB_TRIANGLES);
}

void Scene::addMaterial(const glm::vec4& color){
    if(_Materials.size() == cr::Material::MAX_NB_MATERIALS) return;
    _Materials.emplace_back(color);
    _NbMaterials++;
}

void Scene::addRandomMaterial(){
    if(_Materials.size() == cr::Material::MAX_NB_MATERIALS) return;
    _Materials.emplace_back();
    _NbMaterials++;
}

void Scene::recursiveTopDownTraversalBVH(std::vector<cr::BVH_NodeGPU>& bvhNodesGPU, cr::BVH_Ptr bvh, uint32_t nodeId) const {
    cr::BVH_NodeGPU curNode = bvh->_InternalStruct._Clusters[nodeId].value();
    uint32_t position = bvhNodesGPU.size();
    bvhNodesGPU.push_back(curNode);
    if(!bvh->_InternalStruct._IsLeaf[nodeId]){
        uint32_t leftChildId = bvh->_InternalStruct._LeftChild[nodeId].value();
        bvhNodesGPU[position]._LeftChild = bvhNodesGPU.size();
        recursiveTopDownTraversalBVH(bvhNodesGPU, bvh, leftChildId);
        uint32_t rightChildId = bvh->_InternalStruct._RightChild[nodeId].value();
        bvhNodesGPU[position]._RightChild = bvhNodesGPU.size();
        recursiveTopDownTraversalBVH(bvhNodesGPU, bvh, rightChildId);
    }
}

}