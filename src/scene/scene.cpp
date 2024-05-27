#include "scene.hpp"

#include <algorithm>

#include "errorHandler.hpp"

Scene::Scene(){
    createSSBO();
}

std::array<TriangleGPU, MAX_NB_TRIANGLES> Scene::getTriangleToGPUData() const {
    std::array<TriangleGPU, MAX_NB_TRIANGLES> trianglesGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Triangles.size()), MAX_NB_TRIANGLES); i++){
        trianglesGPU[i] = _Triangles[i]._InternalStruct;
    }
    return trianglesGPU;
}

std::array<MaterialGPU, MAX_NB_MATERIALS> Scene::getMaterialToGPUData() const {
    std::array<MaterialGPU, MAX_NB_MATERIALS> MaterialGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Materials.size()), MAX_NB_MATERIALS); i++){
        MaterialGPU[i] = _Materials[i]._InternalStruct;
    }
    return MaterialGPU;
}

void Scene::addTriangle(uint32_t materialId, const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2){
    // TODO: better handling
    assert(materialId < _Materials.size());
    if(_Triangles.size() == MAX_NB_TRIANGLES) return;
    uint32_t triangleId = _Triangles.size();
    _Triangles.emplace_back(triangleId, materialId, p0, p1, p2);
}

void Scene::addRandomTriangle(){
    uint32_t materialId = rand()%_Materials.size();
    if(_Triangles.size() == MAX_NB_TRIANGLES) return;
    uint32_t triangleId = _Triangles.size();
    _Triangles.emplace_back(triangleId, materialId);
}

void Scene::addMaterial(const glm::vec4& color){
    // TODO: better handling
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    uint32_t materialId = _Materials.size();
    _Materials.emplace_back(materialId, color);
}

void Scene::addRandomMaterial(){
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    uint32_t materialId = _Materials.size();
    _Materials.emplace_back(materialId);
}

void Scene::createSSBO(){
    glCreateBuffers(1, &_MaterialsSSBO);
    glCreateBuffers(1, &_TrianglesSSBO);
    assert(_MaterialsSSBO != 0);
    assert(_TrianglesSSBO != 0);

    // Calculate the total size of the buffer
    size_t materialsSize = sizeof(MaterialGPU) * MAX_NB_MATERIALS;
    size_t trianglesSize = sizeof(TriangleGPU) * MAX_NB_TRIANGLES;

    glNamedBufferStorage(_MaterialsSSBO, 
                     materialsSize, 
                     nullptr, 
                     GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(_TrianglesSSBO, 
                     trianglesSize, 
                     nullptr, 
                     GL_DYNAMIC_STORAGE_BIT);
}

void Scene::bindSSBO(){
    // materials
    GLuint materialsBinding = 2;
    GLsizeiptr materialsSize = sizeof(MaterialGPU) * _Materials.size();
    // update the materials
    glNamedBufferSubData(_MaterialsSSBO,
        0,
        materialsSize,
        getMaterialToGPUData().data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsBinding, _MaterialsSSBO);

    // triangles
    GLuint trianglesBinding = 3;
    GLsizeiptr trianglesSize = sizeof(TriangleGPU) * _Triangles.size();
    // update the materials
    glNamedBufferSubData(_TrianglesSSBO,
        0,
        trianglesSize,
        getTriangleToGPUData().data()
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, trianglesBinding, _TrianglesSSBO);
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
    program->setUInt("uNbTriangles", _Triangles.size());
    program->setUInt("uNbMaterials", _Materials.size());
    glUseProgram(0);
}
