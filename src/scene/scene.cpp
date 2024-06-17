#include "scene.hpp"

#include <algorithm>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "errorHandler.hpp"

Scene::Scene(){
    createSSBO();
}

std::vector<MeshModelGPU> Scene::getMeshModelToGPUData() const {
    std::vector<MeshModelGPU> modelsGPU = std::vector<MeshModelGPU>(MAX_NB_MESHES);
    for(int i=0; i<std::min(static_cast<int>(_Meshes.size()), MAX_NB_MESHES); i++){
        modelsGPU[i] = _Meshes[i]->_InternalStruct;
    }
    return modelsGPU;
}


std::vector<TriangleGPU> Scene::getTriangleToGPUData() const {
    std::vector<TriangleGPU> trianglesGPU = std::vector<TriangleGPU>(MAX_NB_TRIANGLES);
    uint32_t i = 0;
    for(MeshPtr mesh : _Meshes){
        for(Triangle triangle : mesh->_Triangles){
            if(i == MAX_NB_TRIANGLES){
                break;
            }
            trianglesGPU[i] = triangle._InternalStruct;
            i++;
        }  
    }
    
    return trianglesGPU;
}

std::vector<MaterialGPU> Scene::getMaterialToGPUData() const {
    std::vector<MaterialGPU> materialGPU = std::vector<MaterialGPU>(MAX_NB_MATERIALS);
    for(int i=0; i<std::min(static_cast<int>(_Materials.size()), MAX_NB_MATERIALS); i++){
        materialGPU[i] = _Materials[i]._InternalStruct;
    }
    return materialGPU;
}

void Scene::addMesh(MeshPtr mesh){
    if(_Meshes.size() == MAX_NB_MESHES) return;
    _Meshes.push_back(mesh);
    _NbMeshes++;
    _NbTriangles += std::min(static_cast<int>(mesh->_Triangles.size()), MAX_NB_TRIANGLES);
}

void Scene::addMaterial(const glm::vec4& color){
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    _Materials.emplace_back(color);
    _NbMaterials++;
}

void Scene::addRandomMaterial(){
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    _Materials.emplace_back();
    _NbMaterials++;
}

void Scene::createSSBO(){
    glCreateBuffers(1, &_MaterialsSSBO);
    glCreateBuffers(1, &_TrianglesSSBO);
    glCreateBuffers(1, &_MeshModelsSSBO);
    glCreateBuffers(1, &_BVH_SSBO);
    assert(_MaterialsSSBO != 0);
    assert(_TrianglesSSBO != 0);
    assert(_MeshModelsSSBO != 0);
    assert(_BVH_SSBO != 0);

    // Calculate the total size of the buffer
    size_t materialsSize = sizeof(MaterialGPU) * MAX_NB_MATERIALS;
    size_t trianglesSize = sizeof(TriangleGPU) * MAX_NB_TRIANGLES;
    size_t modelsSize = sizeof(MeshModelGPU) * MAX_NB_MESHES;
    size_t bvhSize = (sizeof(BVH_NodeGPU) * ((2*MAX_NB_TRIANGLES)-1)) + (sizeof(uint32_t) * MAX_NB_TRIANGLES);

    glNamedBufferStorage(_MaterialsSSBO, 
                    materialsSize, 
                    nullptr, 
                    GL_DYNAMIC_STORAGE_BIT
    );

    glNamedBufferStorage(_TrianglesSSBO, 
                    trianglesSize, 
                    nullptr, 
                    GL_DYNAMIC_STORAGE_BIT
    );

    glNamedBufferStorage(_MeshModelsSSBO, 
                    modelsSize, 
                    nullptr, 
                    GL_DYNAMIC_STORAGE_BIT
    );

    glNamedBufferStorage(_BVH_SSBO,
                    bvhSize,
                    nullptr,
                    GL_DYNAMIC_STORAGE_BIT
    );
}

void Scene::bindSSBO(){
    // materials
    GLuint materialsBinding = 2;
    auto materialGPU = getMaterialToGPUData();
    GLsizeiptr materialsSize = sizeof(MaterialGPU) * _NbMaterials;
    // update the materials
    glNamedBufferSubData(_MaterialsSSBO,
        0,
        materialsSize,
        materialGPU.data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsBinding, _MaterialsSSBO);

    // triangles
    GLuint trianglesBinding = 3;
    auto triangleGPU = getTriangleToGPUData();
    GLsizeiptr trianglesSize = sizeof(TriangleGPU) * _NbTriangles;
    // update the materials
    glNamedBufferSubData(_TrianglesSSBO,
        0,
        trianglesSize,
        triangleGPU.data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, trianglesBinding, _TrianglesSSBO);

    // models
    GLuint modelsBinding = 4;
    auto modelsGPU = getMeshModelToGPUData();
    GLsizeiptr modelsSize = sizeof(MeshModelGPU) * _NbMeshes;
    // update the materials
    glNamedBufferSubData(_MeshModelsSSBO,
        0,
        modelsSize,
        modelsGPU.data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, modelsBinding, _MeshModelsSSBO);

    // bvh
    BVH_Ptr bvh = BVH_Ptr(new BVH(_NbTriangles, triangleGPU, modelsGPU));
    // bvh->_InternalStruct.printIsLeaf();
    // bvh->_InternalStruct.printLeftChild();
    // bvh->_InternalStruct.printRightChild();
    // bvh->_InternalStruct.printParent();
    // bvh->_InternalStruct.printTriangleIndices();
    // bvh->_InternalStruct.printClusters();
    GLuint bvhBinding = 5;
    auto bvhNodesGPU = getBVH_NodesToGPUData(bvh);
    GLsizeiptr bvhNodesSize = sizeof(BVH_NodeGPU) * ((2*_NbTriangles)-1);
    glNamedBufferSubData(_BVH_SSBO, 
        0, 
        bvhNodesSize, 
        bvhNodesGPU.data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bvhBinding, _BVH_SSBO);

    // tests
    if(glGetError() != GL_NO_ERROR){
        ErrorHandler::handle(
            __FILE__,
            __LINE__,
            ErrorCode::OPENGL_ERROR,
            "Failed to bind the ssbos\n"
        );
    }
}

void Scene::recursiveTopDownTraversalBVH(std::vector<BVH_NodeGPU>& bvhNodesGPU, BVH_Ptr bvh, uint32_t nodeId) const {
    BVH_NodeGPU curNode = bvh->_InternalStruct._Clusters[nodeId].value();
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

std::vector<BVH_NodeGPU> Scene::getBVH_NodesToGPUData(BVH_Ptr bvh) const{
    std::vector<BVH_NodeGPU> bvhNodesGPU = std::vector<BVH_NodeGPU>();
    uint32_t rootId = 2*_NbTriangles - 2;
    recursiveTopDownTraversalBVH(bvhNodesGPU, bvh, rootId);
    return bvhNodesGPU;
}

void Scene::sendDataToGpu(ProgramPtr program){
    program->use();
    // bind the SSBOs
    bindSSBO();
    // Set the number of elements
    program->setUInt("uNbTriangles", _NbTriangles);
    program->setUInt("uNbMaterials", _NbMaterials);
    program->setUInt("uNbModels", _NbMeshes);

    glUseProgram(0);
}
