#include "mesh.hpp"
#include <glm/ext.hpp>

uint32_t Mesh::_IdGenerator = 0;

Mesh::Mesh(){
    _InternalStruct._Id = _IdGenerator;
    _IdGenerator++;
}

void Mesh::setPosition(const glm::vec3& position){
    _InternalStruct._ModelMatrix[3][0] = position.x;
    _InternalStruct._ModelMatrix[3][1] = position.y;
    _InternalStruct._ModelMatrix[3][2] = position.z;
}

void Mesh::setModel(const glm::mat4& model){
    _InternalStruct._ModelMatrix = model;
}


void Mesh::setScale(float scale){
    glm::mat4 scaleMat = glm::mat4(1);
    scaleMat[0][0] *= scale;
    scaleMat[1][1] *= scale;
    scaleMat[2][2] *= scale;
    _InternalStruct._ModelMatrix = glm::transpose(scaleMat*glm::transpose(_InternalStruct._ModelMatrix));
}

void Mesh::setRotation(float thetaX, float thetaY, float thetaZ){
    glm::mat4 rotationX = glm::mat4({
        {1.f, 0.f, 0.f, 0.f},
        {0.f, cos(thetaX), -sin(thetaX), 0.f},
        {0.f, sin(thetaX), cos(thetaX), 0.f},
        {0.f, 0.f, 0.f, 1.f}
    });
    glm::mat4 rotationY = glm::mat4({
        {cos(thetaY), 0.f, sin(thetaY), 0.f},
        {0.f, 1.f, 0.f, 0.f},
        { -sin(thetaY), 0.f, cos(thetaY), 0.f},
        {0.f, 0.f, 0.f, 1.f}
    });
    glm::mat4 rotationZ = glm::mat4({
        {cos(thetaZ), -sin(thetaZ), 0.f, 0.f},
        {sin(thetaZ), cos(thetaZ), 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    });

    _InternalStruct._ModelMatrix = glm::transpose(rotationZ*rotationY*rotationX*glm::transpose(_InternalStruct._ModelMatrix));
}

void Mesh::setMaterial(uint32_t materialId){
    for(Triangle& triangle : _Triangles){
        triangle._InternalStruct._MaterialId = materialId;
    }
}

MeshPtr Mesh::primitiveTriangle(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    newMesh->_Triangles.emplace_back(
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(-1.f, -1.f, 0.f),
        glm::vec3(1.f, -1.f, 0.f),
        newMesh->_InternalStruct._Id
    );
    return newMesh;
}

MeshPtr Mesh::primitiveSquare(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    return newMesh;
}

MeshPtr Mesh::primitiveCube(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    return newMesh;
}

MeshPtr Mesh::primitiveSphere(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    return newMesh;
}
