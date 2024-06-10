#include "scene.hpp"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "errorHandler.hpp"

Scene::Scene(){
    createSSBO();
}

std::array<MeshModelGPU, MAX_NB_MESHES> Scene::getMeshModelToGPUData() const {
    std::array<MeshModelGPU, MAX_NB_MESHES> modelsGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Meshes.size()), MAX_NB_MESHES); i++){
        modelsGPU[i] = _Meshes[i]->_InternalStruct;
    }
    return modelsGPU;
}


std::array<TriangleGPU, MAX_NB_TRIANGLES> Scene::getTriangleToGPUData() const {
    std::array<TriangleGPU, MAX_NB_TRIANGLES> trianglesGPU{};
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

std::array<MaterialGPU, MAX_NB_MATERIALS> Scene::getMaterialToGPUData() const {
    std::array<MaterialGPU, MAX_NB_MATERIALS> materialGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Materials.size()), MAX_NB_MATERIALS); i++){
        materialGPU[i] = _Materials[i]._InternalStruct;
    }
    return materialGPU;
}


void Scene::addObject(MeshPtr mesh){
    if(_Meshes.size() == MAX_NB_MESHES) return;
    _Meshes.push_back(mesh);
    _NbMeshes++;
    _NbTriangles += std::min(static_cast<int>(mesh->_Triangles.size()), MAX_NB_TRIANGLES);
}

void Scene::addMaterial(const glm::vec4& color){
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    uint32_t materialId = _Materials.size();
    _Materials.emplace_back(materialId, color);
    _NbMaterials++;
}

void Scene::addRandomMaterial(){
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    uint32_t materialId = _Materials.size();
    _Materials.emplace_back(materialId);
    _NbMaterials++;
}

void Scene::createSSBO(){
    glCreateBuffers(1, &_MaterialsSSBO);
    glCreateBuffers(1, &_TrianglesSSBO);
    glCreateBuffers(1, &_MeshModelsSSBO);
    assert(_MaterialsSSBO != 0);
    assert(_TrianglesSSBO != 0);
    assert(_MeshModelsSSBO != 0);

    // Calculate the total size of the buffer
    size_t materialsSize = sizeof(MaterialGPU) * MAX_NB_MATERIALS;
    size_t trianglesSize = sizeof(TriangleGPU) * MAX_NB_TRIANGLES;
    size_t modelsSize = sizeof(MeshModelGPU) * MAX_NB_MESHES;

    glNamedBufferStorage(_MaterialsSSBO, 
                     materialsSize, 
                     nullptr, 
                     GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(_TrianglesSSBO, 
                     trianglesSize, 
                     nullptr, 
                     GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(_MeshModelsSSBO, 
                     modelsSize, 
                     nullptr, 
                     GL_DYNAMIC_STORAGE_BIT);
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
