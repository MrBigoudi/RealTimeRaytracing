#include "mesh.hpp"

#include <tiny_obj_loader.h>
#include <glm/ext.hpp>
#include "errorHandler.hpp"

uint32_t Mesh::_IdGenerator = 0;

const std::string Mesh::MODELS_DIRECTORY = std::string(PROJECT_SOURCE_DIR) + "/resources/models/";


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
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, 0.f),
        glm::vec3(-1.f, -1.f, 0.f),
        glm::vec3(1.f, -1.f, 0.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, 0.f),
        glm::vec3(1.f, -1.f, 0.f),
        glm::vec3(1.f, 1.f, 0.f),
        newMesh->_InternalStruct._Id
    );
    return newMesh;
}

MeshPtr Mesh::primitiveCube(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    // front face
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, -1.f),
        glm::vec3(-1.f, -1.f, -1.f),
        glm::vec3(1.f, -1.f, -1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, -1.f),
        glm::vec3(1.f, -1.f, -1.f),
        glm::vec3(1.f, 1.f, -1.f),
        newMesh->_InternalStruct._Id
    );

    // back face
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, -1.f, 1.f),
        glm::vec3(-1.f, 1.f, 1.f),
        glm::vec3(1.f, -1.f, 1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(1.f, -1.f, 1.f),
        glm::vec3(-1.f, 1.f, 1.f),
        glm::vec3(1.f, 1.f, 1.f),
        newMesh->_InternalStruct._Id
    );

    // right face
    newMesh->_Triangles.emplace_back(
        glm::vec3(1.f, -1.f, 1.f),
        glm::vec3(1.f, 1.f, 1.f),
        glm::vec3(1.f, 1.f, -1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(1.f, -1.f, 1.f),
        glm::vec3(1.f, 1.f, -1.f),
        glm::vec3(1.f, -1.f, -1.f),
        newMesh->_InternalStruct._Id
    );

    // left face
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, 1.f),
        glm::vec3(-1.f, -1.f, 1.f),
        glm::vec3(-1.f, 1.f, -1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, -1.f),
        glm::vec3(-1.f, -1.f, 1.f),
        glm::vec3(-1.f, -1.f, -1.f),
        newMesh->_InternalStruct._Id
    );

    // top face
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, -1.f),
        glm::vec3(1.f, 1.f, -1.f),
        glm::vec3(1.f, 1.f, 1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, 1.f, -1.f),
        glm::vec3(1.f, 1.f, 1.f),
        glm::vec3(-1.f, 1.f, 1.f),
        newMesh->_InternalStruct._Id
    );

    // bottom face
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, -1.f, -1.f),
        glm::vec3(1.f, -1.f, 1.f),
        glm::vec3(1.f, -1.f, -1.f),
        newMesh->_InternalStruct._Id
    );
    newMesh->_Triangles.emplace_back(
        glm::vec3(-1.f, -1.f, -1.f),
        glm::vec3(-1.f, -1.f, 1.f),
        glm::vec3(1.f, -1.f, 1.f),
        newMesh->_InternalStruct._Id
    );

    return newMesh;
}

MeshPtr Mesh::primitiveSphere(){
    MeshPtr newMesh = MeshPtr(new Mesh());
    return newMesh;
}

MeshPtr Mesh::load(const std::string& path){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if(!warn.empty()){
        ErrorHandler::handle(
            __FILE__, __LINE__,
            ErrorCode::IO_ERROR,
            "Warning loading object `" + path + "': " + warn + "\n",
            ErrorLevel::WARNING 
        );
    }

    if(!err.empty()){
        ErrorHandler::handle(
            __FILE__, __LINE__,
            ErrorCode::IO_ERROR,
            "Error loading object `" + path + "': " + warn + "\n"
        );
    }

    MeshPtr loadedModel = MeshPtr(new Mesh());
    
    // Loop over shapes
    for (const auto& shape : shapes) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];

            // Check if the face is a triangle
            if (fv != 3) {
                ErrorHandler::handle(
                    __FILE__, __LINE__,
                    ErrorCode::BAD_VALUE_ERROR,
                    "Error loading object `" + path + "': Mesh face with vertices other than 3 is not supported!\n"
                );
                continue;
            }

            glm::vec3 vertices[3];
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                vertices[v] = glm::vec3(vx, vy, vz);
            }

            // check ccw order
            // Calculate the normal of the triangle
            glm::vec3 edge1 = vertices[1] - vertices[0];
            glm::vec3 edge2 = vertices[2] - vertices[0];
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
            // Check if the normal points in the correct direction
            // Assuming positive z direction is the front face direction
            if (normal.z < 0.0f) {
                // If the normal points in the wrong direction, swap vertices[1] and vertices[2]
                std::swap(vertices[1], vertices[2]);
            }

            // Create a triangle and add it to the mesh
            loadedModel->_Triangles.emplace_back(
                vertices[0], vertices[1], vertices[2], 
                loadedModel->_InternalStruct._Id
            );

            index_offset += fv;
        }
    }

    return loadedModel;

}