#include "triangle.hpp"

#include <cstdlib>

namespace cr{

uint32_t Triangle::_IdGenerator = 0;

Triangle::Triangle(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, uint32_t modelId){
    _IdGenerator++;
    _InternalStruct = {};
    _InternalStruct._ModelId = modelId;
    _InternalStruct._P0 = p0;
    _InternalStruct._P1 = p1;
    _InternalStruct._P2 = p2;
}

Triangle::Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, uint32_t modelId){
    _IdGenerator++;
    _InternalStruct = {};
    _InternalStruct._ModelId = modelId;
    _InternalStruct._P0 = glm::vec4(p0, 1.f);
    _InternalStruct._P1 = glm::vec4(p1, 1.f);
    _InternalStruct._P2 = glm::vec4(p2, 1.f);
}

glm::vec3 Triangle::getCentroid(const TriangleGPU& triangle){
    return glm::vec3((1.f/3.f) * (triangle._P0 + triangle._P1 + triangle._P2));
}

glm::vec3 Triangle::getCentroid(const TriangleGPU& triangle, const glm::mat4& model){
    return glm::vec3((1.f/3.f) * model * (triangle._P0 + triangle._P1 + triangle._P2));
}

}
