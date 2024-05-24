#include "scene.hpp"

#include <algorithm>

Scene::Scene(){}

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

void Scene::addMaterial(const glm::vec4& color){
    // TODO: better handling
    if(_Materials.size() == MAX_NB_MATERIALS) return;
    uint32_t materialId = _Materials.size();
    _Materials.emplace_back(materialId, color);
}

void Scene::createUBO(){
    GLuint ubo = 0;
    glGenBuffers(1, &ubo);
    assert(ubo != 0);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    // Calculate the total size of the buffer
    size_t trianglesSize = sizeof(TriangleGPU) * MAX_NB_TRIANGLES;
    size_t materialsSize = sizeof(MaterialGPU) * MAX_NB_MATERIALS;
    size_t totalSize = trianglesSize + materialsSize;

    // Allocate space for the buffer
    glBufferData(GL_UNIFORM_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

    // Upload data1 to the buffer
    std::array<TriangleGPU, MAX_NB_TRIANGLES> dataTriangles = getTriangleToGPUData();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, trianglesSize, dataTriangles.data());

    // Upload data2 to the buffer
    std::array<MaterialGPU, MAX_NB_MATERIALS> dataMaterials = getMaterialToGPUData();
    glBufferSubData(GL_UNIFORM_BUFFER, trianglesSize, materialsSize, dataMaterials.data());

    // Bind the buffer to the uniform block binding point
    GLuint bindingPoint = 0;
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);

    // Unbind the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::sendDataToGpu(ProgramPtr program){
    program->use();
    // Create and bind the UBO
    createUBO();
    // Get the uniform block index
    GLuint blockIndex = glGetUniformBlockIndex(program->getId(), "uGeometryData");
    // Bind the uniform block to the binding point
    GLuint bindingPoint = 0;
    glUniformBlockBinding(program->getId(), blockIndex, bindingPoint);
    // Set the number of elements
    program->setUInt("uNbTriangles", _Triangles.size());
    program->setUInt("uNbMaterials", _Materials.size());
}
